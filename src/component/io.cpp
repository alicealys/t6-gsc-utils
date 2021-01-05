#include <stdafx.hpp>

namespace io
{
	void init()
	{
        function::add("printf", 1, 2, []()
        {
            const auto str = game::get<const char*>(0);

            printf("%s\n", str);
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

        function::add("fgets", 3, 3, []()
        {
            const auto n = game::get<int>(0);
            const auto handle = game::get_ptr<FILE*>(1);

            if (handle)
            {
                fseek(handle, 0, SEEK_END);
                const auto length = ftell(handle);

                fseek(handle, 0, SEEK_SET);
                char* buffer = (char*)calloc(length, sizeof(char));

                const auto str = fgets(buffer, n, handle);

                game::add(str);
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

                buffer[length] = '\0';

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
