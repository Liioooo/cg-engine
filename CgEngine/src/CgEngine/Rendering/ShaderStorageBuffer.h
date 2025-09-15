#pragma once

namespace CgEngine {

    class ShaderStorageBuffer {
    public:
        ShaderStorageBuffer() = default;
        virtual ~ShaderStorageBuffer() = default;

        ShaderStorageBuffer(ShaderStorageBuffer&& other) noexcept = default;
        ShaderStorageBuffer& operator=(ShaderStorageBuffer&& other) noexcept = default;

        ShaderStorageBuffer(ShaderStorageBuffer& other) = delete;
        ShaderStorageBuffer& operator=(ShaderStorageBuffer& other) = delete;

        virtual bool isReady() const = 0;
        virtual size_t getSize() const = 0;

        virtual void setData(const void* data, size_t size) = 0;
        virtual void setSubData(size_t offset, const void* data, size_t size) = 0;
    };

}
