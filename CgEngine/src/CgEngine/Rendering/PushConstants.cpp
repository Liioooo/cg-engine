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

    void PushConstants::setOpenGLUniformFloat(uint32_t location, float value) {
        glUniform1f(location, value);
    }

    void PushConstants::setOpenGLUniformBool(uint32_t location, bool value) {
        glUniform1i(location, value);
    }

    void PushConstants::setOpenGLUniformVec2(uint32_t location, glm::vec2 value) {
        glUniform2f(location, value.x, value.y);
    }

    void PushConstants::setOpenGLUniformVec3(uint32_t location, glm::vec3 value) {
        glUniform3f(location, value.x, value.y, value.z);
    }

    void PushConstants::setOpenGLUniformVec4(uint32_t location, glm::vec4 value) {
        glUniform4f(location, value.x, value.y, value.z, value.w);
    }

    void PushConstants::setOpenGLUniformMat3(uint32_t location, glm::mat3 value) {
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
    }

    void PushConstants::setOpenGLUniformMat4(uint32_t location, glm::mat4 value) {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));

    }
}
