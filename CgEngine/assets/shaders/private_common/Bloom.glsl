vec3 upsampleBloom(sampler2D tex, vec2 texCoord) {
    vec2 texSize = 1.0f / vec2(textureSize(tex, 0));
    float x = texSize.x;
    float y = texSize.y;

    // a - b - c
    // d - e - f
    // g - h - i

    vec3 a = texture(tex, vec2(texCoord.x - x, texCoord.y + y)).rgb;
    vec3 b = texture(tex, vec2(texCoord.x,     texCoord.y + y)).rgb;
    vec3 c = texture(tex, vec2(texCoord.x + x, texCoord.y + y)).rgb;

    vec3 d = texture(tex, vec2(texCoord.x - x, texCoord.y)).rgb;
    vec3 e = texture(tex, vec2(texCoord.x,     texCoord.y)).rgb;
    vec3 f = texture(tex, vec2(texCoord.x + x, texCoord.y)).rgb;

    vec3 g = texture(tex, vec2(texCoord.x - x, texCoord.y - y)).rgb;
    vec3 h = texture(tex, vec2(texCoord.x,     texCoord.y - y)).rgb;
    vec3 i = texture(tex, vec2(texCoord.x + x, texCoord.y - y)).rgb;

    vec3 result = e * 4.0f;
    result += (b+d+f+h) * 2.0f;
    result += (a+c+g+i);
    result *= 1.0f / 16.0f;

    return result;
}