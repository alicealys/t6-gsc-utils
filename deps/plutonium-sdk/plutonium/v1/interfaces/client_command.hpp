#pragma once

namespace plutonium::sdk::v1::interfaces
{
    class client_command
    {
    public:
        typedef void(PLUTONIUM_CALLBACK* function_callback)(int client_num);

    private:
        virtual void PLUTONIUM_INTERNAL_CALLBACK register_client_command_internal(const char* function, function_callback callback) = 0;

    public:
        void register_client_command(const std::string& function, function_callback callback)
        {
            this->register_client_command_internal(function.c_str(), callback);
        }
    };
}
