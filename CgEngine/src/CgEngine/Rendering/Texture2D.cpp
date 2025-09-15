#include "Texture2D.h"
#include "GraphicsObjectsFactory.h"

namespace CgEngine {

    Texture2D* Texture2D::createResource(const std::string& name) {
        return createResource(name, {});
    }

    Texture2D* Texture2D::createResource(const std::string& name, const CgEngine::Texture2DResourceSpecification& spec) {
        return GraphicsObjectsFactory::createTexture2D(name, spec.srgb, spec.wrap, spec.mipMapFiltering, spec.anisotropicFiltering, spec.compression);
    }

}
