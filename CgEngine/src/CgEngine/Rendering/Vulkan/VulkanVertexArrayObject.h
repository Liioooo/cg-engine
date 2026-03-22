#pragma once
#include "VulkanIndexBuffer.h"
#include "VulkanVertexBuffer.h"
#include "Rendering/VertexArrayObject.h"

namespace CgEngine {

    class VulkanVertexArrayObject : public VertexArrayObject {
    public:
        VulkanVertexArrayObject() = default;

        ~VulkanVertexArrayObject() override;

        VulkanVertexArrayObject(VulkanVertexArrayObject&& other) noexcept;
        VulkanVertexArrayObject& operator=(VulkanVertexArrayObject&& other) noexcept;

        VulkanVertexArrayObject(VulkanVertexArrayObject& other) = delete;
        VulkanVertexArrayObject& operator=(VulkanVertexArrayObject& other) = delete;

        void addVertexBuffer(VertexBuffer* buffer) override;
        void setIndexBuffer(const IndexBuffer* buffer) override;
        void useExistingIndexBuffer(const IndexBuffer* buffer) override;
        VertexBuffer* getVertexBuffer(uint32_t index) override;
        const IndexBuffer* getIndexBuffer() const override;
        const std::vector<VertexBufferLayout> getLayout() const override;

        void bind(const vk::CommandBuffer& commandBuffer) const;

    private:
        std::vector<VulkanVertexBuffer*> vertexBuffers{};
        const VulkanIndexBuffer* indexBuffer = nullptr;
        bool usingExistingIndexBuffer = false;
    };

}
