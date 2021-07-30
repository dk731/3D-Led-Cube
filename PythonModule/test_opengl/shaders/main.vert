#version 330 core
layout (location = 0) in vec3 in_pos;

out vec3 current_pos;

void main()
{
    current_pos = in_pos;
    // gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
    gl_Position = vec4(in_pos.x, in_pos.y + in_pos.z * 16, 0.0, 1.0);
}