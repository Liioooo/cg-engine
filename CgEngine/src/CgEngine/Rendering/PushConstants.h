#pragma once

#include "Asserts.h"

namespace CgEngine {
    class PushConstants {
    public:
        PushConstants(std::string prefix) : prefix(std::move(prefix)) {};

        virtual ~PushConstants();

        template<typename T>
        void init() {
            dataSize = sizeof(T);
            CG_ASSERT(dataSize <= 128, "PushConstants size exceeds the maximum allowed size of 128 bytes.");
            data = malloc(dataSize);
            memset(data, 0, dataSize);
        }

        template<typename T, typename MemberType>
        void mapUniform(MemberType T::* member, const std::string& name) {
            mapUniformInternal(sizeof(T), std::make_unique<MemberAccessor<T, MemberType>>(member), prefix + "." + name);
        };

        void setData(const void* newData, size_t size);

    protected:
        struct IMemberAccessor {
            virtual ~IMemberAccessor() = default;
            virtual void setUniform(const void* obj, uint32_t location) const = 0;
        };

        template<typename T, typename MemberType>
        struct MemberAccessor : IMemberAccessor {
            MemberType T::* member;
            explicit MemberAccessor(MemberType T::* m) : member(m) {}
            void setUniform(const void* obj, uint32_t location) const override {
                const auto& value = reinterpret_cast<const T*>(obj)->*member;


                if constexpr (std::is_same_v<MemberType, int>) {
                    setOpenGLUniformInt(location, value);
                } else if constexpr (std::is_same_v<MemberType, float>) {

                } else if constexpr (std::is_same_v<MemberType, glm::vec2>) {

                } else if constexpr (std::is_same_v<MemberType, glm::vec3>) {

                } else if constexpr (std::is_same_v<MemberType, glm::vec4>) {

                } else if constexpr (std::is_same_v<MemberType, glm::mat3>) {

                } else if constexpr (std::is_same_v<MemberType, glm::mat4>) {

                } else {
                    CG_LOGGING_ERROR("Unsupported push constant type")
                }
            }
        };

        virtual void mapUniformInternal(size_t structSize, std::unique_ptr<IMemberAccessor> memberAccessor, const std::string& name) = 0;

        void* data = nullptr;
        size_t dataSize = 0;

    private:
        std::string prefix;

        static void setOpenGLUniformInt(uint32_t location, int value);
    };
}
