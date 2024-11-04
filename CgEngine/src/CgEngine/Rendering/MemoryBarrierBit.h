namespace CgEngine {

    // from glad.h
    enum class MemoryBarrierBit : unsigned int {
        VertexAttribArray =  0x00000001, // GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT
        ElementArray =       0x00000002, // GL_ELEMENT_ARRAY_BARRIER_BIT
        Uniform =            0x00000004, // GL_UNIFORM_BARRIER_BIT
        TextureFetch =       0x00000008, // GL_TEXTURE_FETCH_BARRIER_BIT
        ShaderImageAccess =  0x00000020, // GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
        Command =            0x00000040, // GL_COMMAND_BARRIER_BIT
        PixelBuffer =        0x00000080, // GL_PIXEL_BUFFER_BARRIER_BIT
        TextureUpdate =      0x00000100, // GL_TEXTURE_UPDATE_BARRIER_BIT
        BufferUpdate =       0x00000200, // GL_BUFFER_UPDATE_BARRIER_BIT
        ClientMappedBuffer = 0x00004000, // GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT
        Framebuffer =        0x00000400, // GL_FRAMEBUFFER_BARRIER_BIT
        TransformFeedback =  0x00000800, // GL_TRANSFORM_FEEDBACK_BARRIER_BIT
        AtomicCounter =      0x00001000, // GL_ATOMIC_COUNTER_BARRIER_BIT
        ShaderStorage =      0x00002000, // GL_SHADER_STORAGE_BARRIER_BIT
        QueryBuffer =        0x00008000, // GL_QUERY_BUFFER_BARRIER_BIT
        All =                0xFFFFFFFF  // GL_ALL_BARRIER_BITS
    };

    namespace MemoryBarrierUtils {
        unsigned int convertToBitfield(const std::initializer_list<MemoryBarrierBit>& barriers);
    }
}
