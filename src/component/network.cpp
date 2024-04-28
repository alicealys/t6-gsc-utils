#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include "gsc.hpp"
#include "io.hpp"
#include "scheduler.hpp"
#include "scripting.hpp"

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/flags.hpp>

namespace network
{
	namespace
	{
		struct socket_t
		{
			SOCKET socket;
			scripting::object handle;
		};

		std::vector<socket_t> connected_sockets;
		std::unordered_map<std::uint32_t, SOCKET> handle_to_socket;

		void poll_sockets()
		{
			for (const auto& inst : connected_sockets)
			{
				std::string data;
				data.reserve(0x1000);

				char buffer[0x1000]{};

				auto result = 0;
				do
				{
					result = recv(inst.socket, buffer, sizeof(buffer), 0);
					if (result != -1 && result != 0)
					{
						data.append(buffer);
					}
				} while (result != -1 && result != 0);

				if (data.size() == 0)
				{
					continue;
				}

				auto start_index = 0;
				scripting::array chunks;

				for (auto i = 0u; i < data.size(); i++)
				{
					const auto is_eof = data[i] == EOF;
					const auto is_end = i == data.size() - 1;
					if (is_eof || is_end)
					{
						const auto chunk = data.substr(start_index, i - start_index);
						chunks.emplace_back(chunk);
						start_index = i + 1;
					}
				}

				scripting::notify(inst.handle.get_entity_id(), "data", {chunks.get_raw()});
			}
		}
	}

	class component final : public component_interface
	{
	public:
		void on_startup([[maybe_unused]] plugin::plugin* plugin) override
		{
			if (!utils::flags::has_flag("experimental-utils"))
			{
				return;
			}

			scripting::on_shutdown([]
			{
				for (const auto& inst : connected_sockets)
				{
					closesocket(inst.socket);
				}

				connected_sockets.clear();
				handle_to_socket.clear();
			});

			scheduler::loop(poll_sockets, scheduler::server_packet_loop);

			gsc::function::add("network::connect", [](const std::string& type, const std::string& address, const int port)
				-> scripting::script_value
			{
				static std::unordered_map<std::string, int> socket_types =
				{
					{"tcp", SOCK_STREAM},
					{"udp", SOCK_DGRAM},
				};

				const auto type_lower = utils::string::to_lower(type);
				const auto type_id = socket_types.find(type_lower);
				if (type_id == socket_types.end())
				{
					throw std::runtime_error("invalid socket type");
				}

				const auto sock = socket(AF_INET, type_id->second, 0);

				u_long mode = 1;
				if (ioctlsocket(sock, FIONBIO, &mode) == SOCKET_ERROR)
				{
					closesocket(sock);
					return {};
				}

				int flag = 1;
				setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));

				SOCKADDR_IN sin{};
				sin.sin_family = AF_INET;
				sin.sin_port = htons(static_cast<std::uint16_t>(port));
				inet_pton(AF_INET, address.data(), &sin.sin_addr);

				if (connect(sock, reinterpret_cast<SOCKADDR*>(&sin), sizeof(sin)) == SOCKET_ERROR)
				{
					const auto error = WSAGetLastError();
					if (error != WSAEWOULDBLOCK && error != WSAEINPROGRESS)
					{
						closesocket(sock);
						return {};
					}
				}

				scripting::object handle;

				socket_t socket_ref{};
				socket_ref.socket = sock;
				socket_ref.handle = handle;

				const auto handle_id = handle.get_entity_id();
				handle_to_socket[handle_id] = sock;

				connected_sockets.emplace_back(std::move(socket_ref));

				return handle;
			});

			gsc::function::add("network::send", [](const scripting::object& handle, std::string data)
			{
				const auto handle_id = handle.get_entity_id();
				const auto iter = handle_to_socket.find(handle_id);
				if (iter == handle_to_socket.end())
				{
					return -1;
				}

				const auto sock = iter->second;
				data += EOF;
				return send(sock, data.data(), data.size(), 0);
			});

			gsc::function::add("network::close", [](const scripting::object& handle)
			{
				const auto handle_id = handle.get_entity_id();
				const auto iter = handle_to_socket.find(handle_id);
				if (iter == handle_to_socket.end())
				{
					return;
				}
				
				closesocket(iter->second);
				handle_to_socket.erase(iter);

				for (auto i = connected_sockets.begin(), end = connected_sockets.end(); i != end; ++i)
				{
					if (i->handle.get_entity_id() == handle_id)
					{
						connected_sockets.erase(i);
						break;
					}
				}
			});

			gsc::function::add("network::is_connected", [](const scripting::object& handle)
			{
				const auto handle_id = handle.get_entity_id();
				const auto iter = handle_to_socket.find(handle_id);
				if (iter == handle_to_socket.end())
				{
					return (int)0;
				}

				char code{};
				int size = sizeof(code);
				getsockopt(iter->second, SOL_SOCKET, SO_ERROR, &code, &size);
				return static_cast<int>(code);
			});

			gsc::function::add("network::get_last_error", []()
			{
				return WSAGetLastError();
			});
		}
	};
}

REGISTER_COMPONENT(network::component)
