#include <stdafx.hpp>

namespace io
{
    namespace
    {
        void replace(std::string& str, const std::string& from, const std::string& to) {
            size_t start_pos = str.find(from);

            if (start_pos == std::string::npos)
            {
                return;
            }

            str.replace(start_pos, from.length(), to);
        }
    }

    void init()
    {
        function::add("date", 1, 1, []()
        {
            const auto fmt = game::get<const char*>(0);

            const auto t = std::time(0);
            char buffer[256];

            std::strftime(buffer, 256, fmt, std::localtime(&t));

            game::add(buffer);
        });

        function::add("time", 0, 0, []()
        {
            const auto now = std::chrono::system_clock::now().time_since_epoch();
            const auto count = std::chrono::duration_cast<std::chrono::seconds>(now).count();

            game::Scr_AddInt(game::SCRIPTINSTANCE_SERVER, count);
        });

        function::add("printf", 1, 2, []()
        {
            auto fmt = game::get<std::string>(0);
            const auto num = game::Scr_GetNumParam(game::SCRIPTINSTANCE_SERVER);

            for (auto i = 1; i < num; i++)
            {
                const auto arg = game::get<const char*>(i);

                replace(fmt, "%s", arg);
            }

            printf("%s\n", fmt.data());
        });

        function::add("va", 1, 2, []()
        {
            auto fmt = game::get<std::string>(0);
            const auto num = game::Scr_GetNumParam(game::SCRIPTINSTANCE_SERVER);

            for (auto i = 1; i < num; i++)
            {
                const auto arg = game::get<const char*>(i);

                replace(fmt, "%s", arg);
            }

            game::add(fmt.data());
        });

        function::add("fremove", 1, 1, []()
        {
            const auto path = game::get<const char*>(0);

            const auto result = std::remove(path);

            game::add(result);
        });

        function::add("fopen", 2, 2, []()
        {
            const auto* path = game::get<const char*>(0);
            const auto* mode = game::get<const char*>(1);

            const auto handle = fopen(path, mode);

            if (handle)
            {
                game::add(handle);
            }
            else
            {
                printf("fopen: Invalid path\n");
            }
        });

        function::add("fgetc", 1, 1, []()
        {
            const auto handle = game::get_ptr<FILE*>(0);

            if (handle)
            {
                const auto c = fgetc(handle);
                const char str[2] = { c, '\0' };

                game::add(str);
            }
            else
            {
                printf("fgetc: Invalid handle\n");

                game::add("");
            }
        });

        function::add("fgets", 2, 2, []()
        {
            const auto handle = game::get_ptr<FILE*>(0);
            const auto n = game::get<int>(1);

            if (handle)
            {
                char* buffer = (char*)calloc(n, sizeof(char));

                fgets(buffer, n, handle);

                game::add(buffer);

                free(buffer);
            }
            else
            {
                printf("fgets: Invalid handle\n");

                game::add("");
            }
        });

        function::add("memset", 2, 2, []()
        {
            const auto addr = game::get<int>(0);
            const auto value = game::get<int>(1);

            utils::hook::set(addr, value);
        });

        function::add("feof", 1, 1, []()
        {
            const auto handle = game::get_ptr<FILE*>(0);

            if (handle)
            {
                game::add(feof(handle));
            }
            else
            {
                printf("feof: Invalid handle\n");

                game::add(false);
            }
        });

        function::add("fclose", 1, 1, []()
        {
            const auto handle = game::get_ptr<FILE*>(0);

            if (handle)
            {
                fclose(handle);
            }
            else
            {
                printf("fclose: Invalid handle\n");
            }
        });

        function::add("fputs", 2, 2, []()
        {
            const auto text = game::get<const char*>(0);
            const auto handle = game::get_ptr<FILE*>(1);

            if (handle)
            {
                fputs(text, handle);
            }
            else
            {
                printf("fputs: Invalid handle\n");
            }
        });

        function::add("fprintf", 2, 2, []()
        {
            const auto text = game::get<const char*>(0);
            const auto handle = game::get_ptr<FILE*>(1);

            if (handle)
            {
                fprintf(handle, text);
            }
            else
            {
                printf("fprintf: Invalid handle\n");
            }
        });

        function::add("fread", 1, 1, []()
        {
            const auto handle = game::get_ptr<FILE*>(0);

            if (handle)
            {
                fseek(handle, 0, SEEK_END);
                const auto length = ftell(handle);

                fseek(handle, 0, SEEK_SET);
                char* buffer = (char*)calloc(length, sizeof(char));

                fread(buffer, sizeof(char), length, handle);

                game::add(buffer);

                free(buffer);
            }
            else
            {
                printf("fread: Invalid handle\n");

                game::add("");
            }
        });
    }
}