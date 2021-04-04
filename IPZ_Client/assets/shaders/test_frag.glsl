//type fragment
#version 110
varying vec3 color;
varying vec2 v_uv;
void main()
{

vec2 uv = v_uv;
// Zooms out by a factor of 2.0
uv *= 2.0;
// Shifts every axis by -1.0
uv -= 1.0;

// specify size of border. 0.0 - no border, 1.0 - border occupies the entire space
vec2 borderSize = vec2(1);

// size of rectangle in terms of uv
vec2 rectangleSize = vec2(2.0) - borderSize;

// distance field, 0.0 - point is inside rectangle, 1.0 point is on the far edge of the border.
float distanceField = length(max(abs(uv)-rectangleSize,0.0) / borderSize);

// calculate alpha accordingly to the value of the distance field
float alpha = 1.0 - distanceField;

gl_FragColor = vec4(color, alpha);
};
