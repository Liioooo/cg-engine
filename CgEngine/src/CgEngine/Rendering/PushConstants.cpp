#include "PushConstants.h"
#include "glad/glad.h"

namespace CgEngine {

    PushConstants::~PushConstants() {
        if (data) {
            free(data);
        }
    }

    void PushConstants::setData(const void* newData, size_t size) {
        CG_ASSERT(this->data != nullptr, "PushConstants: your forgot to call init<T>() first!");
        CG_ASSERT(size == this->dataSize, "PushConstants::setData: Data size does not match the PushConstants size.")
        memcpy(this->data, newData, size);
    }

    void PushConstants::setOpenGLUniformInt(uint32_t location, int value) {
        glUniform1i(location, value);
    }
}
