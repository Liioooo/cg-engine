#include <FileSystem.h>
#include <CgEngineSharedUtils/LoaderUtils.h>
#include <Application.h>
#include <Events/MouseButtonPressedEvent.h>
#include "UiCanvas.h"
#include "UiCircle.h"
#include "Rendering/GraphicsObjectsFactory.h"
#include "CgEngineSharedUtils/UIPosUtils.h"

namespace CgEngine {

    UiCanvas::UiCanvas(const pugi::xml_node& canvasNode) {
        width = LoaderUtils::stringToUIPosAndUnit(canvasNode.attribute("width").as_string("100vw"));
        height = LoaderUtils::stringToUIPosAndUnit(canvasNode.attribute("height").as_string("100vh"));

        for (const auto& element: canvasNode.children()) {
            std::string name = element.name();
            if (name == "UiCircle") {
                createElementCircle(element);
            } else if (name == "UiRect") {
                createElementRect(element);
            } else if (name == "UiText") {
                createElementText(element);
            }
        }

        const auto& sceneRenderer = Application::get().getSceneRenderer();

        uiCircleVAO = GraphicsObjectsFactory::createVertexArrayObject();
        auto* uiCircleVertexBuffer = GraphicsObjectsFactory::createVertexBuffer(sizeof(UiCircleVertex) * MAX_UI_VERTICES, VertexBufferUsage::CPUDynamic);
        uiCircleVertexBuffer->setLayout(UI_CIRCLE_VERTEX_BUFFER_LAYOUT);
        uiCircleVAO->addVertexBuffer(uiCircleVertexBuffer);
        uiCircleVAO->useExistingIndexBuffer(sceneRenderer.getUiIndexBuffer());

        uiRectVAO = GraphicsObjectsFactory::createVertexArrayObject();
        auto* uiRectVertexBuffer = GraphicsObjectsFactory::createVertexBuffer(sizeof(UiRectVertex) * MAX_UI_VERTICES, VertexBufferUsage::CPUDynamic);
        uiRectVertexBuffer->setLayout(UI_RECT_VERTEX_BUFFER_LAYOUT);
        uiRectVAO->addVertexBuffer(uiRectVertexBuffer);
        uiRectVAO->useExistingIndexBuffer(sceneRenderer.getUiIndexBuffer());

        uiTextVAO = GraphicsObjectsFactory::createVertexArrayObject();
        auto* uiTextVertexBuffer = GraphicsObjectsFactory::createVertexBuffer(sizeof(UiTextVertex) * MAX_UI_VERTICES, VertexBufferUsage::CPUDynamic);
        uiTextVertexBuffer->setLayout(UI_TEXT_VERTEX_BUFFER_LAYOUT);
        uiTextVAO->addVertexBuffer(uiTextVertexBuffer);
        uiTextVAO->useExistingIndexBuffer(sceneRenderer.getUiIndexBuffer());

        for (auto& item: uiDescriptorSets) {
            item = GraphicsObjectsFactory::createDescriptorSet(sceneRenderer.getUiDescriptorSetLayout());
        }
        for (auto& item: uiTextDescriptorSets) {
            item = GraphicsObjectsFactory::createDescriptorSet(sceneRenderer.getUiTextDescriptorSetLayout());
        }

        AttachmentSpecification attachmentSpec{};
        attachmentSpec.width = 1280;
        attachmentSpec.height = 720;
        attachmentSpec.type = UI_CANVAS_ATTACHMENT_TYPE;
        attachmentSpec.usableAsTexture = true;
        attachmentSpec.textureWrap = TextureWrap::Clamp;
        attachmentSpec.mipMapFiltering = MipMapFiltering::Bilinear;
        attachmentSpec.layerCount = 1;

        uiAttachment = GraphicsObjectsFactory::createAttachment(attachmentSpec);

        DescriptorSetSpecification attachmentSamplerDescriptorSetSpec{};
        attachmentSamplerDescriptorSetSpec.layout = sceneRenderer.getUiCanvasSampleDescriptorSetLayout();
        attachmentSamplerDescriptorSetSpec.attachmentTextureBindings.resize(1);
        attachmentSamplerDescriptorSetSpec.attachmentTextureBindings[0].attachment = uiAttachment;
        attachmentSamplerDescriptorSetSpec.attachmentTextureBindings[0].bindingPoint = 1;
        attachmentSamplerDescriptorSetSpec.attachmentTextureBindings[0].allLayers = true;
        attachmentSamplerDescriptorSet = GraphicsObjectsFactory::createDescriptorSet(attachmentSamplerDescriptorSetSpec);
    }

    UiCanvas::~UiCanvas() {
        delete uiCircleVAO;
        delete uiRectVAO;
        delete uiTextVAO;
        delete attachmentSamplerDescriptorSet;
        delete uiAttachment;

        for (auto& item: uiDescriptorSets) {
            delete item;
        }
        for (auto& item: uiTextDescriptorSets) {
            delete item;
        }
    }

    void UiCanvas::update(uint32_t viewportWidth, uint32_t viewportHeight) {
        if (viewportWidth == 0 && viewportHeight == 0) {
            return;
        }

        uint32_t newPixelWidth = UIPosUtils::convertUIPosToPixels(width, viewportWidth, viewportHeight);
        uint32_t newPixelHeight = UIPosUtils::convertUIPosToPixels(height, viewportWidth, viewportHeight);;

        bool canvasSizeDirty = newPixelWidth != pixelWidth || newPixelHeight != pixelHeight;

        if (canvasSizeDirty) {
            if (GraphicsObjectsFactory::getGraphicsAPI() == GraphicsAPI::Vulkan) {
                uiProjectionMatrix = glm::ortho(0.0f, static_cast<float>(newPixelWidth), static_cast<float>(newPixelHeight), 0.0f);
            } else {
                uiProjectionMatrix = glm::ortho(0.0f, static_cast<float>(newPixelWidth), 0.0f, static_cast<float>(newPixelHeight));
            }
            uiAttachment->resize(newPixelWidth, newPixelHeight);
            attachmentSamplerDescriptorSet->recreate();
        }

        pixelWidth = newPixelWidth;
        pixelHeight = newPixelHeight;

        bool vertexBuffersNeedsUpdate = canvasSizeDirty;

        for (const auto& element: uiElements) {
            vertexBuffersNeedsUpdate |= element.second->update(pixelWidth, pixelHeight, canvasSizeDirty);
        }

        if (vertexBuffersNeedsUpdate || eventStateOrElementsChanged) {
            buildUiDrawInfoQueue();
            buildUiVertexBuffers();
            eventStateOrElementsChanged = false;
        }
    }

    UiCircle* UiCanvas::addUiCircle(const std::string& id) {
        CG_ASSERT(!id.empty(), "ID of UIElement must be set!")
        CG_ASSERT(uiElements.find(id) == uiElements.end(), "ID of UIElement already used!")

        auto element = std::make_unique<UiCircle>();
        UiCircle* ptr = element.get();
        uiElements.insert({id, std::move(element)});
        eventStateOrElementsChanged = true;
        return ptr;
    }

    UiRect* UiCanvas::addUiRect(const std::string& id) {
        CG_ASSERT(!id.empty(), "ID of UIElement must be set!")
        CG_ASSERT(uiElements.find(id) == uiElements.end(), "ID of UIElement already used!")

        auto element = std::make_unique<UiRect>();
        UiRect* ptr = element.get();
        uiElements.insert({id, std::move(element)});
        eventStateOrElementsChanged = true;
        return ptr;
    }

    UiText* UiCanvas::addUiText(const std::string& id) {
        CG_ASSERT(!id.empty(), "ID of UIElement must be set!")
        CG_ASSERT(uiElements.find(id) == uiElements.end(), "ID of UIElement already used!")

        auto element = std::make_unique<UiText>();
        UiText* ptr = element.get();
        uiElements.insert({id, std::move(element)});
        eventStateOrElementsChanged = true;
        return ptr;
    }

    bool UiCanvas::hasUiElement(const std::string& id) {
        return uiElements.find(id) != uiElements.end();
    }

    void UiCanvas::removeUIElement(const std::string& id) {
        if (uiElements.find(id) != uiElements.end()) {
            uiElements.erase(id);
            eventStateOrElementsChanged = true;
        }
    }

    glm::mat4 UiCanvas::getUiProjectionMatrix() const {
        return uiProjectionMatrix;
    }

    Attachment* UiCanvas::getUiAttachment() const {
        return uiAttachment;
    }

    std::vector<UiDrawCommand> UiCanvas::getUiDrawCommands() const {
        std::vector<UiDrawCommand> drawCommands;

        uint32_t zIndex = 0;
        for (const auto& [_, drawInfo]: uiDrawInfoQueue) {
            UiDrawCommand& command = drawCommands.emplace_back();
            command.circleIndexCount = drawInfo.circleIndexCount;
            command.circleVertexCount = drawInfo.circleVertices.size();
            command.circleVAO = uiCircleVAO;

            command.rectIndexCount = drawInfo.rectIndexCount;
            command.rectVertexCount = drawInfo.rectVertices.size();
            command.rectVAO = uiRectVAO;

            command.descriptorSet = uiDescriptorSets[zIndex];

            command.textIndexCount = drawInfo.textIndexCount;
            command.textVertexCount = drawInfo.textVertices.size();
            command.textVAO = uiTextVAO;
            command.textDescriptorSet = uiTextDescriptorSets[zIndex];

            zIndex++;
        }

        return drawCommands;
    }

    glm::ivec2 UiCanvas::getPixelSize() const {
        return glm::ivec2(pixelWidth, pixelHeight);
    }

    const DescriptorSet* UiCanvas::getAttachmentSamplerDescriptorSet() const {
        return attachmentSamplerDescriptorSet;
    }

    void UiCanvas::onEvent(Event& event, glm::vec2 normalizedMousePos) {
        glm::vec2 mousePos = ((normalizedMousePos + 1.0f) / 2.0f) * glm::vec2(static_cast<float>(pixelWidth), static_cast<float>(pixelHeight));

        std::vector<UiElement*> sortedElements;
        sortedElements.reserve(uiElements.size());
        for (const auto& [_, element]: uiElements) {
            const auto elementType = element->getType();
            if (elementType == UIElementType::Circle || elementType == UIElementType::Rect) {
                sortedElements.push_back(element.get());
            }
        }

        std::sort(sortedElements.begin(), sortedElements.end(), [](UiElement* a, UiElement* b) {
            return a->getZIndex() > b->getZIndex();
        });

        UiElement* foundElement = nullptr;
        for (auto* element : sortedElements) {
            if (element->containsPoint(mousePos)) {
                foundElement = element;
                break;
            }
        }

        if (foundElement != currentlyHoveredElement) {
            eventStateOrElementsChanged = true;
        }

        currentlyHoveredElement = foundElement;

        if (currentlyHoveredElement != nullptr && event.getEventType() == EventType::MouseButtonPressed) {
            auto& mouseButtonEvent = static_cast<MouseButtonPressedEvent&>(event);
            if (mouseButtonEvent.getButton() == MouseButton::MouseButtonLeft) {
                currentlyHoveredElement->receiveClickEvent();
            }
        }
    }

    void UiCanvas::clearHoverState() {
        if (currentlyHoveredElement != nullptr) {
            currentlyHoveredElement = nullptr;
            eventStateOrElementsChanged = true;
        }
    }

    void UiCanvas::createElementCircle(const pugi::xml_node& elementNode) {
        auto* element = createElement<UiCircle>(elementNode);

        element->setLineWidth(elementNode.attribute("line-width").as_float(0.0f));
        if (!elementNode.attribute("line-width-hover").empty()) {
            element->setLineWidthHover(elementNode.attribute("line-width-hover").as_float(0.0f));
        }

        element->setLineColor(LoaderUtils::stringTupleToVec4(elementNode.attribute("line-color").as_string("0 0 0 1")));
        if (!elementNode.attribute("line-color-hover").empty()) {
            element->setLineColorHover(LoaderUtils::stringTupleToVec4(elementNode.attribute("line-color-hover").as_string("0 0 0 1")));
        }

        element->setFillColor(LoaderUtils::stringTupleToVec4(elementNode.attribute("fill-color").as_string("0 0 0 1")));
        if (!elementNode.attribute("fill-color-hover").empty()) {
            element->setFillColorHover(LoaderUtils::stringTupleToVec4(elementNode.attribute("fill-color-hover").as_string("0 0 0 1")));
        }

        std::string textureName = elementNode.attribute("texture").as_string("");
        if (!textureName.empty()) {
            std::string texturePath = FileSystem::getAsGamePath(textureName).string();
            auto& resourceManager = Application::get().getResourceManager();
            element->setTexture(resourceManager.getResource<Texture2D>(texturePath));
        }

        auto elDiameter = LoaderUtils::stringToUIPosAndUnit(elementNode.attribute("diameter").as_string("0"));
        element->setDiameter(elDiameter.first, elDiameter.second);
    }

    void UiCanvas::createElementRect(const pugi::xml_node& elementNode) {
        auto* element = createElement<UiRect>(elementNode);

        element->setLineWidth(elementNode.attribute("line-width").as_float(0.0f));
        if (!elementNode.attribute("line-width-hover").empty()) {
            element->setLineWidthHover(elementNode.attribute("line-width-hover").as_float(0.0f));
        }

        element->setLineColor(LoaderUtils::stringTupleToVec4(elementNode.attribute("line-color").as_string("0 0 0 1")));
        if (!elementNode.attribute("line-color-hover").empty()) {
            element->setLineColorHover(LoaderUtils::stringTupleToVec4(elementNode.attribute("line-color-hover").as_string("0 0 0 1")));
        }

        element->setFillColor(LoaderUtils::stringTupleToVec4(elementNode.attribute("fill-color").as_string("0 0 0 1")));
        if (!elementNode.attribute("fill-color-hover").empty()) {
            element->setFillColorHover(LoaderUtils::stringTupleToVec4(elementNode.attribute("fill-color-hover").as_string("0 0 0 1")));
        }

        std::string textureName = elementNode.attribute("texture").as_string("");
        if (!textureName.empty()) {
            auto& resourceManager = Application::get().getResourceManager();
            std::string texturePath = FileSystem::getAsGamePath(textureName).string();

            Texture2DResourceSpecification spec{};
            spec.srgb = true;
            spec.wrap = TextureWrap::Repeat;
            spec.mipMapFiltering = MipMapFiltering::Bilinear;

            element->setTexture(resourceManager.getResource<Texture2D>(texturePath, spec));
        }

        auto elWidth = LoaderUtils::stringToUIPosAndUnit(elementNode.attribute("width").as_string("0"));
        auto elHeight = LoaderUtils::stringToUIPosAndUnit(elementNode.attribute("height").as_string("0"));

        element->setWidth(elWidth.first, elWidth.second);
        element->setHeight(elHeight.first, elHeight.second);
    }

    void UiCanvas::createElementText(const pugi::xml_node& elementNode) {
        auto* element = createElement<UiText>(elementNode);

        auto size = LoaderUtils::stringToUIPosAndUnit(elementNode.attribute("size").as_string("0"));
        element->setSize(size.first, size.second);

        element->setText(elementNode.attribute("text").as_string(""));
        element->setColor(LoaderUtils::stringTupleToVec4(elementNode.attribute("color").as_string("0 0 0 1")));
        element->setUseKerning(elementNode.attribute("kerning").as_bool(true));

        std::string font = elementNode.attribute("font").as_string("");
        CG_ASSERT(!font.empty(), "UIText: Font must be set!")
        element->setFont(font);
    }

    template<typename E>
    E* UiCanvas::createElement(const pugi::xml_node& elementNode) {
        std::string id = elementNode.attribute("id").as_string("");

        CG_ASSERT(!id.empty(), "ID of UIElement must be set!")
        CG_ASSERT(uiElements.find(id) == uiElements.end(), "ID of UIElement already used!")

        auto element = std::make_unique<E>();
        E* ptr = element.get();

        uiElements.insert({id, std::move(element)});

        auto top = LoaderUtils::stringToUIPosAndUnit(elementNode.attribute("top").as_string("-1"));
        auto left = LoaderUtils::stringToUIPosAndUnit(elementNode.attribute("left").as_string("-1"));
        auto bottom = LoaderUtils::stringToUIPosAndUnit(elementNode.attribute("bottom").as_string("-1"));
        auto right = LoaderUtils::stringToUIPosAndUnit(elementNode.attribute("right").as_string("-1"));

        ptr->setTop(top.first, top.second);
        ptr->setLeft(left.first, left.second);
        ptr->setBottom(bottom.first, bottom.second);
        ptr->setRight(right.first, right.second);

        ptr->setXAlignment(LoaderUtils::stringToUIXAlignment(elementNode.attribute("x-align").as_string("center")));
        ptr->setYAlignment(LoaderUtils::stringToUIYAlignment(elementNode.attribute("y-align").as_string("center")));

        ptr->setZIndex(elementNode.attribute("z-index").as_int(0));

        return ptr;
    }

    void UiCanvas::buildUiDrawInfoQueue() {
        uiDrawInfoQueue.clear();

        for (const auto& [_, element]: uiElements) {
            UiDrawInfo& drawInfo = uiDrawInfoQueue[element->getZIndex()];
            bool isElementCurrentlyHovered = (element.get() == currentlyHoveredElement);

            if (element->getType() == UIElementType::Circle) {
                auto* circleElement = static_cast<UiCircle*>(element.get());
                const auto* texture = circleElement->getTexture().get();
                float textureIndex = findDrawInfoTextureIndex(drawInfo, texture);

                for (const auto& v: element->getVertices()) {
                    UiCircleVertex& vertex = drawInfo.circleVertices.emplace_back();
                    vertex.posUV = v;
                    vertex.fillColor = isElementCurrentlyHovered ? circleElement->getFillColorHover() : circleElement->getFillColor();
                    vertex.lineColor = isElementCurrentlyHovered ? circleElement->getLineColorHover() : circleElement->getLineColor();
                    vertex.lineWidth = isElementCurrentlyHovered ? circleElement->getLineWidthHover() : circleElement->getLineWidth();
                    vertex.diameter = circleElement->getPixelDiameter();
                    vertex.textureIndex = textureIndex;
                }
                drawInfo.circleIndexCount += 6;
            } else if (element->getType() == UIElementType::Rect) {
                auto* rectElement = static_cast<UiRect*>(element.get());
                const auto* texture = rectElement->getTexture().get();
                float textureIndex = findDrawInfoTextureIndex(drawInfo, texture);

                for (const auto& v: element->getVertices()) {
                    UiRectVertex& vertex = drawInfo.rectVertices.emplace_back();
                    vertex.posUV = v;
                    vertex.fillColor = isElementCurrentlyHovered ? rectElement->getFillColorHover() : rectElement->getFillColor();
                    vertex.lineColor = isElementCurrentlyHovered ? rectElement->getLineColorHover() : rectElement->getLineColor();
                    vertex.size = rectElement->getSize();
                    vertex.lineWidth = isElementCurrentlyHovered ? rectElement->getLineWidthHover() : rectElement->getLineWidth();
                    vertex.textureIndex = textureIndex;
                }
                drawInfo.rectIndexCount += 6;
            } else if (element->getType() == UIElementType::Text) {
                auto* textElement = static_cast<UiText*>(element.get());
                const auto* fontAtlas = textElement->getFontAtlas();

                float fontAtlasIndex = -1;
                for (uint32_t i = 0; i < drawInfo.filledFontAtlases; i++) {
                    if (drawInfo.fontAtlases[i] == fontAtlas) {
                        fontAtlasIndex = static_cast<float>(i);
                        break;
                    }
                }
                if (fontAtlasIndex < 0.0f) {
                    fontAtlasIndex = static_cast<float>(drawInfo.filledFontAtlases);
                    drawInfo.fontAtlases[drawInfo.filledFontAtlases] = fontAtlas;
                    drawInfo.filledFontAtlases++;
                }

                for (const auto& v: element->getVertices()) {
                    UiTextVertex& vertex = drawInfo.textVertices.emplace_back();
                    vertex.posUV = v;
                    vertex.color = textElement->getColor();
                    vertex.fontAtlasIndex = fontAtlasIndex;
                }

                drawInfo.textIndexCount += textElement->getNumIndices();
            }

            CG_ASSERT(drawInfo.circleIndexCount <= MAX_UI_INDICES, "Cannot render that many UICircles")
            CG_ASSERT(drawInfo.rectIndexCount <= MAX_UI_INDICES, "Cannot render that many UIRects")
            CG_ASSERT(drawInfo.textIndexCount <= MAX_UI_INDICES, "Cannot render that many UIText")
            CG_ASSERT(drawInfo.filledTextureSlots < drawInfo.textureSlots.size(), "Cannot render that many different Textures on a single z-index")
            CG_ASSERT(drawInfo.filledFontAtlases < drawInfo.fontAtlases.size(), "Cannot render that many different Fonts on a single z-index")
            CG_ASSERT(uiDrawInfoQueue.size() < MAX_UI_Z_LAYERS, "Cannot render that many different z-indices")
        }
    }

    float UiCanvas::findDrawInfoTextureIndex(UiDrawInfo& drawInfo, const Texture2D* texture) const {
        float textureIndex = -1;
        if (texture != nullptr) {
            for (uint32_t i = 0; i < drawInfo.filledTextureSlots; i++) {
                if (drawInfo.textureSlots[i] == texture) {
                    textureIndex = static_cast<float>(i);
                    break;
                }
            }
            if (textureIndex < 0.0f) {
                textureIndex = static_cast<float>(drawInfo.filledTextureSlots);
                drawInfo.textureSlots[drawInfo.filledTextureSlots] = texture;
                drawInfo.filledTextureSlots++;
            }
        }
        return textureIndex;
    }

    void UiCanvas::buildUiVertexBuffers() {
        size_t circleOffset = 0;
        size_t rectOffset = 0;
        size_t textOffset = 0;
        uint32_t zIndex = 0;

        for (const auto& [_, drawInfo]: uiDrawInfoQueue) {
            if (drawInfo.circleIndexCount > 0) {
                size_t size = drawInfo.circleVertices.size() * sizeof(UiCircleVertex);
                uiCircleVAO->getVertexBuffer(0)->setSubData(circleOffset, drawInfo.circleVertices.data(), size);
                circleOffset += size;
            }
            if (drawInfo.rectIndexCount > 0) {
                size_t size = drawInfo.rectVertices.size() * sizeof(UiRectVertex);
                uiRectVAO->getVertexBuffer(0)->setSubData(rectOffset, drawInfo.rectVertices.data(), size);
                rectOffset += size;
            }
            if (drawInfo.textIndexCount > 0) {
                size_t size = drawInfo.textVertices.size() * sizeof(UiTextVertex);
                uiTextVAO->getVertexBuffer(0)->setSubData(textOffset, drawInfo.textVertices.data(), size);
                textOffset += size;
            }

            DescriptorSetSpecification uiDescriptorSetSpec{};
            uiDescriptorSetSpec.texture2DBindings.resize(1);
            uiDescriptorSetSpec.texture2DBindings[0].bindingPoint = 0;

            std::vector<const Texture2D*> uiTextureArray{};

            for (uint32_t i = 0; i < drawInfo.filledTextureSlots; i++) {
                uiTextureArray.push_back(drawInfo.textureSlots[i]);
            }
            for (uint32_t i = drawInfo.filledTextureSlots; i < 16; i++) {
                uiTextureArray.push_back(Renderer::getWhiteTexture());
            }

            uiDescriptorSetSpec.texture2DBindings[0].textureArray = std::move(uiTextureArray);
            uiDescriptorSets[zIndex]->reconfigure(uiDescriptorSetSpec);

            DescriptorSetSpecification uiTextDescriptorSetSpec{};
            uiTextDescriptorSetSpec.texture2DBindings.resize(1);
            uiTextDescriptorSetSpec.texture2DBindings[0].bindingPoint = 0;

            std::vector<const Texture2D*> uiTextTextureArray{};

            for (uint32_t i = 0; i < drawInfo.filledFontAtlases; i++) {
                uiTextTextureArray.push_back(drawInfo.fontAtlases[i]);
            }
            for (uint32_t i = drawInfo.filledFontAtlases; i < 4; i++) {
                uiTextTextureArray.push_back(Renderer::getWhiteTexture());
            }
            uiTextDescriptorSetSpec.texture2DBindings[0].textureArray = std::move(uiTextTextureArray);
            uiTextDescriptorSets[zIndex]->reconfigure(uiTextDescriptorSetSpec);

            zIndex++;
        }
    }

}
