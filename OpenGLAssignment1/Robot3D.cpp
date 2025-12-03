/*******************************************************************
           Hierarchical Multi-Part Model Example
********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gl/glut.h>
#include <utility>
#include <vector>
#include "VECTOR3D.h"
#include "QuadMesh.h"
#define M_PI 3.14159265358979323846

const int vWidth = 650;    // Viewport width in pixels
const int vHeight = 500;    // Viewport height in pixels

// Note how everything depends on blimp body dimensions so that can scale entire blimp proportionately
// just by changing blimp body scale
float blimpBodyWidth = 6.0;
float blimpBodyLength = 6.0;
float blimpBodyDepth = 18.0;
float cabinLength = 0.4 * blimpBodyWidth;

// Control blimp movement
float blimpAngle = 90.0;
float blimpPosX = 0.0;
float blimpPosZ = 0.0;
float blimpHeight = 0.0;
float propellerRotationAngle = 0.0;

//control propeller up/down
float propellerUpDown = 0.0;

// Lighting/shading and material properties for blimp - upcoming lecture - just copy for now
// Blimp RGBA material properties (NOTE: we will learn about this later in the semester)
// Blimp body material properties
GLfloat blimpBody_mat_ambient[] = { 0.5f, 0.7f, 1.0f, 1.0f };
GLfloat blimpBody_mat_specular[] = { 0.5f, 0.7f, 1.0f, 1.0f };
GLfloat blimpBody_mat_diffuse[] = { 0.5f, 0.7f, 1.0f, 1.0f };
GLfloat blimpBody_mat_shininess[] = { 64.0F };

// Propeller material properties
GLfloat propeller_mat_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // You can use a neutral color for the propellers
GLfloat propeller_mat_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat propeller_mat_diffuse[] = { 0.1f, 0.1f, 0.1f, 1.0f };
GLfloat propeller_mat_shininess[] = { 32.0F };

// Light properties
GLfloat light_position0[] = { -4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_position1[] = { 4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

// Mouse button
int currentButton;

// A flat open mesh
QuadMesh* groundMesh = NULL;

// Structure defining a bounding box, currently unused
typedef struct BoundingBox {
    VECTOR3D min;
    VECTOR3D max;
} BBox;

// Default Mesh Size
int meshSize = 16;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void drawBlimp();
void drawBody();
void drawCabin();
void drawPropellers();

int main(int argc, char** argv)
{
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(vWidth, vHeight);
    glutInitWindowPosition(200, 30);
    glutCreateWindow("3D Hierarchical Example");

    // Initialize GL
    initOpenGL(vWidth, vHeight);

    // Register callback functions
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotionHandler);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(functionKeys);

    // Start event loop, never returns
    glutMainLoop();

    return 0;
}

// Set up OpenGL. For viewport and projection setup see reshape(). 
void initOpenGL(int w, int h)
{
    // Set up and enable lighting
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);   // This second light is currently off

    // Other OpenGL setup
    glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
    glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
    glClearColor(0.4F, 0.4F, 0.4F, 0.0F);  // Color and depth for glClear
    glClearDepth(1.0f);
    glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Other initializatuion
    // Set up ground quad mesh
    VECTOR3D origin = VECTOR3D(-16.0f, 0.0f, 16.0f);
    VECTOR3D dir1v = VECTOR3D(1.0f, 0.0f, 0.0f);
    VECTOR3D dir2v = VECTOR3D(0.0f, 0.0f, -1.0f);
    groundMesh = new QuadMesh(meshSize, 32.0);
    groundMesh->InitMesh(meshSize, origin, 32.0, 32.0, dir1v, dir2v);

    VECTOR3D ambient = VECTOR3D(0.0f, 0.05f, 0.0f);
    VECTOR3D diffuse = VECTOR3D(0.4f, 0.8f, 0.4f);
    VECTOR3D specular = VECTOR3D(0.04f, 0.04f, 0.04f);
    float shininess = 0.2;
    groundMesh->SetMaterial(ambient, diffuse, specular, shininess);
}

// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    // Create Viewing Matrix V
    // Set up the camera at position (0, 6, 22) looking at the origin, up along the positive y axis
    gluLookAt(0.0, 6.0, 40.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    // Draw Blimp

    // Apply modeling transformations M to move blimp
    // Current transformation matrix is set to IV, where I is the identity matrix
    // CTM = IV
    drawBlimp();

    // Draw ground
    glPushMatrix();
    glTranslatef(0.0, -20.0, 0.0);
    groundMesh->DrawMesh(meshSize);
    glPopMatrix();

    glutSwapBuffers();   // Double buffering, swap buffers
}

void drawBlimp()
{
    glPushMatrix();
    // spin blimp on base. 
    glTranslatef(blimpPosX, blimpHeight, blimpPosZ);
    glRotatef(blimpAngle, 0.0, -1.0, 0.0);

    drawBody();
    drawCabin();
    drawPropellers();
    glPopMatrix();

    glPopMatrix();
}

void drawBody()
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, blimpBody_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, blimpBody_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, blimpBody_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, blimpBody_mat_shininess);

    glPushMatrix();  //Center piece
    glScalef(blimpBodyWidth, blimpBodyLength, blimpBodyDepth);
    gluCylinder(gluNewQuadric(), 1, 1, 1, 20, 10);
    glPopMatrix();

    glPushMatrix();  //Balloon End
    glTranslatef(0, 0, blimpBodyLength * 3);
    glScalef(blimpBodyWidth, blimpBodyLength, blimpBodyDepth * 0.6);
    glutSolidSphere(1, 20, 20);
    glPopMatrix();

    glPushMatrix();  //Balloon End 2
    glScalef(blimpBodyWidth, blimpBodyLength, blimpBodyDepth * 0.6);
    glutSolidSphere(1, 20, 20);
    glPopMatrix();

    glPushMatrix();  //Fins (Horizontal)
    glTranslatef(0, -2, -4);
    glScalef(16, 0.3, 6);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix(); //Fins (Vertical)
    glRotatef(15, 1, 0, 0);
    glTranslatef(0, 0.5, -5);
    glScalef(0.3, 10, 8);
    glutSolidCube(1.0);
    glPopMatrix();
}

void drawCabin()
{
    // Set blimp material properties per body part. Can have separate material properties for each part
    glMaterialfv(GL_FRONT, GL_AMBIENT, blimpBody_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, blimpBody_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, blimpBody_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, blimpBody_mat_shininess);

    glPushMatrix();
    // Position cabin with respect to parent (body)
    glTranslatef(0, 0.5 * blimpBodyLength + 0.5 * cabinLength, 0); // this will be done last

    // Build Cabin
    glTranslatef(0, -10, 5);
    glScalef(blimpBodyWidth, 0.6 * blimpBodyWidth, 1.2 * blimpBodyWidth);
    glutSolidCube(1.0);

    glPopMatrix();
}

void drawPropellers() {
    glMaterialfv(GL_FRONT, GL_AMBIENT, propeller_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, propeller_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, propeller_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, propeller_mat_shininess);

    glPushMatrix();
    glTranslatef(0, 0, 12);
    glScalef(20, 0.6, 3.5);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glScalef(0.8, 0.8, 2.5);

    // First Propeller
    glPushMatrix();
    glTranslatef(12, 0, 4.3);
    glutSolidSphere(2, 20, 20);

    glPushMatrix();
    glRotatef(propellerRotationAngle * 12, 0, 0, 1);
    glPushMatrix();
    glTranslatef(0, 0, 1.8);
    glScalef(8, 1, 0.1);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glRotatef(90, 0, 0, 1);
    glTranslatef(0, 0, 1.8);
    glScalef(8, 1, 0.1);
    glutSolidCube(1.0);
    glPopMatrix();

    glPopMatrix();
    glPopMatrix();

    // Second Propeller
    glPushMatrix();
    glTranslatef(-12, 0, 4.3);
    glutSolidSphere(2, 20, 20);

    glPushMatrix();
    glRotatef(propellerRotationAngle * 12, 0, 0, -1);
    glPushMatrix();
    glTranslatef(0, 0, 1.8);
    glScalef(8, 1, 0.1);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glRotatef(90, 0, 0, 1);
    glTranslatef(0, 0, 1.8);
    glScalef(8, 1, 0.1);
    glutSolidCube(1.0);
    glPopMatrix();

    glPopMatrix();
    glPopMatrix();

    glPopMatrix();
}

// Callback, called at initialization and whenever the user resizes the window.
void reshape(int w, int h)
{
    // Set up viewport, projection, then change to modelview matrix mode - 
    // the display function will then set up the camera and do modeling transforms.
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLdouble)w / h, 0.2, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Set up the camera at position (0, 6, 22) looking at the origin, up along the positive y-axis
    gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

bool stop = false;

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
        blimpPosX += -sin(blimpAngle * (M_PI / 180));
        blimpPosZ += cos(blimpAngle * (M_PI / 180));
        propellerRotationAngle += 2;
        break;
    }

    glutPostRedisplay();   // Trigger a window redisplay
}

// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
    // Help key
    switch (key)
    {
    case GLUT_KEY_LEFT:
        blimpAngle -= 2;
        propellerRotationAngle += 2;
        break;
    case GLUT_KEY_RIGHT:
        blimpAngle += 2;
        propellerRotationAngle += 2;
        break;
    case GLUT_KEY_UP:
        blimpHeight += 1;
        propellerRotationAngle += 2;
        break;
    case GLUT_KEY_DOWN:
        blimpHeight -= 1;
        propellerRotationAngle += 2;
        break;
    }
    // Do transformations with arrow keys
    //else if (...)   // GLUT_KEY_DOWN, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_LEFT
    //{
    //}

    glutPostRedisplay();   // Trigger a window redisplay
}

// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
    // Write some text to the screen to indicate mouse click
    currentButton = button;
}

// Mouse motion callback
void mouseMotionHandler(int xMouse, int yMouse)
{
    // Respond to mouse movement while buttons are pressed

    glutPostRedisplay();   // Trigger a window redisplay
}

GLuint LoadTexture(const std::string& file)
{
    GLuint textureID = SOIL_load_OGL_texture(file.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}