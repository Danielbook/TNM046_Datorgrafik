/*
 * A C++ framework for OpenGL programming in TNM046 for MT1 2014.
 *
 * This is a small, limited framework, designed to be easy to use
 * for students in an introductory computer graphics course in
 * the first year of a M Sc curriculum. It uses custom code
 * for some things that are better solved by external libraries
 * like GLEW and GLM, but the emphasis is on simplicity and
 * readability, not generality.
 * For the window management, GLFW 3.0 is used for convenience.
 * The framework should work in Windows, MacOS X and Linux.
 * Some Windows-specific stuff for extension loading is still
 * here. GLEW could have been used instead, but for clarity
 * and minimal dependence on other code, we rolled our own extension
 * loading for the things we need. That code is short-circuited on
 * platforms other than Windows. This code is dependent only on
 * the GLFW and OpenGL libraries. OpenGL 3.3 or higher is required.
 *
 * Author: Stefan Gustavson (stegu@itn.liu.se) 2013-2014
 * This code is in the public domain.
 */

// File and console I/O for logging and error reporting
#include <iostream>

#include <Utilities.hpp>
#include <Shader.hpp>
#include <math.h>
#include <TriangleSoup.hpp>
#include <Texture.hpp>
#include <Rotator.hpp>

// In MacOS X, tell GLFW to include the modern OpenGL headers.
// Windows does not want this, so we make this Mac-only.
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#endif

// GLFW 3.x, to handle the OpenGL window
#include <GLFW/glfw3.h>

void createVertexBuffer(int location, int dimensions, const float *data, int datasize) {

	GLuint bufferID;

	// Generate buffer, activate it and copy the data
	glGenBuffers(1, &bufferID);
	glBindBuffer(GL_ARRAY_BUFFER, bufferID);
	glBufferData(GL_ARRAY_BUFFER, datasize, data, GL_STATIC_DRAW);

	// Tell OpenGL how the data is stored in our buffer
	// Attribute location (must match layout(location=#) statement in shader)
	// Number of dimensions (3 -> vec3 in the shader, 2-> vec2 in the shader),
	// type GL_FLOAT, not normalized, stride 0, start at element 0
	glVertexAttribPointer(location, dimensions, GL_FLOAT, GL_FALSE, 0, NULL);

	// Enable the attribute in the currently bound VAO
	glEnableVertexAttribArray(location);
}

void createIndexBuffer(const unsigned int *data, int datasize) {
	GLuint bufferID;

	// Generate buffer, activate it and copy the data
	glGenBuffers(1, &bufferID);

    // Activate (bind) the index buffer and copy data to it.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID);

    // Present our vertex indices to OpenGL
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, datasize, data, GL_STATIC_DRAW);
}

// Multiply 4x4 matrice M1 and M2 and put the result in Mout
void mat4mult(float M1[], float M2[], float Mout[]) {

    float Mtemp[16];
    Mtemp[0] = M1[0]*M2[0] + M1[4]*M2[1] + M1[8]*M2[2] + M1[12]*M2[3];
    Mtemp[4] = M1[0]*M2[4] + M1[4]*M2[5] + M1[8]*M2[6] + M1[12]*M2[7];
    Mtemp[8] = M1[0]*M2[8] + M1[4]*M2[9] + M1[8]*M2[10] + M1[12]*M2[11];
    Mtemp[12] = M1[0]*M2[12] + M1[4]*M2[13] + M1[8]*M2[14] + M1[12]*M2[15];

    Mtemp[1] = M1[1]*M2[0] + M1[5]*M2[1] + M1[9]*M2[2] + M1[13]*M2[3];
    Mtemp[5] = M1[1]*M2[4] + M1[5]*M2[5] + M1[9]*M2[6] + M1[13]*M2[7];
    Mtemp[9] = M1[1]*M2[8] + M1[5]*M2[9] + M1[9]*M2[10] + M1[13]*M2[11];
    Mtemp[13] = M1[1]*M2[12] + M1[5]*M2[13] + M1[9]*M2[14] + M1[13]*M2[15];

    Mtemp[2] = M1[2]*M2[0] + M1[6]*M2[1] + M1[10]*M2[2] + M1[14]*M2[3];
    Mtemp[6] = M1[2]*M2[4] + M1[6]*M2[5] + M1[10]*M2[6] + M1[14]*M2[7];
    Mtemp[10] = M1[2]*M2[8] + M1[6]*M2[9] + M1[10]*M2[10] + M1[14]*M2[11];
    Mtemp[14] = M1[2]*M2[12] + M1[6]*M2[13] + M1[10]*M2[14] + M1[14]*M2[15];

    Mtemp[3] = M1[3]*M2[0] + M1[7]*M2[1] + M1[11]*M2[2] + M1[15]*M2[3];
    Mtemp[7] = M1[3]*M2[4] + M1[7]*M2[5] + M1[11]*M2[6] + M1[15]*M2[7];
    Mtemp[11] = M1[3]*M2[8] + M1[7]*M2[9] + M1[11]*M2[10] + M1[15]*M2[11];
    Mtemp[15] = M1[3]*M2[12] + M1[7]*M2[13] + M1[11]*M2[14] + M1[15]*M2[15];

    for(int i = 0; i < 16; i++){
        Mout[i] = Mtemp[i];
    }
}

void mat4identity(float M[]){
    M[0] = 1;
    M[1] = 0;
    M[2] = 0;
    M[3] = 0;

    M[4] = 0;
    M[5] = 1;
    M[6] = 0;
    M[7] = 0;

    M[8]  = 0;
    M[9]  = 0;
    M[10] = 1;
    M[11] = 0;

    M[12] = 0;
    M[13] = 0;
    M[14] = 0;
    M[15] = 1;
}

void mat4rotx(float M[], float angle){
    mat4identity(M);

    M[5] = cos(angle);
    M[6] = sin(angle);
    M[9] = -sin(angle);
    M[10] = cos(angle);
}

void mat4roty(float M[], float angle){
    mat4identity(M);

    M[0] = cos(angle);
    M[2] = -sin(angle);
    M[8] = sin(angle);
    M[10] = cos(angle);
}

void mat4rotz(float M[], float angle){
    mat4identity(M);

    M[0] = cos(angle);
    M[1] = sin(angle);
    M[4] = -sin(angle);
    M[5] = cos(angle);

}

void mat4scale(float M[], float scale){
    mat4identity(M);

    M[0] = scale;
    M[5] = scale;
    M[10] = scale;
    M[15] = scale;
}

void mat4translate(float M[], float x, float y, float z){
    mat4identity(M);

    M[12] = x;
    M[13] = y;
    M[14] = z;
}

void mat4perspective(float M[], float vfov, float aspect, float znear, float zfar){
    mat4identity(M);
    float f = 1.0/tan(vfov/2);

    M[0] = f/aspect;
    M[5] = f;
    M[10] = -( (zfar + znear) / (zfar - znear) );
    M[11] = -1;
    M[14] = -( (2*zfar*znear)/(zfar-znear) );
    M[15] = 0;
}
/*
 * main(argc, argv) - the standard C++ entry point for the program
 */
int main(int argc, char *argv[]) {

    using namespace std;

	int width, height;

    const GLFWvidmode *vidmode;  // GLFW struct to hold information about the display
	GLFWwindow *window;    // GLFW struct to hold information about the window

    // Initialise GLFW
    glfwInit();

    /////////////////
	Shader myShader;

	KeyRotator myKeyRotator;
	MouseRotator myMouseRotator;

	//Texture myTexture;

	Texture dinoTexture;
	Texture earthTexture;

	GLint location_tex;

    float MV[16];
    mat4identity(MV);
    GLint location_MV;

    float P[16];
    mat4identity(P);
    GLint location_P;

    float LV[16];
    mat4identity(LV);
    GLint location_LV;

    float T[16];
    mat4identity(T);

    float R[16];
    mat4identity(R);

    float Rx[16];
    mat4identity(Rx);

    float Rz[16];
    mat4identity(Rz);

	float time;

	GLuint location_time;

	//TriangleSoup myShape;

	TriangleSoup dino;
	TriangleSoup earth;

    // Determine the desktop size
    vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	// Make sure we are getting a GL context of at least version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Exclude old legacy cruft from the context. We don't need it, and we don't want it.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Open a square window (aspect 1:1) to fill half the screen height
    window = glfwCreateWindow(vidmode->height/1, vidmode->height/1, "GLprimer", NULL, NULL);
    if (!window)
    {
        cout << "Unable to open window. Terminating." << endl;
        glfwTerminate(); // No window was opened, so we can't continue in any useful way
        return -1;
    }

    // Make the newly created window the "current context" for OpenGL
    // (This step is strictly required, or things will simply not work)
    glfwMakeContextCurrent(window);

    // Show some useful information on the GL context
    cout << "GL vendor:       " << glGetString(GL_VENDOR) << endl;
    cout << "GL renderer:     " << glGetString(GL_RENDERER) << endl;
    cout << "GL version:      " << glGetString(GL_VERSION) << endl;
    cout << "Desktop size:    " << vidmode->width << "x" << vidmode->height << " pixels" << endl;

    glfwSwapInterval(0); // Do not wait for screen refresh between frames

    myShader.createShader("vertex.glsl", "fragment.glsl");

    // Locate the sampler2D uniform in the shader program
    location_tex = glGetUniformLocation(myShader.programID, "tex");
    // Generate one texture object with data from a TGA file
    //myTexture.createTexture ("textures/trex.tga");
    dinoTexture.createTexture ("textures/trex.tga");
    earthTexture.createTexture ("textures/earth.tga");

    location_time = glGetUniformLocation(myShader.programID, "time");
    if(location_time == -1){
        cout << "Unable to locate variable 'time' in shader!" << endl;
    }

    myKeyRotator.init(window);
    myMouseRotator.init(window);

    //myShape.createSphere(0.5, 100);
    //myShape.createBox(0.5, 0.5, 0.5);
    //myShape.readOBJ("meshes/trex.obj");
    dino.readOBJ("meshes/trex.obj");
    earth.createSphere(0.25, 20);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Main loop
    while(!glfwWindowShouldClose(window))
    {
        // Get window size. It may start out different from the requested
        // size, and will change if the user resizes the window.
        glfwGetWindowSize( window, &width, &height );
        // Set viewport. This is the pixel rectangle we want to draw into.
        glViewport( 0, 0, width, height ); // The entire window
		// Set the clear color and depth, and clear the buffers for drawing
        glClearColor( 0.3f, 0.3f, 0.3f, 0.0f );

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //////////////////RENDERING CODE BELOW//////////////////////////
        Utilities::displayFPS(window);

        glUseProgram(myShader.programID);

        time = (float)glfwGetTime(); //Number of seconds since the program was started

        location_MV = glGetUniformLocation(myShader.programID, "MV");
        location_P = glGetUniformLocation(myShader.programID, "P");
        location_LV = glGetUniformLocation(myShader.programID, "LV");

        glUniform1f(location_time, time);

        myMouseRotator.poll(window);

        mat4rotz(Rz, myMouseRotator.phi);
        mat4rotx(Rx, myMouseRotator.theta);

        mat4mult(Rz, Rx, LV);

        glUniformMatrix4fv(location_LV, 1, GL_FALSE, LV); //Copy the value

        mat4identity(MV);

        myKeyRotator.poll(window);

        mat4roty(Rz, myKeyRotator.phi);
        mat4rotx(Rx, myKeyRotator.theta);

        mat4mult(Rz, Rx, MV);

        mat4translate(T, 0, 0, -5.0);
        mat4mult(T, MV, MV);

        mat4perspective(P, M_PI/6, 1, 0.1, 100.0);

        glUniformMatrix4fv(location_MV, 1, GL_FALSE, MV); //Copy the value
        glUniformMatrix4fv(location_P, 1, GL_FALSE, P); //Copy the value

        // Draw the TriangleSoup object mySphere
        // with a shader program that uses a texture
        glUseProgram (myShader.programID);
        //glBindTexture (GL_TEXTURE_2D, myTexture.textureID);
        glBindTexture (GL_TEXTURE_2D, dinoTexture.textureID);
        glUniform1i (location_tex, 0);

        dino.render();

        ////////////////
        mat4rotz(MV, M_PI/2);

        mat4translate(MV, 0, 0, -1.2);
        mat4roty(R, time);
        mat4translate(T, 0.0, 0.0, -5.0);

        mat4mult(R, MV, MV);
        mat4mult(T, MV, MV);

        glBindTexture (GL_TEXTURE_2D, earthTexture.textureID);
        glUniform1i (location_tex, 0);

        glUniformMatrix4fv(location_MV, 1, GL_FALSE, MV); //Copy the value
        glUniformMatrix4fv(location_P, 1, GL_FALSE, P); //Copy the value

        earth.render();

        glBindTexture (GL_TEXTURE_2D, 0);
        glUseProgram (0);

//              LABB 4
/////////////////////////////////////////////////////////////////////////
//        mat4rotx(MV, M_PI/8);
//
//        mat4translate(MV, 0, 0, -3);
//
//        mat4perspective(P, M_PI/4, 1, 0.1, 100.0);
//
//        mat4roty(MV, time);
//
//        myShape.render();
/////////////////////////////////////////////////////////////////////////

		// Swap buffers, i.e. display the image and prepare for next frame.
        glfwSwapBuffers(window);

		// Poll events (read keyboard and mouse input)
		glfwPollEvents();

        // Exit if the ESC key is pressed (and also if the window is closed).
        if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
          glfwSetWindowShouldClose(window, GL_TRUE);
        }

    }

    // Close the OpenGL window and terminate GLFW.
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
