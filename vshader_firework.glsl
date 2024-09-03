#version 150

in vec4 vPosition;
in vec4 vColor;
in vec3 vVelocity;


out float y_pos;
out vec4 color;

uniform float t;
uniform float tsub;
uniform mat4 MV;
uniform mat4 Projection;




void main() {
    vec4 vColor4 = vec4(vColor.x, vColor.y, vColor.z, 1.0); 
    float x0 = vPosition.x;
    float y0 = vPosition.y;
    float z0 = vPosition.z;
    float v_x = vVelocity.x;
    float v_y = vVelocity.y;
    float v_z = vVelocity.z;

    v_x = clamp(v_x, -1.0, 1.0);
    v_z = clamp(v_z, -1.0, 1.0);
    v_y = clamp(v_y, 0.0, 2.4);
    float x = x0 + 0.001 * v_x * t;
    float z = z0 + 0.001 * v_z * t;
    float a = -0.0000005;
    y_pos = y0 + 0.001 * v_y * t + 0.5 * a * t * t;

    gl_Position = Projection * MV * vec4(x,y_pos,z, 1.0);
    color = vColor4;
}