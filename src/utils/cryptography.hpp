#pragma once

#include <string>
#include <tomcrypt.h>
#include "string.hpp"

namespace utils::cryptography
{
	namespace ecc
	{
		class key final
		{
		public:
			key();
			~key();

			key(key&& obj) noexcept;
			key(const key& obj);
			key& operator=(key&& obj) noexcept;
			key& operator=(const key& obj);

			bool is_valid() const;

			ecc_key& get();
			const ecc_key& get() const;

			std::string get_public_key() const;

			void set(const std::string& pub_key_buffer);

			void deserialize(const std::string& key);

			std::string serialize(int type = PK_PRIVATE) const;

			void free();

			bool operator==(key& key) const;

			uint64_t get_hash() const;

		private:
			ecc_key key_storage_{};
		};

		key generate_key(int bits);
		key generate_key(int bits, const std::string& entropy);
		std::string sign_message(const key& key, const std::string& message);
		bool verify_message(const key& key, const std::string& message, const std::string& signature);

		bool encrypt(const key& key, std::string& data);
		bool decrypt(const key& key, std::string& data);
	}

	namespace rsa
	{
		std::string encrypt(const std::string& data, const std::string& hash, const std::string& key);
		std::string decrypt(const std::string& data, const std::string& hash, const std::string& key);
	}

	namespace des3
	{
		std::string encrypt(const std::string& data, const std::string& iv, const std::string& key);
		std::string decrypt(const std::string& data, const std::string& iv, const std::string& key);
	}

	namespace tiger
	{
		std::string compute(const std::string& data, bool hex = false);
		std::string compute(const uint8_t* data, size_t length, bool hex = false);
	}

	namespace aes
	{
		std::string encrypt(const std::string& data, const std::string& iv, const std::string& key);
		std::string decrypt(const std::string& data, const std::string& iv, const std::string& key);
	}

	namespace hmac_sha1
	{
		std::string compute(const std::string& data, const std::string& key);
	}

	namespace sha1
	{
		std::string compute(const std::string& data, bool hex = false);
		std::string compute(const uint8_t* data, size_t length, bool hex = false);
	}

	namespace sha256
	{
		std::string compute(const std::string& data, bool hex = false);
		std::string compute(const uint8_t* data, size_t length, bool hex = false);
	}

	namespace sha512
	{
		std::string compute(const std::string& data, bool hex = false);
		std::string compute(const uint8_t* data, size_t length, bool hex = false);
	}

	namespace md5
	{
		std::string compute(const std::string& data, bool hex = false);
		std::string compute(const uint8_t* data, size_t length, bool hex = false);
	}

	namespace argon2
	{
		template <size_t HashLen = 32, size_t SaltLen = 16, std::uint32_t TCost = 2,
			std::uint32_t MCost = 1 << 6, std::uint32_t Threads = 1>
		std::string compute(const std::string& data, bool hex = false)
		{
			std::uint8_t buffer[HashLen]{};
			std::uint8_t salt[SaltLen]{};

			argon2i_hash_raw(TCost, MCost, Threads, data.data(), data.size(), salt, SaltLen, buffer, HashLen);

			const auto str = std::string{reinterpret_cast<char*>(buffer), HashLen};
			if (hex)
			{
				return string::dump_hex(str, "", false);
			}
			else
			{
				return str;
			}
		}
	}

	namespace base64
	{
		std::string encode(const uint8_t* data, size_t len);
		std::string encode(const std::string& data);
		std::string decode(const std::string& data);
	}

	namespace jenkins_one_at_a_time
	{
		unsigned int compute(const std::string& data);
		unsigned int compute(const char* key, size_t len);
	};

	namespace random
	{
		uint32_t get_integer();
		uint32_t get_integer(const std::uint32_t min, const std::uint32_t max);
		std::string get_challenge();
		void get_data(void* data, size_t size);
		std::string get_data(const size_t size);
	}
}
