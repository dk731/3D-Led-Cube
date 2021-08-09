#version 330 core
layout (location = 0) out vec3 out_col;

in float id;
in vec3 color;

void main()
{
  gl_FragDepth = id;
  out_col = color;
}