#pragma once

namespace plutonium::sdk::v1::interfaces
{
    class callbacks
    {
    public:
        typedef void(PLUTONIUM_CALLBACK* on_dvar_init_callback)();
        typedef void (PLUTONIUM_CALLBACK* on_after_dvar_init_callback)();
        typedef void (PLUTONIUM_CALLBACK* on_game_init_callback)(int, int);
        typedef void (PLUTONIUM_CALLBACK* on_game_shutdown_callback)(int);
        typedef void (PLUTONIUM_CALLBACK* on_player_pre_connect_callback)(unsigned int);
        typedef void (PLUTONIUM_CALLBACK* on_player_connect_callback)(unsigned int);
        typedef void (PLUTONIUM_CALLBACK* on_player_disconnect_callback)(unsigned int);
        typedef void (PLUTONIUM_CALLBACK* on_scripts_load_callback)();
        typedef void (PLUTONIUM_CALLBACK* on_scripts_execute_callback)();

        virtual void PLUTONIUM_INTERNAL_CALLBACK on_dvar_init(on_dvar_init_callback callback) = 0;
        virtual void PLUTONIUM_INTERNAL_CALLBACK on_after_dvar_init(on_after_dvar_init_callback callback) = 0;
        virtual void PLUTONIUM_INTERNAL_CALLBACK on_game_init(on_game_init_callback callback) = 0;
        virtual void PLUTONIUM_INTERNAL_CALLBACK on_game_shutdown(on_game_shutdown_callback callback) = 0;
        virtual void PLUTONIUM_INTERNAL_CALLBACK on_player_pre_connect(on_player_pre_connect_callback callback) = 0;
        virtual void PLUTONIUM_INTERNAL_CALLBACK on_player_connect(on_player_connect_callback callback) = 0;
        virtual void PLUTONIUM_INTERNAL_CALLBACK on_player_disconnect(on_player_disconnect_callback callback) = 0;
        virtual void PLUTONIUM_INTERNAL_CALLBACK on_scripts_load(on_scripts_load_callback callback) = 0;
        virtual void PLUTONIUM_INTERNAL_CALLBACK on_scripts_execute(on_scripts_execute_callback callback) = 0;
    };
}
