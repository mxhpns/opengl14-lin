#version 150

in vec4 pos;
in vec4 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

smooth out vec4 _color;

void main() {
    gl_Position = projection * view * model * pos;
    _color = color;
}
