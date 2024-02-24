#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <windows.h>
#include <GL/glut.h>
#endif
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <utility>
#include <vector>
#include "VECTOR3D.h"
#include "QuadMesh.h"

//------------------------------------------------------------------------------------------------------

const int vWidth  = 650;    // Viewport width in pixels
const int vHeight = 500;    // Viewport height in pixels

float robotBodyWidth = 8.0;
float robotBodyLength = 9.0;
float robotBodyDepth = 6.0;
float upperArmLength = robotBodyLength;
float upperArmWidth = 0.125*robotBodyWidth;
float gunLength = upperArmLength / 4.0;
float gunWidth = upperArmWidth;
float gunDepth = upperArmWidth;

// Control Robot body rotation on base
float robotAngle = 30.0;
//float robotAngle = -90.0;
//float robotAngle = 0.0;


// Control arm rotation
float shoulderAngle = -40.0;
float cannonRotation = 0.0;
float hipJointAngle = 0.0;
float kneeJointAngle = -40.0;
float bodyJointAngle = 0.0;

bool kneeSelect = false;
bool hipSelect = false;
bool bodySelect = false;

//------------------------------------------------------------------------------------------------------

// Robot RGBA material properties referenced colours from:
// http://www.it.hiof.no/~borres/j3d/explain/light/p-materials.html
// Cyan rubber
GLfloat robotBody_mat_ambient[] = { 0.0f,0.05f,0.05f,1.0f };
GLfloat robotBody_mat_specular[] = { 0.4f,0.5f,0.5f,1.0f };
GLfloat robotBody_mat_diffuse[] = { 0.04f,0.7f,0.7f,1.0f };
GLfloat robotBody_mat_shininess[] = { 10.0f };

// Black rubber
GLfloat robotArm_mat_ambient[] = { 0.02f, 0.02f, 0.02f, 1.0f };
GLfloat robotArm_mat_diffuse[] = { 0.01f, 0.01f, 0.01f, 1.0f };
GLfloat robotArm_mat_specular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
GLfloat robotArm_mat_shininess[] = { 10.0f };

// Chrome
GLfloat gun_mat_ambient[] = { 0.25f, 0.25f, 0.25f, 1.0f };
GLfloat gun_mat_diffuse[] = { 0.4f, 0.4f, 0.4f, 1.0f };
GLfloat gun_mat_specular[] = { 0.774597f, 0.774597f, 0.774597f, 1.0f };
GLfloat gun_mat_shininess[] = { 76.8f };

GLfloat robotLowerBody_mat_ambient[] = { 0.25f, 0.25f, 0.25f, 1.0f };
GLfloat robotLowerBody_mat_diffuse[] = { 0.4f, 0.4f, 0.4f, 1.0f };
GLfloat robotLowerBody_mat_specular[] = { 0.774597f, 0.774597f, 0.774597f, 1.0f };
GLfloat robotLowerBody_mat_shininess[] = { 76.8F };


// Light properties
GLfloat light_position0[] = { -4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_position1[] = { 4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

//------------------------------------------------------------------------------------------------------

// Mouse button
int currentButton;

// A flat open mesh
QuadMesh *groundMesh = NULL;

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
void animationHandler(int param);
void walkAnimation(int param);
void undoWalk(int param);
void cannonAnimation(int param);
void drawRobot();
void drawBody();
void drawLowerBody();
void drawLeftArm();
void drawUpperRightLeg();
void drawlowerRightLeg();
void drawUpperLeftLeg();
void drawlowerLeftLeg();
void drawRightCannon();
void drawLeftCannon();

//------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(200, 30);
	glutCreateWindow("3D Bot");

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
	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 6.0, 26.0, 0.0, -3.0, 0.0, 0.0, 1.0, 0.0);

	// Draw Robot

	// Apply modelling transformations M to move robot
	// Current transformation matrix is set to IV, where I is identity matrix
	// CTM = IV
	drawRobot();

	// Draw ground
	glPushMatrix();
	glTranslatef(0.0, -20.0, 0.0);
	groundMesh->DrawMesh(meshSize);
	glPopMatrix();

	glutSwapBuffers();   // Double buffering, swap buffers
}

void drawRobot()
{
	glPushMatrix();
    // spin robot on base.
    glRotatef(robotAngle, 0.0, 1.0, 0.0);
     
    //  Rotate torso and cannons at hip
     glPushMatrix();
     glRotatef(bodyJointAngle, 1.0, 0.0, 0.0);
	 drawBody();
     drawLeftCannon();
     drawRightCannon();
     glPopMatrix();

//    Rotate hip at body
    glPushMatrix();
    glTranslatef(-(-0.5*robotBodyWidth - 0.5*upperArmWidth), -0.3*robotBodyLength, -0.7*robotBodyDepth);
    glRotatef(hipJointAngle, 1.0, 0.0, 0.0);
    glTranslatef(-0.5*robotBodyWidth - 0.5*upperArmWidth, 0.3*robotBodyLength, 0.7*robotBodyDepth);
     drawUpperLeftLeg();
     drawlowerLeftLeg();
    glPopMatrix();
	 drawUpperRightLeg();
     drawlowerRightLeg();


	glPopMatrix();
}

void drawBody()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotBody_mat_shininess);
    
	glPushMatrix();
	glScalef(robotBodyWidth, robotBodyLength, 2*robotBodyDepth);
	glutSolidCube(1.0);
	glPopMatrix();
}

//LEFT LEG
void drawUpperLeftLeg()
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, gun_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, gun_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, gun_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, gun_mat_shininess);

    glPushMatrix();  // 1
    // --------------------------

    // hip joint --------------------------
    // Position hip joint with respect to parent body
    glTranslatef(-0.5*robotBodyWidth - 0.5*upperArmWidth, -0.3*robotBodyLength, -0.7*robotBodyDepth); // this will be done last
    
    // build hip joint
    glPushMatrix();
    glScalef(1.0, 2.0, 2.0);
    glutSolidCube(1.0);
    glPopMatrix();
    
    // upper leg --------------------------
    
    glPushMatrix();

    // Rotate leg at hip
    glRotatef(-30.0, 0.0, 0.0, 1.0);
    // Position whole leg with respect to parent hip joint
    glTranslatef(0.0, -1.5, 0.0);

    // build upper leg
    glScalef(1.0, 2.0, 1.0);
    glutSolidCube(2.0);

    glPopMatrix();
    
    // --------------------------
//    drawlowerLeftLeg();
    glPopMatrix();  // 1
}


void drawlowerLeftLeg()
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);

    glPushMatrix();
    // Rotate leg at knee
    glTranslatef(-7.0, -5, -7);
    glRotatef(kneeJointAngle, 1.0, 0.0, 0.0);
    glTranslatef(7.0, 5, 7);

    // Position leg with respect to parent upper leg
    glTranslatef(-7.0, -11, -5);

    // build leg
    glPushMatrix();
    glScalef(1.0, 4.0, 1.0);
    glutSolidCube(3.0);
    glPopMatrix();

    //  foot-----------------------------------------------------
    glMaterialfv(GL_FRONT, GL_AMBIENT, gun_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, gun_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, gun_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, gun_mat_shininess);

    glPushMatrix();
    // Position foot with respect to parent leg
    glTranslatef(0, -(0.5*upperArmLength + 0.5*gunLength), 0.0);

    // build foot
    glScalef(4.0, -2.0, -2.0);
    glutSolidCube(1.0);
    glPopMatrix();
    //  foot-----------------------------------------------------

    glPopMatrix();
}

//RIGHT LEG
void drawUpperRightLeg()
{
    // upper leg ------------------------------------------------------------
    glMaterialfv(GL_FRONT, GL_AMBIENT, gun_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, gun_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, gun_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, gun_mat_shininess);

    glPushMatrix();  // 1
    // Position hip joint with respect to parent body
    glTranslatef(0.5*robotBodyWidth + 0.5*upperArmWidth, -0.3*robotBodyLength, -0.7*robotBodyDepth); // this will be done last

    // build hip joint
    glPushMatrix();
    glScalef(1.0, 2.0, 2.0);
    glutSolidCube(1.0);
    glPopMatrix();

    // Rotate leg at hip
//    glRotatef(30.0, 1.0, 0.0, 0.0);
    glRotatef(30.0, 0.0, 0.0, 1.0);

    // Position whole leg with respect to parent hip joint
    glTranslatef(0.0, -1.5, 0.0);

    // build upper leg
    glPushMatrix();
    glScalef(1.0, 2.0, 1.0);
    glutSolidCube(2.0);
    glPopMatrix();

    glPopMatrix();  // 1
}


void drawlowerRightLeg()
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);

    glPushMatrix();
    // Rotate leg at knee
    glTranslatef((0.5*robotBodyWidth + 0.5*upperArmWidth), 0.5*upperArmLength, 0.0);
    glRotatef(shoulderAngle, 1.0, 0.0, 0.0);
    glTranslatef((0.5*robotBodyWidth + 0.5*upperArmWidth), -0.5*upperArmLength, 0.0);

    // Position leg with respect to parent upper leg
    glTranslatef(-2.0, -5.0, -11.0);

    // build leg
    glPushMatrix();
    glScalef(1.0, 4.0, 1.0);
    glutSolidCube(3.0);
    glPopMatrix();

    //  foot-----------------------------------------------------
    glMaterialfv(GL_FRONT, GL_AMBIENT, gun_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, gun_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, gun_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, gun_mat_shininess);

    glPushMatrix();
    // Position foot with respect to parent leg
    glTranslatef(0, -(0.5*upperArmLength + 0.5*gunLength), 0.0);

    // build foot
    glScalef(4.0, -2.0, -2.0);
    glutSolidCube(1.0);
    glPopMatrix();
    //  foot-----------------------------------------------------

    glPopMatrix();
}

void drawLeftCannon()
{
    //    Black
    glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);

    glPushMatrix();  // 1
    
    // Rotate cannon around its own “length” axis
    glTranslatef(-5, 5, 0);
    glRotatef(cannonRotation, 0.0, 0.0, 1.0);
    glTranslatef(5, -5, 0);
    
    // Position whole cannon with respect to parent body ---------------
    glTranslatef(-5.0, 5.0, -1.0); // this will be done last

    // build cannon (torus) --------------------------------------------
    glPushMatrix();  // 2
    glScalef(1.0, 1.0, 7.0);
    glutSolidTorus(0.7, 1.0, 20, 20);
    glPopMatrix();  // 2
    
    //  CYLINDER --------------------------------------
    //    Chrome/silver looking
    glMaterialfv(GL_FRONT, GL_AMBIENT, gun_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, gun_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, gun_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, gun_mat_shininess);
    
    glPushMatrix();  //  3
    // Position cylinder thing with respect to parent cannon torus
    glTranslatef(0.0, 0.0, -5.0);

    // build cylinder thing ------------------------------------------------
    glScalef(1.0, 1.0, 1.0);
    GLUquadricObj*myCylcinder;
    myCylcinder = gluNewQuadric();
    gluQuadricDrawStyle(myCylcinder, GLU_FILL);
    gluCylinder(myCylcinder, 2.5, 2.5, 1.0, 20, 20);
//    glutSolidSphere(2.0, 8, 8);
    
    //  BACK DISK ----------------------------------------
    glPushMatrix();  // 4
    // Position circle thing with respect to cylinder cannon
    glTranslatef(0.0, 0.0, 0.0);
    
    // build circle thing (back)
    glScalef(1.0, 1.0, 1.0);
    GLUquadricObj*myDisk;
    myDisk = gluNewQuadric();
    gluQuadricDrawStyle(myDisk, GLU_FILL);
    gluDisk(myDisk, 0.0, 2.5, 20, 1);
    
    //  FRONT DISK ----------------------------------------
    glPushMatrix();  //  5
    // Position circle thing with respect to cylinder cannon
    glTranslatef(0.0, 0.0, 1.0);
    
    // build circle thing (front)
    glScalef(1.0, 1.0, 1.0);
    GLUquadricObj*myDiskF;
    myDiskF = gluNewQuadric();
    gluQuadricDrawStyle(myDiskF, GLU_FILL);
    gluDisk(myDisk, 0.0, 2.5, 20, 1);
    
    //  SMALL CANNON SUB-PART ----------------------------------------
    glPushMatrix();  // 6
    // Position circle thing with respect to cylinder cannon
    glRotatef(270, 1.0, 0.0, 0.0);
    glTranslatef(0.0, -7.0, 1.2);
    
    // build circle thing (back)
    glutSolidCone(1.0, 1.0, 20, 20);

    glPopMatrix();  //  6 FRONT DISK -----------------------

    glPopMatrix();  //  5
    
    glPopMatrix();  //  4

    glPopMatrix();  //  3
    
    glPopMatrix();  //  1

}

void drawRightCannon()
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);

    glPushMatrix();
    
    // Position cannon with respect to parent body
    glTranslatef(5.0, 5.0, -1.0); // this will be done last

    // build cannon
    glPushMatrix();
    glScalef(1.0, 1.0, 7.0);
    glutSolidTorus(0.7, 1.0, 20, 20);
    glPopMatrix();
    
    //  Circle thing at the back
    glMaterialfv(GL_FRONT, GL_AMBIENT, gun_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, gun_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, gun_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, gun_mat_shininess);
    
    glPushMatrix();
    // Position circle thing with respect to parent cannon
    glTranslatef(0.0, 0.0, -5.0);

    // build cylinder thing------------------------------------------------------------
    glScalef(1.0, 1.0, 1.0);
    GLUquadricObj*myCylcinder;
    myCylcinder = gluNewQuadric();
    gluQuadricDrawStyle(myCylcinder, GLU_FILL);
    gluCylinder(myCylcinder, 2.5, 2.5, 1.0, 20, 20);
//    glutSolidSphere(2.0, 8, 8);
    
    //  disk thing at the back
    glMaterialfv(GL_FRONT, GL_AMBIENT, gun_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, gun_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, gun_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, gun_mat_shininess);
    
    glPushMatrix();
    // Position circle thing with respect to cylinder cannon
    glTranslatef(0.0, 0.0, 0.0);
    
    // build circle thing (back)------------------------------------------------------------
    glScalef(1.0, 1.0, 1.0);
    GLUquadricObj*myDisk;
    myDisk = gluNewQuadric();
    gluQuadricDrawStyle(myDisk, GLU_FILL);
    gluDisk(myDisk, 0.0, 2.5, 20, 1);
    
    //  disk thing at the front
    glMaterialfv(GL_FRONT, GL_AMBIENT, gun_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, gun_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, gun_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, gun_mat_shininess);
    
    glPushMatrix();
    // Position circle thing with respect to cylinder cannon
    glTranslatef(0.0, 0.0, 1.0);
    
    // build circle thing (front)------------------------------------------------------------
    glScalef(1.0, 1.0, 1.0);
    GLUquadricObj*myDiskF;
    myDiskF = gluNewQuadric();
    gluQuadricDrawStyle(myDiskF, GLU_FILL);
    gluDisk(myDisk, 0.0, 2.5, 20, 1);
    
    glPopMatrix();
    
    glPopMatrix();
    
    glPopMatrix();
    
    glPopMatrix();

}

// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
	// Set up viewport, projection, then change to modelview matrix mode - 
	// display function will then set up camera and do modeling transforms.
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)w / h, 0.2, 40.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

bool stop = false;
bool walk = false;

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 't':

		break;
    case 'k':
        kneeSelect = true;
        hipSelect = false;
        bodySelect = false;
        break;
    case 'h':
        hipSelect = true;
        kneeSelect = false;
        bodySelect = false;
        break;
    case 'b':
        bodySelect = true;
        kneeSelect = false;
        hipSelect = false;
        break;
	case 'r':
		robotAngle += 2.0;
		break;
	case 'R':
		robotAngle -= 2.0;
		break;
    case 'c':
        stop = false;
        glutTimerFunc(10, cannonAnimation, 0);
        break;
    case 'C':
        stop = true;
        break;
    case 'w':
        walk = true;
        glutTimerFunc(10, walkAnimation, 0);
        break;
    case 'W':
        walk = false;
        glutTimerFunc(10, undoWalk, 0);
        break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}


void walkAnimation(int param)
{
    if (walk)
    {
        if (hipJointAngle <= 40.0)
        {
            hipJointAngle += 1.0;
        }
        if (kneeJointAngle >= -30.0)
        {
            kneeJointAngle -= 1.0;
        }
        glutPostRedisplay();
        glutTimerFunc(10, walkAnimation, 0);
    }
}

void undoWalk(int param)
{
    if (!walk)
    {
        hipJointAngle = 0.0;
        kneeJointAngle = -40.0;
        bodyJointAngle = 0.0;
        glutPostRedisplay();
    }
}

void cannonAnimation(int param)
{
    if (!stop)
    {
        cannonRotation += 1.0;
        glutPostRedisplay();
        glutTimerFunc(10, cannonAnimation, 0);
    }
}

// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
	// Help key
	if (key == GLUT_KEY_F1)
	{

	}
	// Do transformations with arrow keys
	else if (key == GLUT_KEY_DOWN or key == GLUT_KEY_LEFT)   // GLUT_KEY_DOWN, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_LEFT
	{
        if (kneeSelect) { kneeJointAngle -= 2.0; }
        else if (hipSelect) { hipJointAngle -= 2.0; }
        else if (bodySelect) { bodyJointAngle -= 2.0; }
	}
    else if (key == GLUT_KEY_UP or key == GLUT_KEY_RIGHT)
    {
        if (kneeSelect) { kneeJointAngle += 2.0; }
        else if (hipSelect) { hipJointAngle += 2.0; }
        else if (bodySelect) { bodyJointAngle += 2.0; }
    }

	glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
	currentButton = button;

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			;

		}
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{
			;
		}
		break;
	default:
		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse motion callback - use only if you want to 
void mouseMotionHandler(int xMouse, int yMouse)
{
	if (currentButton == GLUT_LEFT_BUTTON)
	{
		;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}

