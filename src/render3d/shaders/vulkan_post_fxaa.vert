#version 450

layout(location = 0) out vec2 outUv;

void main()
{
    vec2 clipPos;
    clipPos.x = (gl_VertexIndex == 2) ? 3.0 : -1.0;
    clipPos.y = (gl_VertexIndex == 1) ? 3.0 : -1.0;

    gl_Position = vec4(clipPos, 0.0, 1.0);
    outUv = vec2((clipPos.x + 1.0) * 0.5, 1.0 - ((clipPos.y + 1.0) * 0.5));
}