#pragma once

#include "types/entity.hpp"

#include "interfaces/callbacks.hpp"
#include "interfaces/client_command.hpp"
#include "interfaces/gsc.hpp"
#include "interfaces/logging.hpp"
#include "interfaces/scheduler.hpp"

namespace plutonium::sdk::v1
{
    class iinterface
    {
    public:
        virtual ~iinterface() = default;

        virtual interfaces::callbacks* PLUTONIUM_INTERNAL_CALLBACK callbacks() = 0;
        virtual interfaces::client_command* PLUTONIUM_INTERNAL_CALLBACK client_command() = 0;
        virtual interfaces::gsc* PLUTONIUM_INTERNAL_CALLBACK gsc() = 0;
        virtual interfaces::logging* PLUTONIUM_INTERNAL_CALLBACK logging() = 0;
        virtual interfaces::scheduler* PLUTONIUM_INTERNAL_CALLBACK scheduler() = 0;
    };
}
