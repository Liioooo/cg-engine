#pragma once

namespace CgEngine {

    class UniformBuffer {
    public:
        UniformBuffer() = default;

        virtual ~UniformBuffer() = default;

        UniformBuffer(UniformBuffer&& other) noexcept = default;
        UniformBuffer& operator=(UniformBuffer&& other) noexcept = default;

        UniformBuffer(UniformBuffer& other) = delete;
        UniformBuffer& operator=(UniformBuffer& other) = delete;

        virtual bool isReady() const = 0;
        virtual size_t getSize() const = 0;

        virtual void setData(const void* data, size_t size) = 0;
    };

}
