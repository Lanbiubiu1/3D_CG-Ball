/***************************
 * File: vshader42.glsl:
 *   A simple vertex shader.
 *
 * - Vertex attributes (positions & colors) for all vertices are sent
 *   to the GPU via a vertex buffer object created in the OpenGL program.
 *
 * - This vertex shader uses the Model-View and Projection matrices passed
 *   on from the OpenGL program as uniform variables of type mat4.
 ***************************/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
              //      due to different settings of the default GLSL version

in  vec3 vPosition;
in  vec4 vColor;
in  vec2 vTexcoord;

out vec4 color;
out vec2 texcoord;
out vec2 slicetexcoord;
out float fog_dis;

uniform mat4 model_view;
uniform mat4 projection;

uniform int texcoordmode;
uniform int texcoordspace;
uniform int useTexture;
uniform int latticeMode;

void main() 
{
vec4 vPosition4 = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);
vec4 vColor4 = vColor;

    // YJC: Original, incorrect below:
    //      gl_Position = projection*model_view*vPosition/vPosition.w;
    vec4 p=model_view * vPosition4;;

    gl_Position = projection * p;

    color = vColor4;
    texcoord=vTexcoord;


    fog_dis=length(p.xyz);

    vec3 pos=vPosition4.xyz;
    if(texcoordspace==2)
        pos=p.xyz;

    if (useTexture == 1) {
        if (texcoordmode == 1)
            texcoord = vec2(2.5 * pos.x);
        else if (texcoordmode == 2) {
            texcoord = vec2(1.5 * (pos.x + pos.y + pos.z));
        }
    } else if (useTexture == 2) {
        if (texcoordmode == 1) {
            texcoord = 0.5 * pos.xy + vec2(0.5);
        } else if (texcoordmode == 2) {
            texcoord = 0.3*vec2(pos.x+pos.y+pos.z,pos.x-pos.y+pos.z);
        }
    }
    if (latticeMode== 1) {
        slicetexcoord = 0.5 * pos.xy + vec2(0.5);
    } else if (latticeMode== 2) {
        slicetexcoord = 0.3 * vec2(pos.x + pos.y + pos.z, pos.x - pos.y + pos.z);
    }
}
