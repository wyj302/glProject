
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

	//深度测试
	glEnable(GL_DEPTH_TEST);

	//	
	Shader shader("model.vs", "model.frag");

	//light shader
	Shader lightShader("lamp.vs", "lamp.frag");

	//model
	Model ourModel("nanosuit/nanosuit.obj");

	//light
	Model lightModel("nanosuit/nanosuit.obj");

	//line
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glm::vec3 pointLightPositions[] =
	{
		glm::vec3(2.3f, -1.6f, -3.0f),
		glm::vec3(-1.7f, 0.9f, 1.0f)
	};

	while (!glfwWindowShouldClose(window))
	{
		//检查及调用事件
		glfwPollEvents();		
		do_movement();

		//
		glClearColor(0.2f, 0.2, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		shader.Use();		
		 
		glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		GLuint viewLoc = glGetUniformLocation(shader.Program, "view");
		GLuint projectionLoc = glGetUniformLocation(shader.Program, "projection");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// Set the lighting uniforms
		glUniform3f(glGetUniformLocation(shader.Program, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		// Point light 1
		glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
		glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].ambient"), 0.05f, 0.05f, 0.05f);
		glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].diffuse"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].specular"), 1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].linear"), 0.009);
		glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].quadratic"), 0.0032);
		// Point light 2
		glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
		glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].ambient"), 0.05f, 0.05f, 0.05f);
		glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].diffuse"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].specular"), 1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].linear"), 0.009);
		glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].quadratic"), 0.0032);	
		
		//draw model
		glm::mat4 model;
		GLuint modelLoc = glGetUniformLocation(shader.Program, "model");		

		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));		
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		ourModel.Draw(shader);				

		//draw light
		lightShader.Use();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		for (size_t i = 0; i < 2; i++)
		{
			model = glm::mat4();
			model = translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
			glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

			lightModel.Draw(lightShader);
		}

		//交换缓冲
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

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