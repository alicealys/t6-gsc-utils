#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "gsc.hpp"

// lua/lstrlib.c
#define MAX_FORMAT	32
#define L_FMTFLAGSF	"-+#0 "
#define L_FMTFLAGSX	"-#0"
#define L_FMTFLAGSI	"-+0 "
#define L_FMTFLAGSU	"-0"
#define L_FMTFLAGSC	"-"

namespace string
{
	namespace
	{
		// lua/lstrlib.c
		const char* getformat(const char* strfrmt, char* form)
		{
			const auto len = std::strspn(strfrmt, L_FMTFLAGSF "123456789.") + 1;
			if (len >= MAX_FORMAT - 10)
			{
				throw std::runtime_error("invalid format (too long)");
			}

			*(form++) = '%';
			std::memcpy(form, strfrmt, len * sizeof(char));
			*(form + len) = '\0';
			return strfrmt + len - 1;
		}

		// lua/lstrlib.c
		const char* get_2_digits(const char* s)
		{
			if (isdigit(static_cast<unsigned char>(*s)))
			{
				s++;
				if (isdigit(static_cast<unsigned char>(*s)))
				{
					s++;
				}
			}

			return s;
		}

		// lua/lstrlib.c
		void check_format(const char* form, const char* flags, int precision)
		{
			const char* spec = form + 1;
			spec += std::strspn(spec, flags);
			if (*spec != '0')
			{
				spec = get_2_digits(spec);
				if (*spec == '.' && precision)
				{
					spec++;
					spec = get_2_digits(spec);
				}
			}
			if (!std::isalpha(static_cast<unsigned char>(*spec)))
			{
				throw std::runtime_error(utils::string::va("invalid conversion specification: '%s'", form));
			}
		}

		// partially lua/lstrlib.c
		std::string format_string(const std::string& fmt, const scripting::variadic_args& va)
		{
			std::string buffer{};
			size_t va_index{};
			const char* strfrmt = fmt.data();
			const char* strfrmt_end = strfrmt + fmt.size();

			while (strfrmt < strfrmt_end)
			{
				if (*strfrmt != '%')
				{
					buffer.push_back(*strfrmt++);
				}
				else if (*++strfrmt == '%')
				{
					buffer.push_back(*strfrmt++);
				}
				else
				{
					char form[MAX_FORMAT]{};
					const char* flags = "";
					strfrmt = getformat(strfrmt, form);

					switch (*strfrmt++)
					{
					case 'd':
					case 'i':
						flags = L_FMTFLAGSI;
						goto intcase;
					case 'u':
					case 'p':
						flags = L_FMTFLAGSU;
						goto intcase;
					case 'o':
					case 'x':
					case 'X':
						flags = L_FMTFLAGSX;
					intcase:
						{
							check_format(form, flags, 1);
							const auto value = va[va_index].as<int>();
							buffer.append(utils::string::va(form, value));
							va_index++;
							break;
						}
					case 'f':
					case 'F':
					case 'e':
					case 'E':
					case 'g':
					case 'G':
					{
						check_format(form, L_FMTFLAGSF, 1);
						const auto value = va[va_index].as<float>();
						buffer.append(utils::string::va(form, value));
						va_index++;
						break;
					}
					case 'c':
					{
						const auto value = va[va_index].as<int>();
						check_format(form, L_FMTFLAGSC, 0);
						buffer.append(utils::string::va(form, static_cast<char>(value)));
						va_index++;
						break;
					}
					case 's':
					{
						const auto str = va[va_index].as<std::string>();
						buffer.append(str);
						va_index++;
						break;
					}
					default:
					{
						throw std::runtime_error(utils::string::va("invalid conversion '%s' to 'format'", form));
					}
					}
				}
			}

			return buffer;
		}
	}


	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			gsc::function::add_multiple(format_string, "va", "string::va",
				"formatstring", "string::format", "sprintf");

			gsc::function::add("printf", [](const std::string& fmt, const scripting::variadic_args& va)
			{
				printf("%s", format_string(fmt, va).data());
			});

			gsc::function::add("print", [](const scripting::variadic_args& va)
			{
				std::string buffer{};

				for (const auto& arg : va)
				{
					buffer.append(utils::string::va("%s\t", arg.to_string().data()));
				}

				printf("%s\n", buffer.data());
			});

			gsc::function::add_multiple(utils::string::to_upper, "toupper", "string::to_upper");
			gsc::function::add_multiple(utils::string::to_lower, "tolower", "string::to_lower");

			gsc::function::add("string::is_numeric", utils::string::is_numeric);
			gsc::function::add("string::starts_with", utils::string::starts_with);
			gsc::function::add("string::ends_with", utils::string::ends_with);
			gsc::function::add("string::replace", utils::string::replace);

			gsc::function::add("string::regex_replace", [](const std::string& str, const std::regex& expr,
				const std::string& with)
			{
				return std::regex_replace(str, expr, with);
			});

			gsc::function::add("string::regex_match", [](const std::string& str, const std::regex& expr)
			{
				scripting::array array_match{};
				std::smatch match{};

				if (std::regex_match(str, match, std::regex(expr)))
				{
					for (const auto& s : match)
					{
						array_match.emplace_back((s.str()));
					}
				}

				return array_match;
			});

			gsc::function::add("string::regex_search", [](const std::string& str, const std::regex& expr)
			{
				return std::regex_search(str, expr);
			});
		}
	};
}

REGISTER_COMPONENT(string::component)
