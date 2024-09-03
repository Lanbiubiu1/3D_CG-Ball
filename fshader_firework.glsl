#version 150


in vec4 color;
in float y_pos;

out vec4 fColor;

void main() {
    if (y_pos < 0.1) {
        discard;
    }
    fColor = color;
}