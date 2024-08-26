#pragma once

namespace CgEngine {

    class ShaderStorageBuffer {
    public:
        explicit ShaderStorageBuffer();
        ~ShaderStorageBuffer();

        void setData(const void* data, size_t size);
        void setSubData(size_t offset, const void* data, size_t size);
        void bind(uint32_t binding) const;

    private:
        uint32_t bufferId;

    };

}
