#pragma once

namespace CgEngine {

    class ImmutableShaderStorageBuffer {
    public:
        ImmutableShaderStorageBuffer() = default;

        virtual ~ImmutableShaderStorageBuffer() = default;

        ImmutableShaderStorageBuffer(ImmutableShaderStorageBuffer&& other) noexcept = default;
        ImmutableShaderStorageBuffer& operator=(ImmutableShaderStorageBuffer&& other) noexcept = default;

        ImmutableShaderStorageBuffer(ImmutableShaderStorageBuffer& other) = delete;
        ImmutableShaderStorageBuffer& operator=(ImmutableShaderStorageBuffer& other) = delete;

        virtual bool isReady() const = 0;
        virtual size_t getSize() const = 0;
    };

}
