#include "OpenGLDescriptorSet.h"
#include "OpenGLUniformBuffer.h"
#include "Asserts.h"
#include "OpenGLShaderStorageBuffer.h"
#include "OpenGLImmutableShaderStorageBuffer.h"
#include "OpenGLTexture2D.h"
#include "OpenGLAttachment.h"
#include "OpenGLDescriptorSetLayout.h"
#include "OpenGLTextureCube.h"
#include "OpenGLVertexBuffer.h"
#include "OpenGLHelpers.h"

namespace CgEngine {

    OpenGLDescriptorSet::OpenGLDescriptorSet(const DescriptorSetSpecification& spec) : specification(spec), ready(true) {
        CG_ASSERT(spec.layout != nullptr, "DescriptorSet: Layout is null!")
        CG_ASSERT(validateSpecification(spec), "DescriptorSet: Specification is not valid!")
    }

    OpenGLDescriptorSet::OpenGLDescriptorSet(const DescriptorSetLayout *layout) {
        CG_ASSERT(layout != nullptr, "DescriptorSet: Layout is null!")
        specification.layout = layout;
    }

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
        CG_ASSERT(spec.layout == nullptr || spec.layout == specification.layout, "DescriptorSet: Cannot change layout during reconfiguration!")

        auto savedLayout = specification.layout;

        specification = spec;
        specification.layout = savedLayout;
        CG_ASSERT(validateSpecification(specification), "DescriptorSet: Specification is not valid!")

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

    bool OpenGLDescriptorSet::validateSpecification(const DescriptorSetSpecification &spec) const {
        const DescriptorSetLayoutSpecification& layoutSpec = static_cast<const OpenGLDescriptorSetLayout*>(spec.layout)->getSpecification();

        auto validateBindings = [](const auto& layoutBindings, const auto& providedBindings, auto getBindingPoint) {
            std::unordered_set<uint32_t> expected(layoutBindings.begin(),layoutBindings.end());

            std::unordered_set<uint32_t> provided;

            for (const auto& binding : providedBindings) {
                uint32_t bindingPoint = getBindingPoint(binding);

                // duplicate binding
                if (!provided.insert(bindingPoint).second) {
                    return false;
                }

                // binding not present in layout
                if (!expected.contains(bindingPoint)) {
                    return false;
                }
            }

            // every layout binding must be provided exactly once
            return provided == expected;
        };

        if (!validateBindings(layoutSpec.uboBindingPoints, spec.uboBindings, [](const auto& b) { return b.bindingPoint; })) {
            CG_LOGGING_ERROR("DescriptorSet: UBO bindings do not match layout!")
            return false;
        }

        {
            std::vector<uint32_t> ssboBindings;
            ssboBindings.reserve(spec.ssboBindings.size() + spec.immutableSsboBindings.size() + spec.vertexBufferSSBOBindings.size());

            for (const auto& b : spec.ssboBindings)
                ssboBindings.push_back(b.bindingPoint);

            for (const auto& b : spec.immutableSsboBindings)
                ssboBindings.push_back(b.bindingPoint);

            for (const auto& b : spec.vertexBufferSSBOBindings)
                ssboBindings.push_back(b.bindingPoint);

            if (!validateBindings(layoutSpec.ssboBindingPoints, ssboBindings, [](const auto& binding) { return binding; })) {
                CG_LOGGING_ERROR("DescriptorSet: SSBO bindings do not match layout!")
                return false;
            }
        }

        {
            std::vector<uint32_t> textureBindings;
            textureBindings.reserve(spec.texture2DBindings.size() + spec.textureCubeBindings.size() + spec.attachmentTextureBindings.size());

            for (const auto& b : spec.texture2DBindings)
                textureBindings.push_back(b.bindingPoint);

            for (const auto& b : spec.textureCubeBindings)
                textureBindings.push_back(b.bindingPoint);

            for (const auto& b : spec.attachmentTextureBindings)
                textureBindings.push_back(b.bindingPoint);

            if (!validateBindings(layoutSpec.texture2DAndAttachmentBindingPoints, textureBindings, [](uint32_t binding) { return binding; })) {
                CG_LOGGING_ERROR("DescriptorSet: Texture2D/Attachment bindings do not match layout!")
                return false;
            }
        }

        {
            std::vector<uint32_t> imageBindings;
            imageBindings.reserve(spec.imageCubeBindings.size() + spec.attachmentImageBindings.size());

            for (const auto& b : spec.imageCubeBindings)
                imageBindings.push_back(b.bindingPoint);

            for (const auto& b : spec.attachmentImageBindings)
                imageBindings.push_back(b.bindingPoint);

            if (!validateBindings(layoutSpec.imageBindingPoints, imageBindings, [](uint32_t binding) { return binding; })) {
                CG_LOGGING_ERROR("DescriptorSet: Image bindings do not match layout!")
                return false;
            }
        }

        return true;
    }
}
