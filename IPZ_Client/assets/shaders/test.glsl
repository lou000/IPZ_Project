//type vertex
#version 330 core
precision mediump float;

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

float map(float value, float inMin, float inMax, float outMin, float outMax) {
  return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

void main() {
  vec2 uv = gl_FragCoord.xy / u_resolution;

  vec3 color = vec3(uv.x, 0.0, uv.y);

  float aspect = u_resolution.x / u_resolution.y;
  uv.x *= aspect;

  vec2 mouse = u_mouse;
  mouse.x *= aspect;

  float radius = map(sin(u_time), -1.0, 1.0, 0.25, 0.3);

  if (distance(uv, mouse) < radius){
    color.r = 1.0 - color.r;
    color.b = 1.0 - color.b;
  }

  gl_FragColor = vec4(color, 1.0);
}
