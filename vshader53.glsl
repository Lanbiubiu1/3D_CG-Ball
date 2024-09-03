/* 
File Name: "vshader53.glsl":
Vertex shader:
  - Per vertex shading for a single point light source;
    distance attenuation is Yet To Be Completed.
  - Entire shading computation is done in the Eye Frame.
*/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
              //      due to different settings of the default GLSL version

in  vec4 vPosition;
in  vec3 vNormal;
in  vec2 vTexcoord;

out vec4 color;
out float fog_dis;
out vec2 texcoord;
out vec2 slicetexcoord;

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat3 Normal_Matrix;
uniform vec4 LightPosition;   // Must be in Eye Frame
uniform float Shininess;

uniform float ConstAtt;  // Constant Attenuation
uniform float LinearAtt; // Linear Attenuation
uniform float QuadAtt;   // Quadratic Attenuation

uniform vec4 material_ambient;
uniform	vec4 material_diffuse;
uniform	vec4 material_specular;
uniform vec4 input_ambient;
uniform	vec4 input_diffuse;
uniform	vec4 input_specular;

uniform vec4 spot_direction;
uniform float floor_flag;
uniform float sphere_flag;
uniform float spot_flag;
uniform float point_flag;
uniform float spot_exp;
uniform float spot_ang;

uniform int texcoordmode;
uniform int texcoordspace;
uniform int useTexture;
uniform int latticeMode;

void main()
{
    // Transform vertex  position into eye coordinates
    vec3 pos = (ModelView * vPosition).xyz;
    vec3 N = normalize(Normal_Matrix * vNormal);
    vec4 global_ambient_light = vec4(1.0, 1.0, 1.0, 1.0);
    vec3 light_direction = vec3(0.1, 0.0, −1.0);
    vec4 light_ambient = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 light_diffuse = vec4(0.8, 0.8, 0.8, 1.0);
    vec4 light_specular = vec4(0.2, 0.2, 0.2, 1.0);

    vec4 ambient_product = light_ambient * material_ambient;
    vec4 diffuse_product = light_diffuse * material_diffuse;
	vec4 specular_product = light_specular * material_specular;

    vec3 ini_L = normalize(light_direction);
	vec3 ini_E = normalize( -pos );
    vec3 ini_H = normalize( ini_L + ini_E );

    
    

    vec4 ambient_t = ambient_product;
    vec4 diffuse_t = max(dot(ini_L, N), 0.0) * diffuse_product;
    vec4 specular_t = vec4(pow(max(dot(N,ini_H), 0.0), Shininess)) * specular_product;
    if (dot(ini_L, N) < 0.0){
        specular_t = vec4(0.0, 0.0, 0.0, 1.0);
    }


    vec4 ini_color = 1.0 * (ambient_t + diffuse_t + specular_t);

    vec3 L = normalize( LightPosition.xyz - pos );
    vec3 E = normalize( -pos );
    vec3 H = normalize( L + E );

    // Transform vertex normal into eye coordinates
      // vec3 N = normalize( ModelView*vec4(vNormal, 0.0) ).xyz;
    

// YJC Note: N must use the one pointing *toward* the viewer
//     ==> If (N dot E) < 0 then N must be changed to -N
//
   

   

/*--- To Do: Compute attenuation ---*/
float attenuation = 1.0; 
float distance = length(LightPosition.xyz - pos);
attenuation = 1 / (ConstAtt + LinearAtt * distance + QuadAtt * distance * distance);

 // Compute terms in the illumination equation
    vec4 ambient = input_ambient * material_ambient;

    float d = max( dot(L, N), 0.0 );
    vec4  diffuse = d * input_diffuse * material_diffuse;

    float s = pow( max(dot(N, H), 0.0), Shininess );
    vec4  specular = s * input_specular * material_specular;
    
    if( dot(L, N) < 0.0 ) {
	specular = vec4(0.0, 0.0, 0.0, 1.0);
    } 

    gl_Position = Projection * ModelView * vPosition;

/*--- attenuation below must be computed properly ---*/
    vec4 colorpoint = attenuation * (ambient + diffuse + specular);


   vec3 spot_light = -L;
   vec3 spot_l_f= normalize( spot_direction.xyz-LightPosition.xyz );

   float spot_att=pow(dot(spot_light,spot_l_f),spot_exp);

   vec4 colorspot=spot_att*colorpoint;
   
   if ( dot(spot_light,spot_l_f) < spot_ang)
   {colorspot=vec4(0,0,0,1);}

    fog_dis=length(pos);


   if (spot_flag==1.0)
   {
   color=ini_color+global_ambient_light*material_ambient+colorspot;
   }
   else
   {
   color=ini_color+global_ambient_light*material_ambient+colorpoint;
   }

    texcoord=vTexcoord;

    if(texcoordspace==1)
        pos=vPosition.xyz;

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
