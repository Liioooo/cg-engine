#include "OpenGLDescriptorSet.h"
#include "OpenGLUniformBuffer.h"
#include "Asserts.h"
#include "OpenGLShaderStorageBuffer.h"
#include "OpenGLImmutableShaderStorageBuffer.h"
#include "OpenGLTexture2D.h"
#include "OpenGLAttachment.h"
#include "OpenGLTextureCube.h"
#include "OpenGLVertexBuffer.h"
#include "OpenGLHelpers.h"

namespace CgEngine {

    OpenGLDescriptorSet::OpenGLDescriptorSet(const DescriptorSetSpecification& spec) : specification(spec), ready(true) {}

    OpenGLDescriptorSet::OpenGLDescriptorSet(OpenGLDescriptorSet&& other) noexcept : DescriptorSet(std::move(other)), ready(other.ready) {
        specification = std::move(other.specification);
        other.ready = false;
    }

    OpenGLDescriptorSet& OpenGLDescriptorSet::operator=(OpenGLDescriptorSet&& other) noexcept {
        if (this != &other) {
            DescriptorSet::operator=(std::move(other));
            specification = std::move(other.specification);
            other.specification = DescriptorSetSpecification();
            ready = other.ready;
            other.ready = false;
        }
        return *this;
    }

    bool OpenGLDescriptorSet::isReady() const {
        return ready;
    }

    void OpenGLDescriptorSet::recreate() {
        // Nothing to do here for OpenGL, as bind uses specification directly
    }

    void OpenGLDescriptorSet::reconfigure(const DescriptorSetSpecification& spec) {
        specification = spec;
        ready = true;
    }

    const DescriptorSetSpecification& OpenGLDescriptorSet::getSpecification() const {
        CG_ASSERT(ready, "DescriptorSet is not ready!")
        return specification;
    }

    void OpenGLDescriptorSet::bind() const {
        CG_ASSERT(ready, "DescriptorSet is not ready!")

        for (const auto& ubo: specification.uboBindings) {
            glBindBufferBase(GL_UNIFORM_BUFFER, ubo.bindingPoint, static_cast<const OpenGLUniformBuffer*>(ubo.ubo)->getOpenGLHandle());
        }
        for (auto ssbo : specification.ssboBindings) {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssbo.bindingPoint, static_cast<const OpenGLShaderStorageBuffer*>(ssbo.ssbo)->getOpenGLHandle());
        }
        for (auto ssbo : specification.immutableSsboBindings) {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssbo.bindingPoint, static_cast<const OpenGLImmutableShaderStorageBuffer*>(ssbo.ssbo)->getOpenGLHandle());
        }
        for (auto tex2D : specification.texture2DBindings) {
            glBindTextureUnit(tex2D.bindingPoint, static_cast<const OpenGLTexture2D*>(tex2D.texture)->getOpenGLHandle());
        }
        for (auto texCube : specification.textureCubeBindings) {
            glBindTextureUnit(texCube.bindingPoint, static_cast<const OpenGLTextureCube*>(texCube.texture)->getOpenGLHandle());
        }
        for (auto attachment : specification.attachmentTextureBindings) {
            if (attachment.allLayers) {
                glBindTextureUnit(attachment.bindingPoint, static_cast<const OpenGLAttachment*>(attachment.attachment)->getOpenGLHandle());
            } else {
                glBindTextureUnit(attachment.bindingPoint, static_cast<const OpenGLAttachment*>(attachment.attachment)->getOpenGLLayerViewHandle(attachment.layer));
            }
        }
        for (auto imageCubeBinding : specification.imageCubeBindings) {
            glBindImageTexture(imageCubeBinding.bindingPoint, static_cast<const OpenGLTextureCube*>(imageCubeBinding.texture)->getOpenGLHandle(), imageCubeBinding.mipLevel, GL_TRUE, 0, OpenGLHelpers::shaderImageAccessToOpenGL(imageCubeBinding.access), OpenGLHelpers::getOpenGLTextureFormatForImageBind(imageCubeBinding.texture->getFormat()));
        }
        for (auto attachmentImageBinding : specification.attachmentImageBindings) {
            if (attachmentImageBinding.allLayers) {
                glBindImageTexture(attachmentImageBinding.bindingPoint, static_cast<const OpenGLAttachment*>(attachmentImageBinding.attachment)->getOpenGLHandle(), 0, GL_TRUE, 0, OpenGLHelpers::shaderImageAccessToOpenGL(attachmentImageBinding.access), OpenGLHelpers::attachmentTypeToOpenGLInternalFormat(attachmentImageBinding.attachment->getType()));
            } else {
                glBindImageTexture(attachmentImageBinding.bindingPoint, static_cast<const OpenGLAttachment*>(attachmentImageBinding.attachment)->getOpenGLHandle(), 0, GL_FALSE, attachmentImageBinding.layer, OpenGLHelpers::shaderImageAccessToOpenGL(attachmentImageBinding.access), OpenGLHelpers::attachmentTypeToOpenGLInternalFormat(attachmentImageBinding.attachment->getType()));
            }
        }
        for (auto vb : specification.vertexBufferSSBOBindings) {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, vb.bindingPoint, static_cast<const OpenGLVertexBuffer*>(vb.vertexBuffer)->getOpenGLHandle());
        }
    }

}
