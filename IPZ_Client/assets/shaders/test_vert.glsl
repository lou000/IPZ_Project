//type vertex
#version 110
uniform mat4 MVP;
attribute vec3 vCol;
attribute vec2 vPos;
attribute vec4 uv;

varying vec2 v_uv;
varying vec3 color;
void main()
{
    v_uv = vPos + vec2(0.5);
    gl_Position = MVP * vec4(vPos, 0.0, 0.5);
    // color = vec3(0.2, 0.5, 0.8);
    color = vCol;
};
