#pragma once

#ifndef PLUTONIUM_SDK
#define PLUTONIUM_SDK

#ifndef __GNUC__
#pragma warning(push)
#pragma warning(disable: 4229)
#endif

#ifdef __GNUC__
#define PLUTONIUM_CALLBACK __attribute__((__cdecl__))
#else
#define PLUTONIUM_CALLBACK __cdecl
#endif

#ifdef __GNUC__
#define PLUTONIUM_INTERNAL_CALLBACK __attribute__((__thiscall__))
#else
#define PLUTONIUM_INTERNAL_CALLBACK __thiscall
#endif

#ifdef PLUTONIUM_LIB
#define PLUTONIUM_API extern "C" __declspec(dllimport)
#else
#define PLUTONIUM_API extern "C" __declspec(dllexport)
#endif

#include <memory>
#include <string>
#include <cstdint>
#include <exception>
#include <functional>

#ifndef PLUTONIUM_SDK_VERSION
#define PLUTONIUM_SDK_VERSION 1u
#endif

#include "plutonium/exception.hpp"
#include "plutonium/unique_ptr.hpp"

#if PLUTONIUM_SDK_VERSION == 1u
#include "plutonium/v1/interface.hpp"
namespace plutonium::sdk
{
    using iinterface = v1::iinterface;
    namespace interfaces = v1::interfaces;
    namespace types = v1::types;
}
#else
#error Unknown SDK version specified!
#endif

namespace plutonium::sdk
{
    enum class game
    {
        iw5,
        t4,
        t5,
        t6,
    };

    class plugin
    {
    public:
        virtual ~plugin() = default;

        //
        // plugin information
        //
        virtual std::uint32_t PLUTONIUM_INTERNAL_CALLBACK plugin_version()
        {
            return PLUTONIUM_SDK_VERSION;
        }
        virtual const char* PLUTONIUM_INTERNAL_CALLBACK plugin_name() = 0;
        virtual bool PLUTONIUM_INTERNAL_CALLBACK is_game_supported([[maybe_unused]] game game)
        {
            return true;
        }

        //
        // plugin initialization & shutdown
        // 
        virtual void PLUTONIUM_INTERNAL_CALLBACK on_startup(iinterface* interface_ptr, game game) = 0;
        virtual void PLUTONIUM_INTERNAL_CALLBACK on_shutdown() = 0;
    };
}

#ifndef __GNUC__
#pragma warning(pop)
#endif

#endif
