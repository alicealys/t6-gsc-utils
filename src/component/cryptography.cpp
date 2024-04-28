#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include "gsc.hpp"
#include "io.hpp"
#include "scheduler.hpp"
#include "scripting.hpp"

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/cryptography.hpp>
#include <utils/flags.hpp>

namespace cryptography
{
	namespace
	{
		std::vector<utils::cryptography::ecc::key> keys;
	}

	class component final : public component_interface
	{
	public:
		void on_startup([[maybe_unused]] plugin::plugin* plugin) override
		{
			if (!utils::flags::has_flag("experimental-utils"))
			{
				printf("doesnt have experimental utils enabled\n");
				return;
			}

			scripting::on_shutdown([]
			{
				keys.clear();
			});

			gsc::function::add("ecc::generate_key", [](int bits)
			{
				const auto key = utils::cryptography::ecc::generate_key(bits);
				keys.emplace_back(key);
				return keys.size() - 1;
			});

			gsc::function::add("ecc::create_key_from_public_key", [](const std::string& data)
			{
				utils::cryptography::ecc::key key;
				key.set(utils::string::hex_to_bin(data));
				keys.emplace_back(key);
				return keys.size() - 1;
			});

			gsc::function::add("ecc::encrypt", [](const std::uint32_t key_index, const std::string& data)
			{
				if (key_index < 0 || key_index >= keys.size())
				{
					throw std::runtime_error("invalid key");
				}

				const auto& key = keys.at(key_index);
				std::string encrypted = data;
				if (!utils::cryptography::ecc::encrypt(key, encrypted))
				{
					throw std::runtime_error("failed to encrypt");
				}

				return utils::string::bin_to_hex(encrypted);
			});

			gsc::function::add("ecc::decrypt", [](const std::uint32_t key_index, const std::string& data)
			{
				if (key_index < 0 || key_index >= keys.size())
				{
					throw std::runtime_error("invalid key");
				}

				const auto& key = keys.at(key_index);
				std::string decrypted = utils::string::hex_to_bin(data);
				if (!utils::cryptography::ecc::decrypt(key, decrypted))
				{
					throw std::runtime_error("failed to decrypt");
				}

				return decrypted;
			});

			gsc::function::add("ecc::import_key", [](const std::string& data)
			{
				utils::cryptography::ecc::key key;
				key.deserialize(utils::string::hex_to_bin(data));
				keys.emplace_back(key);
				return keys.size() - 1;
			});

			gsc::function::add("ecc::export_public_key", [](const std::uint32_t key_index)
			{
				if (key_index < 0 || key_index >= keys.size())
				{
					throw std::runtime_error("invalid key");
				}

				const auto& key = keys.at(key_index);
				return utils::string::bin_to_hex(key.serialize(PK_PUBLIC));
			});

			gsc::function::add("ecc::export_private_key", [](const std::uint32_t key_index)
			{
				if (key_index < 0 || key_index >= keys.size())
				{
					throw std::runtime_error("invalid key");
				}

				const auto& key = keys.at(key_index);
				return utils::string::bin_to_hex(key.serialize(PK_PRIVATE));
			});

			gsc::function::add("aes::encrypt", [](const std::string& data, const std::string& iv, const std::string& key)
			{
				return utils::string::bin_to_hex(utils::cryptography::aes::encrypt(data, 
					utils::string::hex_to_bin(iv), 
					utils::string::hex_to_bin(key))
					);
			});

			gsc::function::add("aes::decrypt", [](const std::string& data, const std::string& iv, const std::string& key)
			{
				return utils::cryptography::aes::decrypt(utils::string::hex_to_bin(data),
					utils::string::hex_to_bin(iv), 
					utils::string::hex_to_bin(key));
			});

			gsc::function::add("cryptography::random_bytes", [](const int len)
			{
				return utils::string::bin_to_hex(utils::cryptography::random::get_data(len));
			});
		}
	};
}

REGISTER_COMPONENT(cryptography::component)
