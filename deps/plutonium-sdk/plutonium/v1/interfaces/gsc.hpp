#pragma once

namespace plutonium::sdk::v1::interfaces
{
    class gsc
    {
    public:
        typedef void(PLUTONIUM_CALLBACK* method_callback)(types::entref);
        typedef void(PLUTONIUM_CALLBACK* function_callback)();

    private:
        virtual void PLUTONIUM_INTERNAL_CALLBACK register_method_internal(const char* method, method_callback callback) = 0;
        virtual void PLUTONIUM_INTERNAL_CALLBACK register_function_internal(const char* function, function_callback callback) = 0;

    public:
        void register_method(const std::string& message, method_callback callback)
        {
            this->register_method_internal(message.c_str(), callback);
        }

        void register_function(const std::string& function, function_callback callback)
        {
            this->register_function_internal(function.c_str(), callback);
        }
    };
}
