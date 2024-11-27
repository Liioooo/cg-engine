#pragma once

namespace CgEngine {

    class ShaderStorageBuffer {
    public:
        explicit ShaderStorageBuffer(bool initBuffer = true);
        ~ShaderStorageBuffer();

        ShaderStorageBuffer(ShaderStorageBuffer&& other) noexcept;
        ShaderStorageBuffer& operator=(ShaderStorageBuffer&& other) noexcept;

        ShaderStorageBuffer(ShaderStorageBuffer& other) = delete;
        ShaderStorageBuffer& operator=(ShaderStorageBuffer& other) = delete;

        bool isReady() const;

        void setData(const void* data, size_t size);
        void setSubData(size_t offset, const void* data, size_t size);
        void bind(uint32_t binding) const;

    private:
        uint32_t bufferId = ~0;

    };

}
