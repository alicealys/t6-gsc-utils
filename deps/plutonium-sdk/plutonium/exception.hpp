#pragma once

namespace plutonium::sdk
{
    class exception : public std::exception
    {
    public:
        using std::exception::exception;
    };
}
