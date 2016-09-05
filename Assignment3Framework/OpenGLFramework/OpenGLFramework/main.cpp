/**
*	CS 334 - Fundamentals of Computer Graphics
*	Assignment 3 Framework
*
*	Important: The .obj file should contain triangle information instead of polygons
*
*	Instructions:
*	- Press ESC to exit
*/

#include <iostream>
#include <fstream>
#include <string>

#include "GL/glew.h"
#include "GL/glut.h"

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "stb_image.h"
#include "obj_parser.h"

/* Constant values */
const float PI = 3.14159265359;

/* Window information */
float windowWidth = 800;
float windowHeight = 600;

/* Information of the 3D model */
const char* modelFilename = "cube.obj";
GLfloat* modelVertices;
GLfloat* modelNormals;
GLfloat* modelCoords;
int modelVertexCount;

/* Information about the texture */
const char* textureFile = "crate.jpg";
unsigned char* textureData;
GLint textureDataLocation;
int textureWidth;
int textureHeight;
int textureComp;
GLuint crateTexture;

/* The vertex buffers and the vertex array for the 3D model */
GLuint verticesVbo;
GLuint coordsVbo;
GLuint modelVao;

/* The source code of the vertex and fragment shaders */
std::string vertexShaderCode;
std::string fragmentShaderCode;

/* References for the vertex and fragment shaders and the shader program */
GLuint vertexShader = 0;
GLuint fragmentShader = 0;
GLuint shaderProgram = 0;

/* The transformation matrices */
glm::mat4 modelMatrix;
glm::mat4 viewMatrix;
glm::mat4 projMatrix;

/* The location of the transformation matrices */
GLint modelMatrixLocation;
GLint viewMatrixLocation;
GLint projMatrixLocation;

/* Information of the rotation */
float angle = 0.0f;
float angleStep = PI / 5000.0f;


/* 
 * +--------------------------------------------+
 * | Quad variables                             |
 * +--------------------------------------------+
 */

/* The quad vertices */
float quadVertices[] = 
{
	-1.0, -1.0, -0.1,
	 1.0, -1.0, -0.1,
	 1.0,  1.0, -0.1,
	-1.0,  1.0, -0.1
};

/* The coordinates for the quad */
float quadCoords[] = 
{
	0.0, 0.0,
	1.0, 0.0,
	1.0, 1.0,
	0.0, 1.0
};

GLint quadDataLocation;
GLint quadDepthLocation;
GLuint textureIndexLocation;
int textureIndex = 0;

GLfloat mouseXLocation;
GLfloat mouseYLocation;
float mouseX;
float mouseY;

/* The Vertex Buffer Objects and the Vertex Array Object for the quad */
GLuint quadVerticesVbo;
GLuint quadCoordsVbo;
GLuint quadVao;


/* 
 * +--------------------------------------------+
 * | Framebuffer information                     |
 * +--------------------------------------------+
 */

GLuint frameBuffer;
GLuint texColorBuffer;
GLuint rboDepthStencil;

/* 
 * +-------------------------------------------------+
 * | Vertex and Fragment shader for post production   |
 * +-------------------------------------------------+
 */

/* The source code of the vertex and fragment shaders */
std::string postVertexShaderCode;
std::string postFragmentShaderCode;

/* References for the vertex and fragment shaders and the shader program */
GLuint postVertexShader = 0;
GLuint postFragmentShader = 0;
GLuint postShaderProgram = 0;


/**
 *	Reads the indicated file and returns a string array with the content
 */
int loadFile(char* filename, std::string& text)
{
	std::ifstream ifs;
	ifs.open(filename, std::ios::in);

	std::string line;
	while (ifs.good()) {
        getline(ifs, line);
		text += line + "\n";
    }
	return 0;
}


/**
*    Function invoked for drawing using OpenGL
*/
void display()
{
	/* Bind the buffer where the initial image (color and depth) will be stored */
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	/* Clear the window */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

    /* Set the shader program in the pipeline */
	glUseProgram(shaderProgram);
	glBindVertexArray(modelVao);
	
	/* Set the view matrix */
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0));
	viewMatrix = glm::rotate(translation, angle, glm::vec3(0.0f, 1.0f, 0.0f));

	/* Set the model matrix */
	//modelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(1.0));

	/* Set the projection matrix */
	projMatrix = glm::perspective(glm::radians(45.0f), windowWidth / windowHeight, 0.1f, 100.0f);
	
	/* Send matrix to shader */
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(projMatrixLocation, 1, GL_FALSE, glm::value_ptr(projMatrix));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, crateTexture);

	/* Draw the 3D model */
	glDrawArrays(GL_TRIANGLES, 0, modelVertexCount);


	/* 
	 * +--------------------------------------------+
	 * | Load image on buffer for post production    |
	 * +--------------------------------------------+
	 */

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindVertexArray(quadVao);
	glDisable(GL_DEPTH_TEST);
	glUseProgram(postShaderProgram);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, rboDepthStencil);
	const GLint samplers[2] = { 0, 1 }; // we've bound our textures in textures 0 and 1.
	glUniform1iv(quadDataLocation, 2, samplers);
	glUniform1i(textureIndexLocation, textureIndex);
	glUniform1f(mouseXLocation, mouseX);
	glUniform1f(mouseYLocation, mouseY);
	glDrawArrays(GL_POLYGON, 0, 4);

	/* Update rotation for next frame */
	angle += angleStep;

    /* Force execution of OpenGL commands */
    glFlush();

    /* Swap buffers for animation */
    glutSwapBuffers();
}


/**
*    Function invoked when window system events are not being received
*/
void idle()
{
    /* Redraw the window */
    glutPostRedisplay();
}


/**
*    Function invoked when an event on a regular keys occur
*/
void keyboard(unsigned char k, int x, int y)
{
	/* Close application if ESC is pressed */
    if (k == 27)
    {
        /* Delete shaders, shader program and the vertex buffer */
		glDeleteProgram(shaderProgram);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		/* Close the program */
        exit(0);
	}
	else {
		textureIndex = (int)k;
	}
}

void myMouseFunc(int x, int y) {
	mouseX = (float)x;
	mouseY = (float)(600-y);
}


/**
*    Set OpenGL initial state
*/
void init()
{
    /* Set initial OpenGL states */
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);

	/* Initialize the vertex shader (generate, load, compile and check errors) */
	loadFile("vertex.glsl", vertexShaderCode);
	const char* vertexSource = vertexShaderCode.c_str();
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	GLint status = 0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	if(status != GL_TRUE) 
	{
		char buffer[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
		std::cout << "Error while compiling the vertex shader: " << std::endl << buffer << std::endl;
	}

	/* Initialize the fragment shader (generate, load, compile and check errors) */
	loadFile("fragment.glsl", fragmentShaderCode);
	const char* fragmentSource = fragmentShaderCode.c_str();
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	status = 0;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
	if(status != GL_TRUE) 
	{
		char buffer[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, buffer);
		std::cout << "Error while compiling the fragment shader: " << std::endl << buffer << std::endl;
	}

	/* Load the information of the 3D model */
	load_obj_file(modelFilename, modelVertices, modelCoords, modelNormals, modelVertexCount);

	/* Initialize the Vertex Buffer Object for the vertices */
	glGenBuffers(1, &verticesVbo);
	glBindBuffer(GL_ARRAY_BUFFER, verticesVbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * modelVertexCount * sizeof(GLfloat), modelVertices, GL_STATIC_DRAW);

	/* Initialize the Vertex Buffer Object for the texture coordinates */
	glGenBuffers(1, &coordsVbo);
	glBindBuffer(GL_ARRAY_BUFFER, coordsVbo);
	glBufferData(GL_ARRAY_BUFFER, 2 * modelVertexCount * sizeof(GLfloat), modelCoords, GL_STATIC_DRAW);

	/* Define the Vertex Array Object of the 3D model */
	glGenVertexArrays(1, &modelVao);
	glBindVertexArray(modelVao);
	glBindBuffer(GL_ARRAY_BUFFER, verticesVbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, coordsVbo);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	/* Set the texture of the model */
	textureData = stbi_load(textureFile, &textureWidth, &textureHeight, &textureComp, STBI_rgb);

	glGenTextures(1, &crateTexture);
	glBindTexture(GL_TEXTURE_2D, crateTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glActiveTexture(GL_TEXTURE0);

	/* Initialize the shader program */
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindAttribLocation(shaderProgram, 0, "inPoint");
	glBindAttribLocation(shaderProgram, 1, "inCoords");
	glLinkProgram(shaderProgram);

	/* Get the location of the uniform variables */
	modelMatrixLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
	viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
	projMatrixLocation = glGetUniformLocation(shaderProgram, "projMatrix");
	textureDataLocation = glGetUniformLocation(shaderProgram, "textureData");
	
	/* Set the shader program in the pipeline */
	glUseProgram(0);

	/* 
	 * +--------------------------------------------+
	 * | Define the shader program for the quad     |
	 * +--------------------------------------------+
	 */

	/* Initialize the quad vertex shader */
	loadFile("postVertexShader.glsl", postVertexShaderCode);
	const char* postVertexSource = postVertexShaderCode.c_str();
	postVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(postVertexShader, 1, &postVertexSource, NULL);
	glCompileShader(postVertexShader);
	status = 0;
	glGetShaderiv(postVertexShader, GL_COMPILE_STATUS, &status);
	if(status != GL_TRUE) 
	{
		char buffer[512];
		glGetShaderInfoLog(postVertexShader, 512, NULL, buffer);
		std::cout << "Error while compiling the quad vertex shader: " << std::endl << buffer << std::endl;
	}

	/* Initialize the quad fragment shader */
	loadFile("postFragmentShader.glsl", postFragmentShaderCode);
	const char* postFragmentSource = postFragmentShaderCode.c_str();
	postFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(postFragmentShader, 1, &postFragmentSource, NULL);
	glCompileShader(postFragmentShader);
	status = 0;
	glGetShaderiv(postFragmentShader, GL_COMPILE_STATUS, &status);
	if(status != GL_TRUE) 
	{
		char buffer[512];
		glGetShaderInfoLog(postFragmentShader, 512, NULL, buffer);
		std::cout << "Error while compiling the quad fragment shader: " << std::endl << buffer << std::endl;
	}

	/* Initialize the Vertex Buffer Object for the quad's vertices */
	glGenBuffers(1, &quadVerticesVbo);
	glBindBuffer(GL_ARRAY_BUFFER, quadVerticesVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	/* Initialize the Vertex Buffer Object for the quad's texture coords */
	glGenBuffers(1, &quadCoordsVbo);
	glBindBuffer(GL_ARRAY_BUFFER, quadCoordsVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadCoords), quadCoords, GL_STATIC_DRAW);

	/* Define the Vertex Array Object for the quad */
	glGenVertexArrays(1, &quadVao);
	glBindVertexArray(quadVao);
	glBindBuffer(GL_ARRAY_BUFFER, quadVerticesVbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, quadCoordsVbo);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	/* Initialize the quad shader program */
	postShaderProgram = glCreateProgram();
	glAttachShader(postShaderProgram, postVertexShader);
	glAttachShader(postShaderProgram, postFragmentShader);
	glBindAttribLocation(postShaderProgram, 0, "inPoint");
	glBindAttribLocation(postShaderProgram, 1, "inCoord");
	glLinkProgram(postShaderProgram);

	/* Get the location of the uniform variables */
	quadDataLocation = glGetUniformLocation(postShaderProgram, "textureData");
	textureIndexLocation = glGetUniformLocation(postShaderProgram, "textureIndex");
	mouseXLocation = glGetUniformLocation(postShaderProgram, "mouseX");
	mouseYLocation = glGetUniformLocation(postShaderProgram, "mouseY");

	/* Generate the frame buffer and the texture color buffer */
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

	/*glGenRenderbuffers(1, &rboDepthStencil);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencil);*/

	glGenTextures(1, &rboDepthStencil);
	glBindTexture(GL_TEXTURE_2D, rboDepthStencil);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, rboDepthStencil, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
}


/**
*    Main function
*/
int main(int argc, char **argv)
{
    /* Initialize the GLUT window */
    glutInit(&argc, argv);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(30, 30);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("OpenGL/FreeGLUT - Example: Rendering a textured .obj model using shaders");

	/* Init GLEW */
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
	}
	std::cout << "GLEW version: " << glewGetString(GLEW_VERSION) << std::endl;

    /* Set OpenGL initial state */
    init();

    /* Callback functions */
    glutDisplayFunc(display);
    glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(myMouseFunc);

    /* Start the main GLUT loop */
    /* NOTE: No code runs after this */
    glutMainLoop();
}
