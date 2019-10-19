/*
 * TextureProj.cpp - Version 0.2 - February 5, 2018
 *
 * Starting code for Math 155A, Project #6,
 * 
 * Author: Sam Buss
 *
 * Software accompanying POSSIBLE SECOND EDITION TO the book
 *		3D Computer Graphics: A Mathematical Introduction with OpenGL,
 *		by S. Buss, Cambridge University Press, 2003.
 *
 * Software is "as-is" and carries no warranty.  It may be used without
 *   restriction, but if you modify it, please change the filenames to
 *   prevent confusion between different versions.
 * Bug reports: Sam Buss, sbuss@ucsd.edu.
 * Web page: http://math.ucsd.edu/~sbuss/MathCG2
 */

// These libraries are needed to link the program.
// First five are usually provided by the system.
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"glew32s.lib")
#pragma comment(lib,"glew32.lib")


// Use the static library (so glew32.dll is not needed):
#define GLEW_STATIC

#include "GL/glew.h" 
#include "GLFW/glfw3.h"

#include "LinearR3.h"		// Adjust path as needed.
#include "LinearR4.h"		// Adjust path as needed.
#include "EduPhong.h"
#include "PhongData.h"
#include "ShaderBuild.h"
#include "GlGeomSphere.h"
#include "GlGeomCylinder.h"

// Enable standard input and output via printf(), etc.
// Put this include *after* the includes for glew and GLFW!
#include <stdio.h>

#include "TextureProj.h"
#include "MyGeometries.h"



// Animation controls and state infornation

// These variables control the view direction.
//    The arrow keys are used to change these values.
double viewAzimuth = 0.25;	// Angle of view up/down (in radians)
double viewDirection = 0.0; // Rotation of view around y-axis (in radians)
double deltaAngle = 0.01;	// Change in view angle for each up/down/left/right arrow key press
LinearMapR4 viewMatrix;		// The current view matrix, based on viewAzimuth and viewDirection.

// This variable controls whether running or paused.
bool spinMode = true;

// Control Phong lighting modes
// Use Gouraud or not.  (true == use Gouraud).
bool UsePhongGouraud = false;
phGlobal globalPhongData;

// These two variables control how triangles are rendered.
bool wireframeMode = false;	// Equals true for polygon GL_LINE mode. False for polygon GL_FILL mode.
bool cullBackFaces = true;

// The next variable controls the resolution of the meshes for cylinders and spheres.
int meshRes=4;             // Resolution of the meshes (slices, stacks, and rings all equal)

// These variables control the animation's state and speed.
// YOU PROBABLY WANT TO RE-DO THIS FOR YOUR CUSTOM ANIMATION.  
double animateIncrement = 0.01;   // Make bigger to speed up animation, smaller to slow it down.
double currentTime = 0.0;         // Current "time" for the animation.
double currentDelta = 0.0;        // Current state of the animation (YOUR CODE MAY NOT WANT TO USE THIS.)

// General data helping with setting up VAO (Vertex Array Objects)
//    and Vertex Buffer Objects.
unsigned int projMatLocation;						// Location of the projectionMatrix in the currently active shader program
unsigned int modelviewMatLocation;					// Location of the modelviewMatrix in the currently active shader program
//unsigned int projMatLocationS;						// Location of the projectionMatrixS in the currently active shader program
//unsigned int modelviewMatLocationS;					// Location of the modelviewMatrixS in the currently active shader program
unsigned int applyTextureLocation; 					// Location of the applyTexture bool in the currently active shader program

//  The Projection matrix: Controls the "camera view/field-of-view" transformation
//     Generally is the same for all objects in the scene.
LinearMapR4 theProjectionMatrix;		//  The Projection matrix: Controls the "camera/view" transformation


// These variables set the dimensions of the perspective region we wish to view.
// They are used to help form the projection matrix and the view matrix
// All rendered objects lie in the rectangular prism centered on the z-axis
//     equal to (-Xmax,Xmax) x (-Ymax,Ymax) x (Zmin,Zmax)
// Be sure to leave some slack in these values, to allow for rotations, etc.
// The model/view matrix can be used to move objects to this position
// THESE VALUES MAY NEED AD-HOC ADJUSTMENT TO GET THE SCENE TO BE VISIBLE.
/*
const double Xmax = 8.0;                // Control x dimensions of viewable scene
const double Ymax = 6.0;                // Control y dimensions of viewable scene
const double Zmin = -9.0, Zmax = 9.0;   // Control z dimensions of the viewable scene

// zDistance equals the initial distance from the camera to the z = Zmax plane
const double zDistance = 20.0;              // Make this value larger or smaller to affect field of view.

double ZextraDistance = 0.0;              // Extra distance we have moved to/from the scene
double ZextraDelta = 0.2;                // Pressing HOME/END moves closer/farther by this amount
const double ZextraDistanceMin = -15.0;
const double ZextraDistanceMax = 50.0;
int screenWidth = 800, screenHeight = 600;     // Width and height in pixels. Initially 800x600
*/
const double Xmax = 8.0;                // Control x dimensions of viewable scene
const double Ymax = 6.0;                // Control y dimensions of viewable scene
const double Zmin = -9.0, Zmax = 9.0;   // Control z dimensions of the viewable scene

										// zDistance equals the initial distance from the camera to the z = Zmax plane
const double zDistance = 20.0;              // Make this value larger or smaller to affect field of view.

double ZextraDistance = 0.0;              // Extra distance we have moved to/from the scene
double ZextraDelta = 0.2;                // Pressing HOME/END moves closer/farther by this amount
const double ZextraDistanceMin = -15.0;
const double ZextraDistanceMax = 50.0;
int screenWidth = 800, screenHeight = 600;     // Width and height in pixels. Initially 800x600



//								 l   r   b  t  n   f
//theProjectionMatrix.Set_glOrtho(-8 , 8, -6, 6, 20, 38);

int viewMode = 0;             // 0 -> Perspective, 1 -> Ortho (Shadow mapping)


// mySetupGeometries defines the scene data, especially vertex  positions and colors.
//    - It also loads all the data into the VAO's (Vertex Array Objects) and
//      into the VBO's (Vertex Buffer Objects).
// This routine is only called once to initialize the data.

void mySetupGeometries()
{
    MySetupSurfaces();

    mySetViewMatrix();

    check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!
}


void mySetViewMatrix()
{
    // Set the view matrix. Sets view distance, and view direction.
    // The final translation is done because the ground plane lies in the xz-plane,
    //    se the center of the scene is about 3 or 4 units above the origin.
    // YOU MAY NEED TO ADJUST THE FINAL TRANSLATION AND?OR ADD A SCALING TO MAKE THE SCENE VISIBLE.

	/*
	viewMatrix.Set_glTranslate(0.0, 0.0, -(Zmax + zDistance + ZextraDistance));      // Translate to be in front of the camera
	viewMatrix.Mult_glRotate(viewAzimuth, 1.0, 0.0, 0.0);	    // Rotate viewAzimuth radians around x-axis
	viewMatrix.Mult_glRotate(-viewDirection, 0.0, 1.0, 0.0);    // Rotate -viewDirection radians around y-axis
	viewMatrix.Mult_glTranslate(0.0, -3.5, 0.0);                // Translate the scene down the y-axis so the center is near the origin.
	*/
	
	if (viewMode == 0)
	{
		viewMatrix.Set_glTranslate(0.0, 0.0, -(Zmax + zDistance + ZextraDistance));      // Translate to be in front of the camera
		viewMatrix.Mult_glRotate(viewAzimuth, 1.0, 0.0, 0.0);	    // Rotate viewAzimuth radians around x-axis
		viewMatrix.Mult_glRotate(-viewDirection, 0.0, 1.0, 0.0);    // Rotate -viewDirection radians around y-axis
		viewMatrix.Mult_glTranslate(0.0, -3.5, 0.0);                // Translate the scene down the y-axis so the center is near the origin.
	}
	else if (viewMode == 1)
	{
		// Light is directly above

		//viewMatrix.Set_glTranslate(0.0, 0.0, -(Zmax + zDistance + ZextraDistance));      // Translate to be in front of the camera
		viewMatrix.Set_glTranslate(0.0, 0.0, -(Zmax + zDistance));
		viewMatrix.Mult_glRotate(PI/2.0, 1.0, 0.0, 0.0);	    // Rotate viewAzimuth radians around x-axis
		//viewMatrix.Mult_glRotate(-0, 0.0, 1.0, 0.0);    // Rotate -viewDirection radians around y-axis
		viewMatrix.Mult_glTranslate(0.0, -3.5, 0.0);                // Translate the scene down the y-axis so the center is near the origin.
	}
	else if (viewMode == 2)
	{
		viewMatrix.Set_glTranslate(0.0, 0.0, -(Zmax + zDistance + ZextraDistance));      // Translate to be in front of the camera
																						 //viewMatrix.Mult_glRotate(viewAzimuth, 1.0, 0.0, 0.0);	    // Rotate viewAzimuth radians around x-axis
																						 //viewMatrix.Mult_glRotate(-viewDirection, 0.0, 1.0, 0.0);    // Rotate -viewDirection radians around y-axis
		viewMatrix.Mult_glTranslate(0.0, -3.5, 0.0);                // Translate the scene down the y-axis so the center is near the origin.
	}
	
}


// Main routine for rendering the scene
// myRenderScene() is called every time the scene needs to be redrawn.
// mySetupGeometries() has already created the vertex and buffer objects
//    and the model view matrices.
// The EduPhong shaders are already setup.

void myRenderScene()
{   
    // Clear the rendering window
    
	static const float white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    const float clearDepth = 1.0f;
    glClearBufferfv(GL_COLOR, 0, white);
    glClearBufferfv(GL_DEPTH, 0, &clearDepth);	// Must pass in a *pointer* to the depth

	/*
	viewMode = 1;
	mySetViewMatrix();
	setProjectionMatrix();

	// First pass: render shadowmap to depth texture

	GLuint Shadowbuffer;
	glGenFramebuffers(1, &Shadowbuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, Shadowbuffer);

	GLuint depthTexture;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthTexture, 0);

	GLuint rboDepthStencil3;
	glGenRenderbuffers(1, &rboDepthStencil3);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil3);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencil3);

	glBindFramebuffer(GL_FRAMEBUFFER, Shadowbuffer);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);   //////////////////////
	glViewport(0, 0, screenWidth, screenHeight);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	UsePhongGouraud = false;
	projMatLocation = UsePhongGouraud ? projMatLocationPG : projMatLocationPP;
	modelviewMatLocation = UsePhongGouraud ? modelviewMatLocationPG : modelviewMatLocationPP;
	applyTextureLocation = UsePhongGouraud ? applyTextureLocationPG : applyTextureLocationPP;
	glUseProgram(UsePhongGouraud ? phShaderPhongGouraud : phShaderPhongPhong);

	glUniform1i(applyTextureLocation, 3);
	MyRenderGeometries();
	*/

    // Second pass: render normals to texture

	viewMode = 0;
	mySetViewMatrix();
	setProjectionMatrix();

	GLuint FramebufferName;
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	
	GLuint renderedTexture;
	glGenTextures(1, &renderedTexture);
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);
	
	GLuint rboDepthStencil;
	glGenRenderbuffers(1, &rboDepthStencil);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencil);

	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	glViewport(0, 0, screenWidth, screenHeight);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	UsePhongGouraud = false;
	projMatLocation = UsePhongGouraud ? projMatLocationPG : projMatLocationPP;
	modelviewMatLocation = UsePhongGouraud ? modelviewMatLocationPG : modelviewMatLocationPP;
	applyTextureLocation = UsePhongGouraud ? applyTextureLocationPG : applyTextureLocationPP;
	glUseProgram(UsePhongGouraud ? phShaderPhongGouraud : phShaderPhongPhong);
	
	glUniform1i(applyTextureLocation, 0);
	MyRenderGeometries();

	// Third pass: render depth and more normals to texture

	GLuint FramebufferName1;
	glGenFramebuffers(1, &FramebufferName1);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName1);

	GLuint renderedTexture1;
	glGenTextures(1, &renderedTexture1);
	glBindTexture(GL_TEXTURE_2D, renderedTexture1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture1, 0);

	GLuint rboDepthStencil1;
	glGenRenderbuffers(1, &rboDepthStencil1);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil1);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencil1);

	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName1);
	glViewport(0, 0, screenWidth, screenHeight);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniform1i(applyTextureLocation, 1);
	MyRenderGeometries();


	// Fourth pass: render cel shaded scene to texture

	GLuint FramebufferName2;
	glGenFramebuffers(1, &FramebufferName2);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName2);

	GLuint renderedTexture2;
	glGenTextures(1, &renderedTexture2);
	glBindTexture(GL_TEXTURE_2D, renderedTexture2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture2, 0);

	GLuint rboDepthStencil2;
	glGenRenderbuffers(1, &rboDepthStencil2);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil2);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencil2);

	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName2);
	glViewport(0, 0, screenWidth, screenHeight);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glClearBufferfv(GL_COLOR, 0, white);
	glClearBufferfv(GL_DEPTH, 0, &clearDepth);	// Must pass in a *pointer* to the depth
	UsePhongGouraud = false;
	projMatLocation = UsePhongGouraud ? projMatLocationPG : projMatLocationPP;
	modelviewMatLocation = UsePhongGouraud ? modelviewMatLocationPG : modelviewMatLocationPP;
	applyTextureLocation = UsePhongGouraud ? applyTextureLocationPG : applyTextureLocationPP;
	glUseProgram(UsePhongGouraud ? phShaderPhongGouraud : phShaderPhongPhong);

	//
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//

	/*
	int texLoc = glGetUniformLocation(phShaderPhongPhong, "depthTexture");
	glUniform1i(texLoc, 1);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthTexture);

	glActiveTexture(GL_TEXTURE0);
	*/

	glUniform1i(applyTextureLocation, 2);
	MyRenderGeometries();


	// Final pass: render contours


	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	UsePhongGouraud = true;
	projMatLocation = UsePhongGouraud ? projMatLocationPG : projMatLocationPP;
	modelviewMatLocation = UsePhongGouraud ? modelviewMatLocationPG : modelviewMatLocationPP;
	applyTextureLocation = UsePhongGouraud ? applyTextureLocationPG : applyTextureLocationPP;
	glUseProgram(UsePhongGouraud ? phShaderPhongGouraud : phShaderPhongPhong);

	int texLoc = glGetUniformLocation(phShaderPhongGouraud, "renderedTexture");
	glUniform1i(texLoc, 0);

	texLoc = glGetUniformLocation(phShaderPhongGouraud, "renderedTexture2");
	glUniform1i(texLoc, 1);

	texLoc = glGetUniformLocation(phShaderPhongGouraud, "renderedTexture1");
	glUniform1i(texLoc, 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderedTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderedTexture2);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, renderedTexture1);


	MyRenderPass2();
	//MyRenderSpheresForLights();

	glActiveTexture(GL_TEXTURE0);

	glDeleteTextures(1, &renderedTexture);
	glDeleteRenderbuffers(1, &rboDepthStencil);
	glDeleteFramebuffers(1, &FramebufferName);

	glDeleteTextures(1, &renderedTexture1);
	glDeleteRenderbuffers(1, &rboDepthStencil1);
	glDeleteFramebuffers(1, &FramebufferName1);

	glDeleteTextures(1, &renderedTexture2);
	glDeleteRenderbuffers(1, &rboDepthStencil2);
	glDeleteFramebuffers(1, &FramebufferName2);


	//glDeleteTextures(1, &depthTexture);
	//glDeleteRenderbuffers(1, &rboDepthStencil3);
	//glDeleteFramebuffers(1, &Shadowbuffer);




	//viewMode = 0;

    check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!
}


void my_setup_SceneData()
{
	setup_phong_shaders();
	mySetupGeometries();
    SetupForTextures();

    projMatLocation = UsePhongGouraud ? projMatLocationPG : projMatLocationPP;
    modelviewMatLocation = UsePhongGouraud ? modelviewMatLocationPG : modelviewMatLocationPP;
    applyTextureLocation = UsePhongGouraud ? applyTextureLocationPG : applyTextureLocationPP;

    MySetupGlobalLight();
    MySetupLights();
    LoadAllLights();
    MySetupMaterials();


	check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!
}


// Process all key press events.
// This routine is called each time a key is pressed or released.

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    static const double Pi = 3.1415926535f;

    if (action == GLFW_RELEASE) {
        return;			// Ignore key up (key release) events
    }
    bool viewChanged = false;
    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, true);
        return;
    case '4':
    case '1':
    case '2':
    case '3':
    {
        phLight& theLight = myLights[key - '1'];
        theLight.IsEnabled = !theLight.IsEnabled;   // Toggle whether the light is enabled.
        LoadAllLights();
        return;
    }
    case 'R':
        spinMode = !spinMode;	// Toggle animation on and off.
        return;
    case 'W':		// Toggle wireframe mode
        if (wireframeMode) {
            wireframeMode = false;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else {
            wireframeMode = true;
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        return;
    case 'C':		// Toggle backface culling
        cullBackFaces = !cullBackFaces;     // Negate truth value of cullBackFaces
        if (cullBackFaces) {
            glEnable(GL_CULL_FACE);
        }
        else {
            glDisable(GL_CULL_FACE);
        }
        return;
    case 'F':
        if (mods & GLFW_MOD_SHIFT) {                // If upper case 'F'
            animateIncrement *= sqrt(2.0);			// Double the animation time step after two key presses
        }
        else {                                      // Else lose case 'f',
            animateIncrement *= sqrt(0.5);			// Halve the animation time step after two key presses
        }
        return;
    case GLFW_KEY_P:
        UsePhongGouraud = !UsePhongGouraud;
        projMatLocation = UsePhongGouraud ? projMatLocationPG : projMatLocationPP;
        modelviewMatLocation = UsePhongGouraud ? modelviewMatLocationPG : modelviewMatLocationPP;
        applyTextureLocation = UsePhongGouraud ? applyTextureLocationPG : applyTextureLocationPP;
        return;
    case GLFW_KEY_UP:
        viewAzimuth = Min(viewAzimuth + 0.01, PIfourths - 0.05);
        viewChanged = true;
        break;
    case GLFW_KEY_DOWN:
        viewAzimuth = Max(viewAzimuth - 0.01, -PIfourths + 0.05);
        viewChanged = true;
        break;
    case GLFW_KEY_RIGHT:
        viewDirection += 0.01;
        if (viewDirection > PI) {
            viewDirection -= PI2;
        }
        viewChanged = true;
        break;
    case GLFW_KEY_LEFT:
        viewDirection -= 0.01;
        if (viewDirection < -PI) {
            viewDirection += PI2;
        }
        viewChanged = true;
        break;
    case GLFW_KEY_HOME:     
        ZextraDistance -= ZextraDelta;         // Move closer to the scene     
        ClampMin(&ZextraDistance, ZextraDistanceMin);
        viewChanged = true;
        break;
    case GLFW_KEY_END:
        ZextraDistance += ZextraDelta;         // Move farther away from the scene
        ClampMax(&ZextraDistance, ZextraDistanceMax);
        viewChanged = true;
        break;
    case GLFW_KEY_A:
        globalPhongData.EnableAmbient = !globalPhongData.EnableAmbient;
        break;
    case GLFW_KEY_E:
        globalPhongData.EnableEmissive = !globalPhongData.EnableEmissive;
        break;
    case GLFW_KEY_D:
        globalPhongData.EnableDiffuse = !globalPhongData.EnableDiffuse;
        break;
    case GLFW_KEY_S:
        globalPhongData.EnableSpecular = !globalPhongData.EnableSpecular;
        break;
    case GLFW_KEY_V:
        globalPhongData.LocalViewer = !globalPhongData.LocalViewer;
        break;

    }

    if (viewChanged) {
        mySetViewMatrix();
        setProjectionMatrix();
        LoadAllLights();        // Have to call this since it affects the position of the lights!
    }
    else {
        // Updated the global phong data above: upload it to the shader program.
        globalPhongData.LoadIntoShaders();
    }
}


// Called with the graphics window is first created and when resized

void window_size_callback(GLFWwindow* window, int width, int height)
{
    setProjectionMatrix();
}


void setProjectionMatrix()
{
	// Setup the projection matrix as a perspective view
	double w = (double)screenWidth;
	double h = (double)screenHeight;
	double windowXmax, windowYmax;
    double aspectFactor = w * Ymax / (h * Xmax);   // == (w/h)/(Xmax/Ymax), ratio of aspect ratios
	if (aspectFactor>1) {
		windowXmax = Xmax * aspectFactor;
		windowYmax = Ymax;
	}
	else {
		windowYmax = Ymax / aspectFactor;
		windowXmax = Xmax;
	}

	// Set up the orthographic projection
    double zNear = zDistance+ZextraDistance;
    double zFar = zNear + Zmax - Zmin;
    double scale = zNear / zDistance;

	double zNear2 = zDistance;
	double zFar2 = zNear2 + Zmax - Zmin;
	double scale2 = zNear2 / zDistance;

	
	if (viewMode == 0) {
		theProjectionMatrix.Set_glFrustum(-windowXmax * scale, windowXmax * scale,
			-windowYmax * scale, windowYmax * scale, zNear, zFar);
	}
	else if (viewMode == 1) {
		theProjectionMatrix.Set_glOrtho(-windowXmax * scale2, windowXmax * scale2,
			-windowYmax * scale2, windowYmax * scale2, zNear2, zFar2);
	}
	else if (viewMode == 2) {
		theProjectionMatrix.Set_glFrustum(-windowXmax * scale, windowXmax * scale,
			-windowYmax * scale, windowYmax * scale, zNear, zFar);
	}

    if (glIsProgram(phShaderPhongGouraud)) {
        glUseProgram(phShaderPhongGouraud);
        theProjectionMatrix.DumpByColumns(matEntries);
        glUniformMatrix4fv(projMatLocationPG, 1, false, matEntries);
    }
    if (glIsProgram(phShaderPhongPhong)) {
        glUseProgram(phShaderPhongPhong);
        theProjectionMatrix.DumpByColumns(matEntries);
        glUniformMatrix4fv(projMatLocationPP, 1, false, matEntries);
    }
    check_for_opengl_errors();
}


void my_setup_OpenGL()
{
	glEnable(GL_DEPTH_TEST);	// Enable depth buffering
	glDepthFunc(GL_LEQUAL);		// Useful for multipass shaders

	// Set polygon drawing mode for front and back of each polygon
    glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL );

    glEnable(GL_CULL_FACE);

	check_for_opengl_errors();
}


void error_callback(int error, const char* description)
{
	// Print error
	fputs(description, stderr);
}


void setup_callbacks(GLFWwindow* window)
{
	// Set callback function for resizing the window
	glfwSetFramebufferSizeCallback(window, window_size_callback);

	// Set callback for key up/down/repeat events
	glfwSetKeyCallback(window, key_callback);

	// Set callbacks for mouse movement (cursor position) and mouse botton up/down events.
	// glfwSetCursorPosCallback(window, cursor_pos_callback);
	// glfwSetMouseButtonCallback(window, mouse_button_callback);
}


int main()
{
	glfwSetErrorCallback(error_callback);	// Supposed to be called in event of errors
	glfwInit();
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Toon Shading", NULL, NULL);
	if (window == NULL) {
		printf("Failed to create GLFW window!\n");
		return -1;
	}
	glfwSetWindowSizeLimits(window, screenWidth, screenHeight, screenWidth, screenHeight);
	glfwMakeContextCurrent(window);

	if (GLEW_OK != glewInit()) {
		printf("Failed to initialize GLEW!.\n");
		return -1;
	}


	// Print info of GPU and supported OpenGL version
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("OpenGL version supported %s\n", glGetString(GL_VERSION));
#ifdef GL_SHADING_LANGUAGE_VERSION
	printf("Supported GLSL version is %s.\n", (char *)glGetString(GL_SHADING_LANGUAGE_VERSION));
#endif
    printf("Using GLEW version %s.\n", glewGetString(GLEW_VERSION));

	printf("------------------------------\n");
	printf("Press 'r' or 'R' (Run) to toggle(off and on) running the animation.\n");
    printf("Press arrow keys to adjust the view direction.\n");
    printf("Press HOME or END to closer to and farther away from the scene.\n");
    printf("Press 'w' or 'W' (wireframe) to toggle whether wireframe or fill mode.\n");
    printf("Press 'M' (mesh) to increase the mesh resolution.\n");
    printf("Press 'm' (mesh) to decrease the mesh resolution.\n");
    printf("Press 'P' key (Phong) to toggle using Phong shading and Gouraud shading.\n");
    printf("Press 'E' key (Emissive) to toggle rendering Emissive light.\n");
    printf("Press 'A' key (Ambient) to toggle rendering Ambient light.\n");
    printf("Press 'D' key (Diffuse) to toggle rendering Diffuse light.\n");
    printf("Press 'S' key (Specular) to toggle rendering Specular light.\n");
    printf("Press 'V' key (Viewer) to toggle using a local viewer.\n");
    printf("Press ESCAPE to exit.\n");
	
    setup_callbacks(window);
   
	// Initialize OpenGL, the scene and the shaders
    my_setup_OpenGL();
	my_setup_SceneData();
 	window_size_callback(window, screenWidth, screenHeight);

    // Loop while program is not terminated.
	while (!glfwWindowShouldClose(window)) {
	
		myRenderScene();				// Render into the current buffer
		glfwSwapBuffers(window);		// Displays what was just rendered (using double buffering).

		// Poll events (key presses, mouse events)
		glfwWaitEventsTimeout(1.0/60.0);	    // Use this to animate at 60 frames/sec (timing is NOT reliable)
		// glfwWaitEvents();					// Or, Use this instead if no animation.
		// glfwPollEvents();					// Use this version when animating as fast as possible
	}

	glfwTerminate();
	return 0;
}


// If an error is found, it could have been caused by any command since the
//   previous call to check_for_opengl_errors()
// To find what generated the error, you can try adding more calls to
//   check_for_opengl_errors().

char errNames[8][36] = {
	"Unknown OpenGL error",
	"GL_INVALID_ENUM", "GL_INVALID_VALUE", "GL_INVALID_OPERATION",
	"GL_INVALID_FRAMEBUFFER_OPERATION", "GL_OUT_OF_MEMORY",
	"GL_STACK_UNDERFLOW", "GL_STACK_OVERFLOW" };

bool check_for_opengl_errors()
{
	int numErrors = 0;
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		numErrors++;
		int errNum = 0;
		switch (err) {
		case GL_INVALID_ENUM:
			errNum = 1;
			break;
		case GL_INVALID_VALUE:
			errNum = 2;
			break;
		case GL_INVALID_OPERATION:
			errNum = 3;
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			errNum = 4;
			break;
		case GL_OUT_OF_MEMORY:
			errNum = 5;
			break;
		case GL_STACK_UNDERFLOW:
			errNum = 6;
			break;
		case GL_STACK_OVERFLOW:
			errNum = 7;
			break;
		}
		printf("OpenGL ERROR: %s.\n", errNames[errNum]);
	}
	return (numErrors != 0);
}