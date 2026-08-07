#pragma once

#include <pugixml.hpp>
#include "UiRect.h"
#include "UiCircle.h"
#include "UiText.h"
#include "Rendering/VertexArrayObject.h"
#include "Rendering/DescriptorSet.h"
#include "UIVertexBufferLayouts.h"
#include "Rendering/Attachment.h"
#include "Events/Event.h"

namespace CgEngine {

    struct UiCircleVertex {
        glm::vec4 posUV;
        glm::vec4 lineColor;
        glm::vec4 fillColor;
        float diameter;
        float lineWidth;
        float textureIndex;
    };

    struct UiRectVertex {
        glm::vec4 posUV;
        glm::vec4 lineColor;
        glm::vec4 fillColor;
        glm::vec2 size;
        float lineWidth;
        float textureIndex;
    };

    struct UiTextVertex {
        glm::vec4 posUV;
        glm::vec4 color;
        float fontAtlasIndex;
    };

    struct UiDrawInfo {
        uint32_t circleIndexCount = 0;
        std::vector<UiCircleVertex> circleVertices;
        uint32_t rectIndexCount = 0;
        std::vector<UiRectVertex> rectVertices;
        std::array<const Texture2D*, 16> textureSlots{};
        uint32_t filledTextureSlots = 0;

        std::vector<UiTextVertex> textVertices;
        uint32_t textIndexCount = 0;
        std::array<const Texture2D*, 4> fontAtlases;
        uint32_t filledFontAtlases = 0;
    };

    class UiCanvas {
    public:
        UiCanvas(const pugi::xml_node& canvasNode);
        ~UiCanvas();

        void update(uint32_t viewportWidth, uint32_t viewportHeight);

        UiCircle* addUiCircle(const std::string& id);
        UiRect* addUiRect(const std::string& id);
        UiText* addUiText(const std::string& id);

        bool hasUiElement(const std::string& id);
        void removeUIElement(const std::string& id);

        template<typename U>
        U* getUIElement(const std::string& id) {
            return dynamic_cast<U*>(uiElements.at(id).get());
        }

        glm::mat4 getUiProjectionMatrix() const;
        Attachment* getUiAttachment() const;
        std::vector<UiDrawCommand> getUiDrawCommands() const;
        glm::ivec2 getPixelSize() const;
        const DescriptorSet* getAttachmentSamplerDescriptorSet() const;

        void onEvent(Event& event, glm::vec2 normalizedMousePos);
        void clearHoverState();

    private:
        std::unordered_map<std::string, std::unique_ptr<UiElement>> uiElements;
        uint32_t pixelWidth = 0;
        uint32_t pixelHeight = 0;

        std::pair<float, UIPosUnit> width = {0.0f, UIPosUnit::Pixel};
        std::pair<float, UIPosUnit> height = {0.0f, UIPosUnit::Pixel};

        std::map<uint32_t, UiDrawInfo> uiDrawInfoQueue;
        glm::mat4 uiProjectionMatrix;

        VertexArrayObject* uiCircleVAO;
        VertexArrayObject* uiRectVAO;
        VertexArrayObject* uiTextVAO;

        std::array<DescriptorSet*, MAX_UI_Z_LAYERS> uiDescriptorSets;
        std::array<DescriptorSet*, MAX_UI_Z_LAYERS> uiTextDescriptorSets;
        Attachment* uiAttachment;

        DescriptorSet* attachmentSamplerDescriptorSet;

        UiElement* currentlyHoveredElement = nullptr;
        bool eventStateOrElementsChanged = false;

        void createElementCircle(const pugi::xml_node& elementNode);
        void createElementRect(const pugi::xml_node& elementNode);
        void createElementText(const pugi::xml_node& elementNode);

        template<typename E>
        E* createElement(const pugi::xml_node& elementNode);

        void buildUiDrawInfoQueue();
        float findDrawInfoTextureIndex(UiDrawInfo& drawInfo, const Texture2D* texture) const;
        void buildUiVertexBuffers();
    };
}
