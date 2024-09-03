/* 
File Name: "fshader53.glsl":
           Fragment Shader
*/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
              //      due to different settings of the default GLSL version

in  vec4 color;
in float fog_dis;
in vec2 texcoord;
in vec2 slicetexcoord;
uniform int fog_fl;

uniform int useTexture;
uniform int texcoordmode;
uniform int latticeMode;
uniform int isShadow;

uniform sampler1D tex1d;
uniform sampler2D tex2d;

out vec4 fColor;

void main() 
{
    if(latticeMode>0)
    if (fract(4 * slicetexcoord.x) < 0.35 && fract(4 * slicetexcoord.y) < 0.35) discard;

    vec4 texture_color = color;
    if(isShadow==0)
    {
    if(useTexture==1)
    {
        texture_color=texture(tex1d,texcoord.x)*color;
    }
    else if(useTexture==2)
    {
        vec4 c=texture_color=texture(tex2d,texcoord);
        if (texcoordmode > 0)
            if (c.r < 0.9) c = vec4(0.9, 0.1, 0.1, 1.0);
        texture_color=c*color;
    }
    }

    vec4 fog_color = vec4(0.7, 0.7, 0.7, 0.5);

    if (fog_fl == 0){
        fColor = texture_color;
    }else if(fog_fl == 1){
        float x = clamp(fog_dis, 0.0, 18.0);
        fColor = mix(texture_color, fog_color, x/18);
    }
    else if (fog_fl == 2){
        float x = 1/exp(0.09 * fog_dis);
        fColor = mix(texture_color, fog_color, 1-x);
    }
    else if (fog_fl == 3){
        float x = 1/exp(0.09 * 0.09 * fog_dis * fog_dis);
        fColor = mix(texture_color, fog_color, 1-x);
    }
    else{
        fColor=vec4(0,0,0,1);
    }
} 

