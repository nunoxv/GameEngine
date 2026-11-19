#version 330 core
layout(location = 0) in vec2 aPos;   // unit quad, centred on the origin: [-0.5, 0.5]
layout(location = 1) in vec2 aUV;    // (0,0) = top-left corner of the quad

uniform mat4 uProjection;            // orthographic, pixel coordinates (y down)
uniform mat4 uModel;                 // translation * rotation * scale

out vec2 vUV;

void main()
{
    vUV = aUV;
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
}
