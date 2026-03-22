#include "VulkanVertexArrayObject.h"

#include "Asserts.h"

namespace CgEngine {
    VulkanVertexArrayObject::~VulkanVertexArrayObject() {
        if (!usingExistingIndexBuffer) {
            delete indexBuffer;
        }

        for (auto* item: vertexBuffers) {
            delete item;
        }
    }

    VulkanVertexArrayObject::VulkanVertexArrayObject(VulkanVertexArrayObject &&other) noexcept : VertexArrayObject(std::move(other)) {
         usingExistingIndexBuffer = other.usingExistingIndexBuffer;
         vertexBuffers = std::move(other.vertexBuffers);
         indexBuffer = other.indexBuffer;
         other.indexBuffer = nullptr;
         other.usingExistingIndexBuffer = false;
    }

    VulkanVertexArrayObject & VulkanVertexArrayObject::operator=(VulkanVertexArrayObject &&other) noexcept {
        if (this != &other) {
            VertexArrayObject::operator=(std::move(other));

            if (!usingExistingIndexBuffer) {
                delete indexBuffer;
            }

            for (auto* item: vertexBuffers) {
                delete item;
            }

            vertexBuffers = std::move(other.vertexBuffers);
            indexBuffer = other.indexBuffer;
            usingExistingIndexBuffer = other.usingExistingIndexBuffer;

            other.indexBuffer = nullptr;
            other.usingExistingIndexBuffer = false;
        }
        return *this;
    }

    void VulkanVertexArrayObject::addVertexBuffer(VertexBuffer *buffer) {
        vertexBuffers.push_back(static_cast<VulkanVertexBuffer*>(buffer));
    }

    void VulkanVertexArrayObject::setIndexBuffer(const IndexBuffer *buffer) {
        indexBuffer = static_cast<const VulkanIndexBuffer*>(buffer);
        usingExistingIndexBuffer = false;
    }

    void VulkanVertexArrayObject::useExistingIndexBuffer(const IndexBuffer *buffer) {
        indexBuffer = static_cast<const VulkanIndexBuffer*>(buffer);
        usingExistingIndexBuffer = true;
    }

    VertexBuffer* VulkanVertexArrayObject::getVertexBuffer(uint32_t index) {
        CG_ASSERT(index < vertexBuffers.size(), "Vertex buffer index out of range");
        return vertexBuffers[index];
    }

    const IndexBuffer* VulkanVertexArrayObject::getIndexBuffer() const {
        return indexBuffer;
    }

    const std::vector<VertexBufferLayout> VulkanVertexArrayObject::getLayout() const {
        std::vector<VertexBufferLayout> layouts;
        layouts.reserve(vertexBuffers.size());
        for (const auto& buffer: vertexBuffers) {
            layouts.push_back(buffer->getLayout());
        }
        return layouts;
    }

    void VulkanVertexArrayObject::bind(const vk::CommandBuffer& commandBuffer) const {
        if (vertexBuffers.size() > 1) {
            std::vector<vk::Buffer> vkBuffers;
            std::vector<vk::DeviceSize> vkOffsets;
            vkBuffers.reserve(vertexBuffers.size());
            vkOffsets.reserve(vertexBuffers.size());

            for (auto& buffer: vertexBuffers) {
                vkBuffers.push_back(buffer->getVulkanBufferHandle());
                vkOffsets.push_back(buffer->getOffsetForCurrentFrame());
            }

            commandBuffer.bindVertexBuffers(0, vertexBuffers.size(), vkBuffers.data(), vkOffsets.data());
        } else {
            vk::DeviceSize offsets[1] = {vertexBuffers[0]->getOffsetForCurrentFrame()};
            commandBuffer.bindVertexBuffers(0, 1, &vertexBuffers[0]->getVulkanBufferHandle(), offsets);
        }

        if (indexBuffer && indexBuffer->hasData()) {
            commandBuffer.bindIndexBuffer(indexBuffer->getVulkanBufferHandle(), 0, indexBuffer->getVulkanIndexType());
        }
    }
}
