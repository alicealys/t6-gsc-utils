#pragma once

namespace plutonium::sdk
{
    template <typename T>
    using unique_ptr_deleter = std::function<void(T*)>;

    template <typename T>
    using unique_ptr = std::unique_ptr<T, unique_ptr_deleter<T>>;
}
