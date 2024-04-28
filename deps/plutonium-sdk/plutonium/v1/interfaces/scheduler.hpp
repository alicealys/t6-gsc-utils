#pragma once

namespace plutonium::sdk::v1::interfaces
{
    class scheduler
    {
    public:
        enum thread : unsigned int
        {
            main = 0,
            game = 1,
        };

        enum evaluation : bool
        {
            reschedule = false,
            remove = true
        };

        typedef void(PLUTONIUM_CALLBACK* scheduled_callback)();
        typedef evaluation(PLUTONIUM_CALLBACK* reschedulable_callback)();

        virtual void PLUTONIUM_INTERNAL_CALLBACK delay(scheduled_callback callback, unsigned int milliseconds, thread thread = main) = 0;
        virtual void PLUTONIUM_INTERNAL_CALLBACK every(reschedulable_callback callback, unsigned int milliseconds, thread thread = main) = 0;
        virtual void PLUTONIUM_INTERNAL_CALLBACK on_frame(scheduled_callback callback, thread thread = main) = 0;
        virtual void PLUTONIUM_INTERNAL_CALLBACK once(scheduled_callback callback, thread thread = main) = 0;
    };
}
