#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec4 uUvRect;   // u0, v0, u1, v1 : the sprite-sheet frame (may exceed [0,1] with GL_REPEAT)
uniform vec4 uTint;

void main()
{
    vec2 uv = mix(uUvRect.xy, uUvRect.zw, vUV);
    vec4 c = texture(uTexture, uv) * uTint;
    if (c.a < 0.01)      // colour-keyed (magenta) texels were converted to alpha = 0 at load time
        discard;
    FragColor = c;
}
