#include <Logging.h>
#include "TextureCube.h"
#include "GraphicsObjectsFactory.h"

namespace CgEngine {

    TextureCube* TextureCube::createResource(const std::string& name) {
        CG_LOGGING_WARNING("Creating Resource of Type TextureCube will create a empty Resource! ({0})", name)
        return GraphicsObjectsFactory::createTextureCube();
    }

}
