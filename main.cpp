
//glew
#define		GLEW_STATIC
#include <GL/glew.h>
#include <cmath>

//glfw
#include <GLFW/glfw3.h>
#include <iostream>
#include <soil.h>
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

//glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//assimp
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

//#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

const GLuint screenWidth = 800;
const GLuint screenHeight = 600;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();
GLuint loadTexture(GLchar* path);
//------------------------------------------------------------

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
bool keys[1024];
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

bool firstMoust = true;
GLfloat lastX = screenWidth / 2.0f;
GLfloat lastY = screenHeight / 2.0f;

GLfloat pitch = 0.0f;
GLfloat yaw = 90.0f;
GLfloat aspect = 45.0f;

//camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

int main(int argc, char* argv[])
{
	//git diff test
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	//callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW Window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	//
	glViewport(0, 0, screenWidth, screenHeight);

	//面剔除
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CCW); //GL_CCW：逆时针 GL_CW：顺时针

	//深度测试
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//模板测试
	glEnable(GL_STENCIL_TEST);	
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	//	
	Shader shader("depth.vs", "depth.frag");
	Shader singleColor("depth.vs", "ShaderSingleColor.frag");
	
	#pragma region "object_initialization"

	//line
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	GLfloat cubeVertices[] = {
		// Back face
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // Bottom-left
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, // bottom-right         
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f, // top-right
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // bottom-left
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left
		// Front face
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f, // bottom-right
		0.5f, 0.5f, 0.5f, 1.0f, 1.0f, // top-right
		0.5f, 0.5f, 0.5f, 1.0f, 1.0f, // top-right
		-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, // top-left
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left
		// Left face
		-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, // top-right
		-0.5f, 0.5f, -0.5f, 1.0f, 1.0f, // top-left
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-right
		-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, // top-right
		// Right face
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, // top-left
		0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-right
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f, // top-right         
		0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-right
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, // top-left
		0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left     
		// Bottom face
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f, 1.0f, 1.0f, // top-left
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f, // bottom-left
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f, // bottom-left
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-right
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // top-right
		// Top face
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, // bottom-right
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f, // top-right     
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, // bottom-right
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left
		-0.5f, 0.5f, 0.5f, 0.0f, 0.0f  // bottom-left        
	};
	GLfloat planeVertices[] = {
		// Positions            // Texture Coords (note we set these higher than 1 that together with GL_REPEAT as texture wrapping mode will cause the floor texture to repeat)
		5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
		-5.0f, -0.5f, 5.0f, 0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f, 0.0f, 2.0f,

		5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
		-5.0f, -0.5f, -5.0f, 0.0f, 2.0f,
		5.0f, -0.5f, -5.0f, 2.0f, 2.0f
	};

	GLuint cubeTexture = loadTexture("marble.jpg");
	GLuint planeTexture = loadTexture("metal.png");

	//draw cube
	GLuint cubeVAO, cubeVBO;
	glGenBuffers(1, &cubeVBO);
	glGenVertexArrays(1, &cubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBindVertexArray(cubeVAO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

	glBindVertexArray(0);

	//draw plane
	GLuint planeVAO, planeVBO;
	glGenBuffers(1, &planeVBO);
	glGenVertexArrays(1, &planeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBindVertexArray(planeVAO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glBindVertexArray(0);

	#pragma endregion


	while (!glfwWindowShouldClose(window))
	{
		//检查及调用事件
		glfwPollEvents();		
		do_movement();

		//
		glClearColor(0.2f, 0.2, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//
		singleColor.Use();		
		glm::mat4 model;
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 100.0f);				
		glUniformMatrix4fv(glGetUniformLocation(singleColor.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(singleColor.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		shader.Use();
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glStencilMask(0x00);
		//draw plane
		glBindVertexArray(planeVAO);
		glBindTexture(GL_TEXTURE_2D, planeTexture);
		model = glm::mat4();
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);

		// draw cube
		glBindVertexArray(cubeVAO);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		model = glm::translate(model, glm::vec3(-1.0f, 0.05f, -1.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));		
		glDrawArrays(GL_TRIANGLES, 0, 36);

		model = glm::mat4();
		model = glm::translate(model, glm::vec3(2.0f, 0.05f, 0.0f)); // y 0.1 fix z-fighting
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);


		//绘制边框
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);
		singleColor.Use();
		GLfloat scale = 1.1;

		glBindVertexArray(cubeVAO);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);

		model = glm::mat4();
		model = glm::translate(model, glm::vec3(-1.0f, 0.05f, -1.0f));
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		glUniformMatrix4fv(glGetUniformLocation(singleColor.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		model = glm::mat4();
		model = glm::translate(model, glm::vec3(2.0f, 0.05f, 0.0f)); // y 0.1 fix z-fighting
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		glUniformMatrix4fv(glGetUniformLocation(singleColor.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glStencilMask(0xFF);
		glEnable(GL_DEPTH_TEST);

		//交换缓冲
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

GLuint loadTexture(GLchar* path)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);


	//
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);

	return textureID;
}


#pragma region "User input"
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}		
	}
	
}
void do_movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}