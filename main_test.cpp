#include <stdio.h>
#include <math.h>
#include <vector>

using namespace std;

struct Circle {
    int x, y, r;
};

vector<Circle> input_circles;

#ifdef __APPLE__  // include Mac OS X verions of headers
#include <GLUT/glut.h>
#else // non-Mac OS X operating systems
#include <GL/glut.h>
#endif

#define XOFF          50
#define YOFF          50
#define WINDOW_WIDTH  600
#define WINDOW_HEIGHT 600

int x, y, r;
int choice;
float maxX, maxY, scaleX, scaleY;
int currentFrame = 0; 
int totalFrames = 60;
bool animationInit = false;

void display(void);
void myinit(void);
void draw_circle(int, int, int);
void executePartC(void);
void executePartD(void); 
void executePartE(void); 
void transformCircle(void);
void updateAnimation(int);



/* Function to handle file input; modification may be needed */
void file_in(void);

/*-----------------
The main function
------------------*/
int main(int argc, char** argv)
{
    

    printf("Choose an option:\n");
    printf("1. Draw Circle (Part C)\n");
    printf("2. Draw Circles from input file D\n");
    printf("3. Animation of drawing circles E\n");
    printf("Enter your choice: ");
    scanf_s("%d", &choice);

    switch (choice) {
        case 1:
            executePartC();
            break;
        case 2:
            //handle by display
            break;
        case 3:
            //handle by display
            
            break;
        default:
            printf("Invalid choice. Exiting program.\n");
            return 0;
    }

    glutInit(&argc, argv);

    /* Use both double buffering and Z buffer */
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition(XOFF, YOFF);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Assignment 1 part_c");
    glutDisplayFunc(display);
    

    /* Function call to handle file input here */
    file_in();

    myinit();
    glutMainLoop();

    free(&input_circles); 
    return 0;
}

/*----------
file_in(): file input function. Modify here.
------------*/
void file_in(void)
{
    FILE* file;
    errno_t err;

    if ((err = fopen_s(&file, "input_circles.txt", "r")) != 0) {
        printf("Fail to open the file\n");
        return;
    }

    int circle_cnt;
    fscanf_s(file, "%d", &circle_cnt);
    input_circles.resize(circle_cnt);

    for (int i = 0; i < circle_cnt; i++) {
        fscanf_s(file, "%d %d %d", &input_circles[i].x, &input_circles[i].y, &input_circles[i].r);
        
    }
    fclose(file);

    transformCircle();
}


/*---------------------------------------------------------------------
display(): This function is called once for _every_ frame.
---------------------------------------------------------------------*/
void display(void)
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(1.0f, 0.84f, 0.0f);         /* draw in golden yellow */
    glPointSize(1.0);                     /* size of each point */

    glBegin(GL_POINTS);
    switch (choice) {
        case 1:
            draw_circle(x, y, r);
            break;
        case 2:
            executePartD();
            break;
        case 3:
            if (!animationInit) {
                executePartE();
            }
            for (const auto& circle : input_circles) {
                int scaled_x = (circle.x * scaleX) + WINDOW_WIDTH / 2;
                int scaled_y = (circle.y * scaleY) + WINDOW_HEIGHT / 2;
                int scaled_radius = (circle.r * scaleX * currentFrame) /totalFrames;
                draw_circle(scaled_x, scaled_y, scaled_radius);
                
            }
            
            break;
        default:
            printf("Invalid choice. Exiting program.\n");
            
    }
    
    
    glEnd();

    glFlush();                            /* render graphics */

    glutSwapBuffers();                    /* swap buffers */
}

/*---------------------------------------------------------------------
myinit(): Set up attributes and viewing
---------------------------------------------------------------------*/
void myinit()
{
    glClearColor(0.0f, 0.0f, 0.92f, 0.0f);    /* blue background*/

    /* set up viewing */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
}

/*---------------------------------------------------------------------
function set: display function for each parts
---------------------------------------------------------------------*/

void executePartC() {
    printf("Enter the Xcenter of the circle : ");
    scanf_s("%d", &x);
    printf("Enter the Ycenter of the circle : ");
    scanf_s("%d", &y);
    printf("Enter the radius of the circle: ");
    scanf_s("%d", &r);
    
}

void draw_circle(int xCenter, int yCenter, int radius) {
    int x = 0;
    int y = radius;
    int d = 5 - 4 * radius; 
    
    auto circlePoint = [&](int xCenter, int yCenter, int x, int y) {
        glVertex2i(xCenter + x, yCenter + y);
        glVertex2i(xCenter - x, yCenter + y);
        glVertex2i(xCenter + x, yCenter - y);
        glVertex2i(xCenter - x, yCenter - y);
        glVertex2i(xCenter + y, yCenter + x);
        glVertex2i(xCenter - y, yCenter + x);
        glVertex2i(xCenter + y, yCenter - x);
        glVertex2i(xCenter - y, yCenter - x);
    };


    circlePoint(xCenter, yCenter, x, y);

    while (y >= x) {
        x++;
        if (d < 0) {
            d += 8 * x + 12;
        }
        else {
            y--;
            d += 8 * x - 8 * y + 20;
        }
        circlePoint(xCenter, yCenter, x, y);
    }
}

void transformCircle() {

    if (!input_circles.empty()) {

        maxX = abs(input_circles[0].x) + input_circles[0].r;
        maxY = abs(input_circles[0].y) + input_circles[0].r;
    }

    for (const auto& circle : input_circles) {

        maxX = max(maxX, float(abs(circle.x) + circle.r));
        maxY = max(maxY, float(abs(circle.y) + circle.r));
    }

    float widthScale = (WINDOW_WIDTH) / (2 * maxX);
    float heightScale = (WINDOW_HEIGHT) / (2 * maxY);

    float scaleFactor = min(widthScale, heightScale);
    scaleX = scaleY = scaleFactor;
}

void executePartD() {
    for (const auto& circle : input_circles) {
        int scaled_x = (circle.x * scaleX) + WINDOW_WIDTH / 2;
        int scaled_y = (circle.y * scaleY) + WINDOW_HEIGHT / 2;
        int scaled_radius = circle.r * scaleX;
        draw_circle(scaled_x, scaled_y, scaled_radius); 
    }
}

void updateAnimation(int value) {
    currentFrame = (currentFrame + 1) % totalFrames;
    glutPostRedisplay();
    glutTimerFunc(50, updateAnimation, 0);
}
void executePartE(void) {
    if (!animationInit) {
        currentFrame = 0;
        updateAnimation(0);
        animationInit = true;
    }
}