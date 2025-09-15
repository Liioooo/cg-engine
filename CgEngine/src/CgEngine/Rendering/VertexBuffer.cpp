#include "VertexBuffer.h"
#include "Helpers.h"

namespace CgEngine {

    VertexBufferElement::VertexBufferElement(ShaderDataType dataType, bool normalized) : dataType(dataType), normalized(normalized), size(Helpers::getSizeForShaderDataType(dataType)) {}

}
