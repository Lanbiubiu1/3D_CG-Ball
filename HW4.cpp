#include "Angel-yjc.h"
#include <fstream>
#include <string>
#include <math.h>
#include <vector>
#include <cmath>
using namespace std;

/* global definitions for constants and global image arrays */

#define ImageWidth  64
#define ImageHeight 64
GLubyte Image[ImageHeight][ImageWidth][4];

#define	stripeImageWidth 32
GLubyte stripeImage[4 * stripeImageWidth];

GLuint tex;
GLuint stripeTex;

/*************************************************************
void image_set_up(void):
  generate checkerboard and stripe images.

* Inside init(), call this function and set up texture objects
  for texture mapping.
  (init() is called from main() before calling glutMainLoop().)
***************************************************************/
void image_set_up(void)
{
    int i, j, c;

    /* --- Generate checkerboard image to the image array ---*/
    for (i = 0; i < ImageHeight; i++)
        for (j = 0; j < ImageWidth; j++)
        {
            c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0));

            if (c == 1) /* white */
            {
                c = 255;
                Image[i][j][0] = (GLubyte)c;
                Image[i][j][1] = (GLubyte)c;
                Image[i][j][2] = (GLubyte)c;
            }
            else  /* green */
            {
                Image[i][j][0] = (GLubyte)0;
                Image[i][j][1] = (GLubyte)150;
                Image[i][j][2] = (GLubyte)0;
            }

            Image[i][j][3] = (GLubyte)255;
        }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /*--- Generate 1D stripe image to array stripeImage[] ---*/
    for (j = 0; j < stripeImageWidth; j++) {
        /* When j <= 4, the color is (255, 0, 0),   i.e., red stripe/line.
           When j > 4,  the color is (255, 255, 0), i.e., yellow remaining texture
         */
        stripeImage[4 * j] = (GLubyte)255;
        stripeImage[4 * j + 1] = (GLubyte)((j > 4) ? 255 : 0);
        stripeImage[4 * j + 2] = (GLubyte)0;
        stripeImage[4 * j + 3] = (GLubyte)255;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    /*----------- End 1D stripe image ----------------*/

    /*--- texture mapping set-up is to be done in
          init() (set up texture objects),
          display() (activate the texture object to be used, etc.)
          and in shaders.
     ---*/

} /* end function */

typedef Angel::vec3  color3;
typedef Angel::vec3  point3;
typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

GLuint program53;       /* shader program object id */
GLuint program42;
GLuint programF;
GLuint cube_buffer;   /* vertex buffer object id for cube */
GLuint floor_buffer;  /* vertex buffer object id for floor */
GLuint floor_light_buffer;
GLuint line_buffer;


GLuint quad_buffer;  /* vertex buffer object id for quad*/
GLuint X_buffer;     /* vertex buffer object id for x axis*/
GLuint Y_buffer;     /* vertex buffer object id for y axis*/
GLuint Z_buffer;     /* vertex buffer object id for z axis*/

GLuint sphere_buffer;  /* vertex buffer object id for sphere*/
GLuint sphere_shadow_buffer;
GLuint flatsphere_buffer;
GLuint smoothsphere_buffer;
GLuint firework_buffer;

// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 13.0;

GLfloat angle = 0.0; // rotation angle in degrees
vec4 init_eye(7.0, 3.0, -10.0, 1.0); // initial viewer position
vec4 eye = init_eye;               // current viewer position

int animationFlag = 0; // 1: animation; 0: non-animation. Toggled by key 'a' or 'A'

int cubeFlag = 1;   // 1: solid cube; 0: wireframe cube. Toggled by key 'c' or 'C'
int floorFlag = 1;  // 1: solid floor; 0: wireframe floor. Toggled by key 'f' or 'F'
int sphereFlag = 1; // 1: solid sphere; 0: wireframe sphere.

int solidFlag = 0;
int shadow = 0;
int flatShadow = 0;
int smoothShadow = 0;
int lighting = 0;
int spotlight = 0;
int pointSource = 0;
int fog_fl = 0;
int floor_textured = 0;
int sphere_use_texture = 0;    // 1d or 2d
int sphere_texture_space = 1;  // object or view
int sphere_texture_mode = 1;   // slanted or vertical
int latticeMode = 0;   // lattice
int latticeModeTemp = 1;

point3 rotation_routine[] = { point3(3, 1, 5), point3(-1, 1, -4), point3(3.5, 1, -2.5), point3(3, 1, 5) };
int current_index = 0;
int total = 3;

GLfloat theta = 0.0;
GLfloat delta = 1.6;
point3 current_position = rotation_routine[current_index];

point3* rotation_axis;
point3* vectors;
GLfloat radius = 1.0;
mat4 acc_rotation = Angel::identity();


const int N = 300;

int firework_flag = 0;
float tsub = 0;
float t;
float tmax = 5000.0f;
point3 firework_particle[N];
color3 firework_colors[N];
vec3 firework_v[N];



const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point3 floor_points[floor_NumVertices]; // positions for all vertices
color4 floor_colors[floor_NumVertices]; // colors for all vertices
vec2 floor_texcoords[floor_NumVertices]; // colors for all vertices

point4 floor_light[6];
vec3 floor_normal[6];



point3* sphere_vertice;
color4* sphere_color;
int vertice_cnt; //number of vertice in sphere
int triangle_cnt; // number of trangle in sphere

point3 axis_points[9];
color4 axis_colors[9];

bool mouse_in = false;
bool isRolling = false;
// Vertices of a sphere centered at origin, sides aligned with axes
point3 floor_vertices[4] = {
    point3(5, 0,  8),
    point3(5, 0,  -4),
    point3(-5,  0,  -4),
    point3(-5,  0,  8),
};


point4 floor_light_vertices[4] = {
    point4(5, 0,  8, 1.0),
    point4(5, 0,  -4, 1.0),
    point4(-5,  0,  -4, 1.0),
    point4(-5,  0,  8, 1.0),
};
vec2 floor_texcoords_sb[4] = {
   vec2(3. / 2., 1.2),
   vec2(3. / 2., 0),
   vec2(0,  0),
   vec2(0,  1.2),
};
// RGBA colors
color4 vertex_colors[8] = {
    color4(0.0, 0.0, 0.0,1),  // black
    color4(1.0, 0.0, 0.0,1),  // red
    color4(1.0, 1.0, 0.0,1),  // yellow
    color4(0.0, 1.0, 0.0,1),  // green
    color4(0.0, 0.0, 1.0,1),  // blue
    color4(1.0, 0.0, 1.0,1),  // magenta
    color4(1.0, 1.0, 1.0,1),  // white
    color4(0.0, 1.0, 1.0,1),   // cyan
};
int Index = 0;
void quad(int a, int b, int c, int d)
{
    vec4 u = floor_light_vertices[b] - floor_light_vertices[a];
    vec4 v = floor_light_vertices[d] - floor_light_vertices[a];

    vec3 tmp_normal = -normalize(cross(v, u));
    floor_texcoords[Index] = floor_texcoords_sb[a];
    floor_normal[Index] = tmp_normal; floor_light[Index] = floor_light_vertices[a]; Index++;
    floor_texcoords[Index] = floor_texcoords_sb[b];
    floor_normal[Index] = tmp_normal; floor_light[Index] = floor_light_vertices[b]; Index++;
    floor_texcoords[Index] = floor_texcoords_sb[c];
    floor_normal[Index] = tmp_normal; floor_light[Index] = floor_light_vertices[c]; Index++;
    floor_texcoords[Index] = floor_texcoords_sb[a];
    floor_normal[Index] = tmp_normal; floor_light[Index] = floor_light_vertices[a]; Index++;
    floor_texcoords[Index] = floor_texcoords_sb[c];
    floor_normal[Index] = tmp_normal; floor_light[Index] = floor_light_vertices[c]; Index++;
    floor_texcoords[Index] = floor_texcoords_sb[d];
    floor_normal[Index] = tmp_normal; floor_light[Index] = floor_light_vertices[d]; Index++;
}
void floor_light_set()
{
    Index = 0;
    // quad(1, 0, 3, 2);
    quad(0, 1, 2, 3);

}
//----------------------------------------------------------------------------
 // YJC: This must be a global variable since quad() is called
               //      multiple times and Index should then go up to 36 for
               //      the 36 vertices and colors

vec4 light_position(-14.0, 12.0, -3.0, 1.0);
vec4 normal_plane(0.0, 1.0, 0.0, 0.0);
mat4 shadow_projection = Angel::identity();

void shadowprojecttion() {
    float d = 0;
    float dot = light_position.x * normal_plane.x + light_position.y * normal_plane.y
        + light_position.z * normal_plane.z + d * light_position.w;

    shadow_projection[0][0] = dot - light_position.x * normal_plane.x;
    shadow_projection[0][1] = -light_position.x * normal_plane.y;
    shadow_projection[0][2] = -light_position.x * normal_plane.z;
    shadow_projection[0][3] = -light_position.x * d;

    shadow_projection[1][0] = -light_position.y * normal_plane.x;
    shadow_projection[1][1] = dot - light_position.y * normal_plane.y;
    shadow_projection[1][2] = -light_position.y * normal_plane.z;
    shadow_projection[1][3] = -light_position.y * d;

    shadow_projection[2][0] = -light_position.z * normal_plane.x;
    shadow_projection[2][1] = -light_position.z * normal_plane.y;
    shadow_projection[2][2] = dot - light_position.z * normal_plane.z;
    shadow_projection[2][3] = -light_position.z * d;

    shadow_projection[3][0] = 0;
    shadow_projection[3][1] = 0;
    shadow_projection[3][2] = 0;
    shadow_projection[3][3] = dot;
}
void floor_material() {
    color4 material_ambient(0.2, 0.2, 0.2, 1.0);
    color4 material_diffuse(0.0, 1.0, 0.0, 1.0);
    color4 material_specular(0.0, 0.0, 0.0, 1.0);
    float  shininess = 125;
    glUniform4fv(glGetUniformLocation(program53, "material_ambient"), 1,
        material_ambient);
    glUniform4fv(glGetUniformLocation(program53, "material_diffuse"), 1,
        material_diffuse);
    glUniform4fv(glGetUniformLocation(program53, "material_specular"), 1,
        material_specular);
    glUniform1f(glGetUniformLocation(program53, "Shininess"), shininess);
}

void sphere_material() {
    color4 material_ambient(0.2, 0.2, 0.2, 1.0);
    color4 material_diffuse(1.0, 0.84, 0.0, 1.0);
    color4 material_specular(1.0, 0.84, 0.0, 1.0);
    float  shininess = 125;
    glUniform4fv(glGetUniformLocation(program53, "material_ambient"), 1,
        material_ambient);
    glUniform4fv(glGetUniformLocation(program53, "material_diffuse"), 1,
        material_diffuse);
    glUniform4fv(glGetUniformLocation(program53, "material_specular"), 1,
        material_specular);
    glUniform1f(glGetUniformLocation(program53, "Shininess"), shininess);
}

void point_light(mat4 mv) {
    color4 light_ambient(0, 0, 0, 1.0);
    color4 light_diffuse(1.0, 1.0, 1.0, 1.0);
    color4 light_specular(1.0, 1.0, 1.0, 1.0);

    float const_att = 2.0;
    float linear_att = 0.01;
    float quad_att = 0.001;

    point4 light_position(-14, 12.0, -3, 1.0);
    vec4 light_position_eye = mv * light_position;

    glUniform4fv(glGetUniformLocation(program53, "LightPosition"),
        1, light_position_eye);

    glUniform1f(glGetUniformLocation(program53, "ConstAtt"),
        const_att);
    glUniform1f(glGetUniformLocation(program53, "LinearAtt"),
        linear_att);
    glUniform1f(glGetUniformLocation(program53, "QuadAtt"),
        quad_att);
    glUniform4fv(glGetUniformLocation(program53, "input_ambient"), 1,
        light_ambient);
    glUniform4fv(glGetUniformLocation(program53, "input_diffuse"), 1,
        light_diffuse);
    glUniform4fv(glGetUniformLocation(program53, "input_specular"), 1,
        light_specular);
    glUniform1f(glGetUniformLocation(program53, "point_flag"), pointSource * 1.0);
    glUniform1f(glGetUniformLocation(program53, "spot_flag"), 0.0);
}

void spot_light(mat4 mv)
{
    color4 light_ambient(0, 0, 0, 1.0);
    color4 light_diffuse(1.0, 1.0, 1.0, 1.0);
    color4 light_specular(1.0, 1.0, 1.0, 1.0);
    float const_att = 2.0;
    float linear_att = 0.01;
    float quad_att = 0.001;
    point4 light_position(-14, 12.0, -3, 1.0);

    float spot_exp = 15.0;
    float spot_ang = cos(20.0 * 3.1415926 / 180);

    vec4 light_position_eye = mv * light_position;
    glUniform4fv(glGetUniformLocation(program53, "LightPosition"),
        1, light_position_eye);

    glUniform1f(glGetUniformLocation(program53, "ConstAtt"),
        const_att);
    glUniform1f(glGetUniformLocation(program53, "LinearAtt"),
        linear_att);
    glUniform1f(glGetUniformLocation(program53, "QuadAtt"),
        quad_att);
    glUniform4fv(glGetUniformLocation(program53, "input_ambient"), 1,
        light_ambient);
    glUniform4fv(glGetUniformLocation(program53, "input_diffuse"), 1,
        light_diffuse);
    glUniform4fv(glGetUniformLocation(program53, "input_specular"), 1,
        light_specular);
    glUniform1f(glGetUniformLocation(program53, "point_flag"), 0.0);
    glUniform1f(glGetUniformLocation(program53, "spot_flag"), 1.0);
    point4 spot_direction = mv * vec4(-6.0, 0.0, -4.5, 1.0);
    glUniform4fv(glGetUniformLocation(program53, "spot_direction"), 1,
        spot_direction);
    glUniform1f(glGetUniformLocation(program53, "spot_exp"), spot_exp);
    glUniform1f(glGetUniformLocation(program53, "spot_ang"), spot_ang);

}
void ini_firework(void) {
    srand((unsigned)time(NULL));

    for (int i = 0; i < N; i++) {
        firework_particle[i] = point3(0.0, 0.1, 0.0);

        float r = (float)rand() / RAND_MAX;
        float g = (float)rand() / RAND_MAX;
        float b = (float)rand() / RAND_MAX;

        firework_colors[i] = color3(r, g, b);
        firework_v[i] = vec3(2.0 * ((float)rand() / RAND_MAX - 0.5), 2.4 * ((float)rand() / RAND_MAX), 2.0 * ((float)rand() / RAND_MAX - 0.5));
    }
}


//----------------------------------------------------------------------------
// read input file data

point3* sphere_shadow;
point4* sphere_shadow_color;

point4* sphere_shader_data;
vec3* sphere_flat_shader;
vec3* sphere_smooth_shader;


void ini_flat_shader() {
    int index = 0;

    for (int i = 0; i < triangle_cnt; i++) {
        vec4 p1 = sphere_shader_data[3 * i];
        vec4 p2 = sphere_shader_data[3 * i + 1];
        vec4 p3 = sphere_shader_data[3 * i + 2];

        vec3 normal = normalize(cross((p2 - p1), (p3 - p1)));

        for (int j = 0; j < 3; j++) {
            sphere_flat_shader[index++] = normal;
        }
    }
}

void ini_smooth_shader() {
    for (int i = 0; i < triangle_cnt; i++) {
        for (int j = 0; j < 3; ++j)
        {
            vec4 point = sphere_shader_data[i * 3 + j];
            sphere_smooth_shader[i * 3 + j] = normalize(vec3(point[0], point[1], point[2]));
        }
    }

}

void FilesIn() {

    ifstream fileptr;

    char* filename = new char[2048];
    // char* filename = "sphere.1024";

    int points;
    float placehold[3];

    int p_index = 0; // index holder for  the loop sh

    cout << "Enter filename: sphere.256 or sphere.1024" << endl;
    cin.getline(filename, 2048);
    if (strlen(filename) == 0) {
        cout << "please enter filename" << endl;
        exit(1);
    }

    fileptr.open(filename);
    if (!fileptr) {
        cout << "cannot open file" << filename << endl;
        exit(1);
    }

    fileptr >> triangle_cnt;
    sphere_vertice = new point3[triangle_cnt * 3];
    sphere_color = new point4[triangle_cnt * 3];

    sphere_shadow = new point3[triangle_cnt * 3];
    sphere_shadow_color = new point4[triangle_cnt * 3];

    sphere_shader_data = new point4[triangle_cnt * 3];
    sphere_flat_shader = new point3[triangle_cnt * 3];
    sphere_smooth_shader = new point3[triangle_cnt * 3];


    for (int i = 0; i < triangle_cnt; i++) {
        fileptr >> points;
        for (int j = 0; j < points; j++) {

            fileptr >> placehold[0] >> placehold[1] >> placehold[2];
            sphere_vertice[p_index] = (point3(placehold[0], placehold[1], placehold[2]));
            sphere_shadow[p_index] = (point3(placehold[0], placehold[1], placehold[2]));
            sphere_shader_data[p_index++] = vec4(placehold[0], placehold[1], placehold[2], 1);
        }

    }
    vertice_cnt = 3 * triangle_cnt;
    fileptr.close();
    ini_flat_shader();
    ini_smooth_shader();
}



//----------------------------------------------------------------------------
void axis_line() {
    axis_colors[0] = vertex_colors[1]; axis_points[0] = point3(0, 0.05, 0);
    axis_colors[1] = vertex_colors[1]; axis_points[1] = point3(10, 0.05, 0);
    axis_colors[2] = vertex_colors[1]; axis_points[2] = point3(20, 0.05, 0);

    axis_colors[3] = vertex_colors[5]; axis_points[3] = point3(0, 0.05, 0);
    axis_colors[4] = vertex_colors[5]; axis_points[4] = point3(0, 10, 0);
    axis_colors[5] = vertex_colors[5]; axis_points[5] = point3(0, 20, 0);

    axis_colors[6] = vertex_colors[4]; axis_points[6] = point3(0, 0.05, 0);
    axis_colors[7] = vertex_colors[4]; axis_points[7] = point3(0, 0.05, 10);
    axis_colors[8] = vertex_colors[4]; axis_points[8] = point3(0, 0.05, 20);
}

void spherecolor() {
    for (int i = 0; i < vertice_cnt; i++) {
        sphere_color[i] = color4(1.0, 0.84, 0, 1);
        sphere_shadow_color[i] = color4(0.25, 0.25, 0.25, 0.65);
    }
}


// generate 2 triangles: 6 vertices and 6 colors
void floor()
{
    floor_colors[0] = vertex_colors[3]; floor_points[0] = floor_vertices[0];
    floor_colors[1] = vertex_colors[3]; floor_points[1] = floor_vertices[1];
    floor_colors[2] = vertex_colors[3]; floor_points[2] = floor_vertices[2];

    floor_colors[3] = vertex_colors[3]; floor_points[3] = floor_vertices[0];
    floor_colors[4] = vertex_colors[3]; floor_points[4] = floor_vertices[2];
    floor_colors[5] = vertex_colors[3]; floor_points[5] = floor_vertices[3];
}

void createTexture() {
    image_set_up();

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ImageWidth, ImageHeight, GL_FALSE, GL_RGBA, GL_UNSIGNED_BYTE, Image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenTextures(1, &stripeTex);
    glBindTexture(GL_TEXTURE_1D, stripeTex);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, stripeImageWidth, GL_FALSE, GL_RGBA, GL_UNSIGNED_BYTE, stripeImage);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
}

//----------------------------------------------------------------------------
// OpenGL initialization
void init()
{
    createTexture();

    floor();
    spherecolor();
    axis_line();
    shadowprojecttion();
    floor_light_set();
    //where to rotate
    vectors = new point3[total];
    rotation_axis = new point3[total];

    //calculate unit vectors between three points
    for (int i = 0; i < total; i++) {
        point3 trav;
        point3 start = rotation_routine[i];
        point3 dest = rotation_routine[i + 1];

        trav.x = dest.x - start.x;
        trav.y = dest.y - start.y;
        trav.z = dest.z - start.z;

        float unit = sqrt(trav.x * trav.x + trav.y * trav.y + trav.z * trav.z);
        trav.x = trav.x / unit;
        trav.y = trav.y / unit;
        trav.z = trav.z / unit;

        vectors[i] = trav;
    }

    //calculate the rotation axis. 
    //As the floor is x-z plane, 
    //the rotation axis should be cross product of y axis and rotation direction.
    for (int i = 0; i < total; i++) {
        point3 cross_p;
        point3 current_vector = vectors[i];
        cross_p.x = 1 * current_vector.z - 0 * current_vector.y;
        cross_p.y = 0 * current_vector.x - 0 * current_vector.z;
        cross_p.z = 0 * current_vector.y - 1 * current_vector.x;
        rotation_axis[i] = cross_p;
    }




    // Create and initialize a vertex buffer object for floor, to be used in display()
    glGenBuffers(1, &floor_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors) + sizeof(floor_texcoords),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors),
        floor_colors);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors), sizeof(floor_texcoords),
        floor_texcoords);

    glGenBuffers(1, &floor_light_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_light_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_light) + sizeof(floor_normal) + sizeof(floor_texcoords),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_light), floor_light);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_light),
        sizeof(floor_normal), floor_normal);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_light) + sizeof(floor_normal), sizeof(floor_texcoords),
        floor_texcoords);

    glGenBuffers(1, &line_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, line_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axis_points) + sizeof(axis_colors),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_points), axis_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(axis_points), sizeof(axis_colors),
        axis_colors);

    glGenBuffers(1, &sphere_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertice_cnt * sizeof(point3) + vertice_cnt * sizeof(color4),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertice_cnt * sizeof(point3), sphere_vertice);
    glBufferSubData(GL_ARRAY_BUFFER, vertice_cnt * sizeof(point3), vertice_cnt * sizeof(color4),
        sphere_color);

    glGenBuffers(1, &sphere_shadow_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_shadow_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertice_cnt * sizeof(point3) + vertice_cnt * sizeof(color4),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertice_cnt * sizeof(point3), sphere_shadow);
    glBufferSubData(GL_ARRAY_BUFFER, vertice_cnt * sizeof(point3), vertice_cnt * sizeof(color4),
        sphere_shadow_color);

    glGenBuffers(1, &flatsphere_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, flatsphere_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertice_cnt * sizeof(point4) + vertice_cnt * sizeof(color4),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertice_cnt * sizeof(point4), sphere_shader_data);
    glBufferSubData(GL_ARRAY_BUFFER, vertice_cnt * sizeof(point4),
        vertice_cnt * sizeof(color4), sphere_flat_shader);

    glGenBuffers(1, &smoothsphere_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, smoothsphere_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertice_cnt * sizeof(point4) + vertice_cnt * sizeof(color4),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertice_cnt * sizeof(point4), sphere_shader_data);
    glBufferSubData(GL_ARRAY_BUFFER, vertice_cnt * sizeof(point4),
        vertice_cnt * sizeof(color4), sphere_smooth_shader);

    ini_firework();
    glGenBuffers(1, &firework_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, firework_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point3) * N + sizeof(color3) * N + sizeof(vec3) * N,
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point3) * N, firework_particle);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point3) * N, sizeof(color3) * N,
        firework_colors);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point3) * N + sizeof(color3) * N, sizeof(vec3) * N,
        firework_v);
    

    programF = InitShader("vshader_firework.glsl", "fshader_firework.glsl");

    



    // Load shaders and create a shader program (to be used in display())
    program53 = InitShader("vshader53.glsl", "fshader53.glsl");
    program42 = InitShader("vshader42.glsl", "fshader53.glsl");
    glUseProgram(program42);
    glUniform1i(glGetUniformLocation(program42, "tex2d"), 0);
    glUniform1i(glGetUniformLocation(program42, "tex1d"), 1);
    glUseProgram(program53);
    glUniform1i(glGetUniformLocation(program53, "tex2d"), 0);
    glUniform1i(glGetUniformLocation(program53, "tex1d"), 1);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.529, 0.807, 0.92, 0.0);
    glLineWidth(2.0);
}
//----------------------------------------------------------------------------
// drawObj(buffer, num_vertices):
//   draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices.
//
//model type 0 axis, 1 floor, 2 sphere
void drawObj53(GLuint buffer, int num_vertices, int modelType = 0)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program53, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(0));




    GLuint vNormal = glGetAttribLocation(program53, "vNormal");
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(sizeof(point4) * num_vertices));

    glUniform1i(glGetUniformLocation(program53, "useTexture"), 0);
    glUniform1i(glGetUniformLocation(program53, "texcoordmode"), 0);
    glUniform1i(glGetUniformLocation(program53, "texcoordspace"), 0);
    glUniform1i(glGetUniformLocation(program53, "latticeMode"), 0);
    glUniform1i(glGetUniformLocation(program53, "isShadow"), 0);
    glUniform1i(glGetUniformLocation(program53, "fog_fl"), fog_fl);

    // the offset is the (total) size of the previous vertex attribute array(s)
    if (modelType == 1)
    {
        GLuint vTexcoord = glGetAttribLocation(program53, "vTexcoord");
        glEnableVertexAttribArray(vTexcoord);
        glVertexAttribPointer(vTexcoord, 2, GL_FLOAT, GL_FALSE, 0,
            BUFFER_OFFSET((sizeof(point3) + sizeof(point4)) * num_vertices));
        if (floor_textured)
            glUniform1i(glGetUniformLocation(program53, "useTexture"), 2);
    }
    else if (modelType == 2 || modelType == 3)
    {
        glUniform1i(glGetUniformLocation(program53, "useTexture"), sphere_use_texture);
        glUniform1i(glGetUniformLocation(program53, "texcoordmode"), sphere_texture_mode);
        glUniform1i(glGetUniformLocation(program53, "texcoordspace"), sphere_texture_space);
        glUniform1i(glGetUniformLocation(program53, "latticeMode"), latticeMode);
    }
    if (modelType == 3)
    {
        glUniform1i(glGetUniformLocation(program53, "isShadow"), 1);
    }


    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
       (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vNormal);
}

void drawObj42(GLuint buffer, int num_vertices, int modelType = 0)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program42, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(0));



    GLuint vColor = glGetAttribLocation(program42, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(sizeof(point3) * num_vertices));
    // the offset is the (total) size of the previous vertex attribute array(s)

    glUniform1i(glGetUniformLocation(program42, "useTexture"), 0);
    glUniform1i(glGetUniformLocation(program42, "texcoordmode"), 0);
    glUniform1i(glGetUniformLocation(program42, "texcoordspace"), 0);
    glUniform1i(glGetUniformLocation(program42, "latticeMode"), 0);
    glUniform1i(glGetUniformLocation(program42, "isShadow"), 0);
    glUniform1i(glGetUniformLocation(program42, "fog_fl"), fog_fl);
    if (modelType == 1)
    {
        GLuint vTexcoord = glGetAttribLocation(program42, "vTexcoord");
        glEnableVertexAttribArray(vTexcoord);
        glVertexAttribPointer(vTexcoord, 2, GL_FLOAT, GL_FALSE, 0,
            BUFFER_OFFSET((sizeof(point3) + sizeof(point4)) * num_vertices));
        if (floor_textured)
            glUniform1i(glGetUniformLocation(program42, "useTexture"), 2);
    }
    else if (modelType == 2 || modelType == 3)
    {
        glUniform1i(glGetUniformLocation(program42, "useTexture"), sphere_use_texture);
        glUniform1i(glGetUniformLocation(program42, "texcoordmode"), sphere_texture_mode);
        glUniform1i(glGetUniformLocation(program42, "texcoordspace"), sphere_texture_space);
        glUniform1i(glGetUniformLocation(program42, "latticeMode"), latticeMode);
    }
    if (modelType == 3)
    {
        glUniform1i(glGetUniformLocation(program42, "isShadow"), 1);
    }

    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
       (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
}
void drawF(GLuint buffer, int num_vertices)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(programF, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(programF, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(sizeof(point3) * num_vertices));

    GLuint vVelocity = glGetAttribLocation(programF, "vVelocity");
    glEnableVertexAttribArray(vVelocity);
    glVertexAttribPointer(vVelocity, 3, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(sizeof(point3) * num_vertices + sizeof(color3) * num_vertices));
    // the offset is the (total) size of the previous vertex attribute array(s)

  /* Draw a sequence of geometric lines from the vertex buffer
     (using the attributes specified in each enabled vertex attribute array) */
    glPointSize(3.0);
    glDrawArrays(GL_POINTS, 0, num_vertices);

    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
    glDisableVertexAttribArray(vVelocity);
}



//----------------------------------------------------------------------------
void display(void)
{
    GLuint  model_view_42;
    GLuint  projection_42;
    GLuint  ModelView;
    GLuint  Projection;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, stripeTex);

    vec4    at(0.0, 0.0, 0.0, 1.0);
    vec4    up(0.0, 1.0, 0.0, 0.0);
    vec4    eye_position = eye;

    mat4  mv = LookAt(eye_position, at, up);
    mat4  p = Perspective(fovy, aspect, zNear, zFar);

    if (shadow == 1 && eye.y >= 0) {
        glDepthMask(GL_FALSE);
    }

    if (!lighting) {
        glUseProgram(program42);
        model_view_42 = glGetUniformLocation(program42, "model_view");
        projection_42 = glGetUniformLocation(program42, "projection");
        glUniformMatrix4fv(projection_42, 1, GL_TRUE, p);
        glUniformMatrix4fv(model_view_42, 1, GL_TRUE, mv);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawObj42(floor_buffer, floor_NumVertices, 1);

    }
    else {
        glUseProgram(program53);
        ModelView = glGetUniformLocation(program53, "ModelView");
        Projection = glGetUniformLocation(program53, "Projection");
        glUniformMatrix4fv(Projection, 1, GL_TRUE, p);
        if (pointSource)
        {
            point_light(mv);
        }
        else
        {
            spot_light(mv);
        }
        floor_material();
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv);
        glUniformMatrix3fv(glGetUniformLocation(program53, "Normal_Matrix"),
            1, GL_TRUE, NormalMatrix(mv, 1));
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawObj53(floor_light_buffer, 6, 1);
    }
    glDepthMask(GL_TRUE);


    glUseProgram(program42);
    model_view_42 = glGetUniformLocation(program42, "model_view");
    projection_42 = glGetUniformLocation(program42, "projection");
    glUniformMatrix4fv(model_view_42, 1, GL_TRUE, mv);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawObj42(line_buffer, 9);

    glUseProgram(program42);


    acc_rotation = Rotate(delta, rotation_axis[current_index].x, rotation_axis[current_index].y, rotation_axis[current_index].z) * acc_rotation;
    mat4  mv_sphere = LookAt(eye, at, up) * Translate(current_position.x, current_position.y, current_position.z) * acc_rotation;
    if (eye.y < 0) {
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    }
    if (shadow == 1)
    {
        mv = LookAt(eye, at, up) * shadow_projection * Translate(current_position.x, current_position.y, current_position.z) * acc_rotation;
        glUniformMatrix4fv(model_view_42, 1, GL_TRUE, mv);
        if (solidFlag == 0)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        glEnable(GL_CULL_FACE);
        drawObj42(sphere_shadow_buffer, vertice_cnt, 3);
        glDisable(GL_CULL_FACE);
    }

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    if (flatShadow == 1) {
        glUseProgram(program53);
        ModelView = glGetUniformLocation(program53, "ModelView");
        Projection = glGetUniformLocation(program53, "Projection");
        glUniformMatrix4fv(Projection, 1, GL_TRUE, p);
        if (pointSource)
        {
            point_light(mv);
        }
        else
        {
            spot_light(mv);
        }
        sphere_material();
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv_sphere);
        glUniformMatrix3fv(glGetUniformLocation(program53, "Normal_Matrix"),
            1, GL_TRUE, NormalMatrix(mv_sphere, 1));
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawObj53(flatsphere_buffer, vertice_cnt, 2);

    }
    else if (smoothShadow == 1) {
        glUseProgram(program53);
        ModelView = glGetUniformLocation(program53, "ModelView");
        Projection = glGetUniformLocation(program53, "Projection");
        glUniformMatrix4fv(Projection, 1, GL_TRUE, p);
        if (pointSource)
        {
            point_light(mv);
        }
        else
        {
            spot_light(mv);
        }
        sphere_material();
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv_sphere);
        glUniformMatrix3fv(glGetUniformLocation(program53, "Normal_Matrix"),
            1, GL_TRUE, NormalMatrix(mv_sphere, 1));
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawObj53(smoothsphere_buffer, vertice_cnt, 2);
    }
    else {
        glUseProgram(program42);
        model_view_42 = glGetUniformLocation(program42, "model_view");
        projection_42 = glGetUniformLocation(program42, "projection");
        glUniformMatrix4fv(projection_42, 1, GL_TRUE, p);
        glUniformMatrix4fv(model_view_42, 1, GL_TRUE, mv_sphere);
        if (solidFlag == 0)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        drawObj42(sphere_buffer, vertice_cnt, 2);

    }
    if (firework_flag == 1) {
        t = glutGet(GLUT_ELAPSED_TIME);
        glUseProgram(programF);

        t = fmod(t - tsub, tmax);
        glUniform1f(glGetUniformLocation(programF, "t"), t);
        glUniformMatrix4fv(glGetUniformLocation(programF, "MV"), 1, GL_TRUE, mv);
        glUniformMatrix4fv(glGetUniformLocation(programF, "Projection"), 1, GL_TRUE, p);
        drawF(firework_buffer, N);
    }
    
    glutSwapBuffers();
}
//---------------------------------------------------------------------------
float Distance(point3 start, point3 dest) {
    float dx = start.x - dest.x;
    float dy = start.y - dest.y;
    float dz = start.z - dest.z;

    return sqrt(dx * dx + dy * dy + dz * dz);


}

void idle(void)
{
    theta += delta;
    theta = fmod(theta, GLfloat(360.0));

    //offset from rotation to transilation
    float offset = (radius * delta * 3.1415926) / 180;
    current_position.x += vectors[current_index].x * offset;
    current_position.y += vectors[current_index].y * offset;
    current_position.z += vectors[current_index].z * offset;

    int next = (current_index + 1 == total) ? 0 : current_index + 1;
    point3 start = rotation_routine[current_index];
    point3 dest = rotation_routine[next];

    if (Distance(start, current_position) > Distance(start, dest)) {
        current_index = (current_index + 1 == total) ? 0 : current_index + 1;
        current_position = rotation_routine[current_index];
    }

    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 033: // Escape Key
    case 'q': case 'Q':

        //delocate the memory
        delete[]rotation_axis;
        delete[]vectors;
        delete[]sphere_vertice;
        delete[]sphere_color;
        delete[]sphere_shadow;
        delete[]sphere_shadow_color;
        delete[]sphere_shader_data;
        delete[]sphere_flat_shader;
        delete[]sphere_smooth_shader;


        exit(EXIT_SUCCESS);
        break;
    case 'b': case 'B':
        if (!mouse_in) {  // If 'B' is pressed for the first time

            glutIdleFunc(idle); // Start the animation
            mouse_in = true;    // Enable mouse control

        }


        break;
    case 'X': eye[0] += 1.0; break;
    case 'x': eye[0] -= 1.0; break;
    case 'Y': eye[1] += 1.0; break;
    case 'y': eye[1] -= 1.0; break;
    case 'Z': eye[2] += 1.0; break;
    case 'z': eye[2] -= 1.0; break;


    case 'a': case 'A': // Toggle between animation and non-animation
        if (mouse_in) {

            animationFlag = 1 - animationFlag;
            if (animationFlag == 1) glutIdleFunc(idle);
            else                    glutIdleFunc(NULL);

        }
        break;

        /*case 'c': case 'C': // Toggle between filled and wireframe cube
            cubeFlag = 1 -  cubeFlag;
                break;*/

    case 'f': case 'F': // Toggle between filled and wireframe floor
        floorFlag = 1 - floorFlag;
        break;

    case 'd': case 'D':  // reset to initial viewer/eye position
        eye = init_eye;
        break;
    case 'v': case 'V':
        sphere_texture_mode = 1;
        break;
    case 's': case 'S':
        sphere_texture_mode = 2;
        break;
    case 'o': case 'O':
        sphere_texture_space = 1;
        break;
    case 'e': case 'E':
        sphere_texture_space = 2;
        break;
    case 'u': case 'U':
        latticeMode = latticeModeTemp = 1;
        break;
    case 't': case 'T':
        latticeMode = latticeModeTemp = 2;
        break;
    case 'l': case 'L':
        if (latticeMode > 0)latticeMode = 0;
        else latticeMode = latticeModeTemp;
        break;
    }
    glutPostRedisplay();
}
void mouse(int button, int state, int x, int y) {
    if (mouse_in) {

        // Toggle rolling with right mouse button
        if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
            if (isRolling) {
                glutIdleFunc(NULL); // Stop rolling
                isRolling = false;

            }
            else {
                glutIdleFunc(idle); //resume rolling
                isRolling = true;

            }
        }
    }

}
enum MenuEntries {
    DEFAULT_VIEW = 0,
    QUIT = 1,
    TOGGLE_SPHERE_MODE = 2,
    SHADOW_OFF = 3,
    SHADOW_ON = 4,
    FLAT_SHADING = 5,
    SMOOTH_SHADING = 6,
    LIGHTING_OFF = 7,
    LIGHTING_ON = 8,
    SPOT_LIGHT = 9,
    POINT_LIGHT = 10,
    ENABLE_LIGHTING_NO = 11,
    ENABLE_LIGHTING_YES = 12
};
void menu(int num) {
    switch (num) {
    case DEFAULT_VIEW:
        eye = init_eye; // Reset to initial viewer/eye position
        break;
    case QUIT:

        //delocate the memory
        delete[]rotation_axis;
        delete[]vectors;
        delete[]sphere_vertice;
        delete[]sphere_color;
        delete[]sphere_shadow;
        delete[]sphere_shadow_color;
        delete[]sphere_shader_data;
        delete[]sphere_flat_shader;
        delete[]sphere_smooth_shader;

        exit(EXIT_SUCCESS); // Exit the program
        break;
    case TOGGLE_SPHERE_MODE:
        solidFlag = !solidFlag;
        smoothShadow = false;
        flatShadow = false;
        break;
    }
    glutPostRedisplay();
}
void shadow_menu(int index) {
    shadow = (index == SHADOW_ON);
    glutPostRedisplay();
}
void shader_menu(int index) {
    if (!lighting) {
        smoothShadow = false;
        flatShadow = false;
    }
    else {
        flatShadow = (index == FLAT_SHADING);
        smoothShadow = (index == SMOOTH_SHADING);
    }
    solidFlag = true;
    glutPostRedisplay();
}

void lighting_menu(int index) {
    lighting = (index == ENABLE_LIGHTING_YES);
    if (!lighting) {
        smoothShadow = false;
        flatShadow = false;
    }
    glutPostRedisplay();
}

void source_menu(int index) {

    spotlight = (index == SPOT_LIGHT);
    pointSource = (index == POINT_LIGHT);

    glutPostRedisplay();
}
void fog_menu(int index) {
    fog_fl = index;
    glutPostRedisplay();
}
void blend_menu(int index) {
    // blendShadow=index;
    if (index == 0)
    {
        glDisable(GL_BLEND);
    }
    else if (index == 1)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    glutPostRedisplay();
}
void floor_menu(int index) {
    floor_textured = index;
    glutPostRedisplay();
}
void sphere_menu(int index) {
    sphere_use_texture = index;
    glutPostRedisplay();
}
void firework_menu(int index) {
    firework_flag = index;
    glutPostRedisplay();
}
void createMenu() {
    int lightingMenu = glutCreateMenu(lighting_menu);
    glutAddMenuEntry("No", ENABLE_LIGHTING_NO);
    glutAddMenuEntry("Yes", ENABLE_LIGHTING_YES);
    glutSetMenuFont(lightingMenu, (void*)GLUT_BITMAP_HELVETICA_18);

    int shadowMenu = glutCreateMenu(shadow_menu);
    glutAddMenuEntry("No", SHADOW_OFF);
    glutAddMenuEntry("Yes", SHADOW_ON);
    glutSetMenuFont(shadowMenu, (void*)GLUT_BITMAP_HELVETICA_18);


    int shaderMenu = glutCreateMenu(shader_menu);
    glutAddMenuEntry("Flat Shading", FLAT_SHADING);
    glutAddMenuEntry("Smooth Shading", SMOOTH_SHADING);
    glutSetMenuFont(shaderMenu, (void*)GLUT_BITMAP_HELVETICA_18);

    int lightsourceMenu = glutCreateMenu(source_menu);
    glutAddMenuEntry("Spot Light", SPOT_LIGHT);
    glutAddMenuEntry("Point Light", POINT_LIGHT);
    glutSetMenuFont(lightsourceMenu, (void*)GLUT_BITMAP_HELVETICA_18);

    int fogMenu = glutCreateMenu(fog_menu);
    glutAddMenuEntry("No Fog", 0);
    glutAddMenuEntry("Linear", 1);
    glutAddMenuEntry("Exponential", 2);
    glutAddMenuEntry("Exponential Square", 3);
    glutSetMenuFont(fogMenu, (void*)GLUT_BITMAP_HELVETICA_18);

    int blendMenu = glutCreateMenu(blend_menu);
    glutAddMenuEntry("No", 0);
    glutAddMenuEntry("Yes", 1);
    glutSetMenuFont(blendMenu, (void*)GLUT_BITMAP_HELVETICA_18);

    int floorMenu = glutCreateMenu(floor_menu);
    glutAddMenuEntry("No", 0);
    glutAddMenuEntry("Yes", 1);
    glutSetMenuFont(floorMenu, (void*)GLUT_BITMAP_HELVETICA_18);

    int fireworkMenu = glutCreateMenu(firework_menu);
    glutAddMenuEntry("No", 0);
    glutAddMenuEntry("Yes", 1);
    glutSetMenuFont(fireworkMenu, (void*)GLUT_BITMAP_HELVETICA_18);

    int sphereMenu = glutCreateMenu(sphere_menu);
    glutAddMenuEntry("No", 0);
    glutAddMenuEntry("Yes Contour Lines", 1);
    glutAddMenuEntry("Yes Checkerboard", 2);
    glutSetMenuFont(sphereMenu, (void*)GLUT_BITMAP_HELVETICA_18);

    int menuID = glutCreateMenu(menu);
    glutAddMenuEntry("Default View Point", DEFAULT_VIEW);
    glutAddMenuEntry("Quit", QUIT);
    glutAddMenuEntry("Toggle Sphere Mode", TOGGLE_SPHERE_MODE);
    glutAddSubMenu("Enable Lighting", lightingMenu);
    glutAddSubMenu("Shadow", shadowMenu);
    glutAddSubMenu("Shading", shaderMenu);
    glutAddSubMenu("Light", lightsourceMenu);
    glutAddSubMenu("Fog Options", fogMenu);
    glutAddSubMenu("Blending Shadow", blendMenu);
    glutAddSubMenu("Textured Mapped Ground", floorMenu);
    glutAddSubMenu("Textured Mapped Sphere", sphereMenu);
    glutAddSubMenu("FireWork", fireworkMenu);

    // Set the menu font as desired
    glutSetMenuFont(menuID, (void*)GLUT_BITMAP_HELVETICA_18);


    glutAttachMenu(GLUT_LEFT_BUTTON);
}



//----------------------------------------------------------------------------
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = (GLfloat)width / (GLfloat)height;
    glutPostRedisplay();
}


//----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    FilesIn();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    glutCreateWindow("Sphere");


    int err = glewInit();
    if (GLEW_OK != err)
    {
        printf("Error: glewInit failed: %s\n", (char*)glewGetErrorString(err));
        exit(1);
    }


    // Get info of GPU and supported OpenGL version
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL version supported %s\n", glGetString(GL_VERSION));

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glCullFace(GL_BACK);

    init();
    createMenu();
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);



    glutMainLoop();


    delete[]rotation_axis;
    delete[]vectors;
    delete[]sphere_vertice;
    delete[]sphere_color;
    delete[]sphere_shadow;
    delete[]sphere_shadow_color;
    delete[]sphere_shader_data;
    delete[]sphere_flat_shader;
    delete[]sphere_smooth_shader;


    return 0;
}
