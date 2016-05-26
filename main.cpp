
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
#include <map>
#include <random>
//glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//assimp
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

//freetype
#include <ft2build.h>
#include FT_FREETYPE_H

//#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

const GLuint SCR_WIDTH = 800;
const GLuint SCR_HEIGHT = 600;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();
GLuint loadTexture(GLchar* path, GLboolean alpha = false);
GLuint loadCubemap(std::vector<const GLchar*> faces);
void calcFPS(GLFWwindow* window, GLfloat lastframe);
GLuint generateMultiSampleTexture(GLuint samples);
void RenderScene(Shader& shader);
void RenderQuad();
void RenderCube();
void DisplayFramebufferTexture(GLuint textureID);
bool notInitialized = false;
GLuint generateAttachmentTexture(GLboolean depth, GLboolean stencils);
//------------------------------------------------------------
bool keys[1024];
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;


GLfloat lastX = SCR_WIDTH / 2.0f;
GLfloat lastY = SCR_HEIGHT / 2.0f;


//camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
GLboolean gammaEnabled = false;


// Options
GLuint draw_mode = 1;

GLfloat lerp(GLfloat a, GLfloat b, GLfloat f)
{
	return a + f * (b - a);
}

GLboolean bloom = true;
GLfloat exposure = 1.0f;

struct Character
{
	GLuint TextureID;
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	GLuint Advance;
};

std::map<GLchar, Character> Characters;
GLuint VAO, VBO;
void RenderText(Shader &shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

int main(int argc, char* argv[])
{
	
	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr); // Windowed
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	glDepthFunc(GL_LESS);

	//
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Setup and compile our shaders
	Shader shaderGeometryPass("ssao_geometry.vs", "ssao_geometry.frag");
	Shader shaderLightingPass("ssao.vs", "ssao.frag");
	Shader shaderSSAO("ssao.vs", "ssao.frag");
	Shader shaderSSAOBlur("ssao.vs", "ssao_blur.frag");
	
	// set samplers
	shaderLightingPass.Use();
	glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "gPoitionDepth"), 0);
	glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "gNormal"), 1);
	glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "gAlbedo"), 2);
	glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "ssao"), 3);
	shaderSSAO.Use();
	glUniform1i(glGetUniformLocation(shaderSSAO.Program, "gPositionDepth"), 0);
	glUniform1i(glGetUniformLocation(shaderSSAO.Program, "gNormal"), 1);
	glUniform1i(glGetUniformLocation(shaderSSAO.Program, "texNoise"), 2);

	//model object
	Model nanosuit("./nanosuit/nanosuit.obj");

	glm::vec3 lightPos = glm::vec3(2.0f, 4.0f, -2.0f);
	glm::vec3 lightColor = glm::vec3(0.2, 0.2, 0.7);

	//set G-Buffer
	//3 texture:
	//1. Position + depth (RGBA)
	//2. Color (RGB)
	//3. Normal (RGB)
	GLuint gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	GLuint gPositionDepth, gNormal, gAlbedo;
	//position + linear depth color buffer
	glGenTextures(1, &gPositionDepth);
	glBindTexture(GL_TEXTURE_2D, gPositionDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPositionDepth, 0);
	//normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gNormal, 0);
	//gAlbedo color buffer
	glGenTextures(1, &gAlbedo);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gAlbedo, 0);

	GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);
	// - Create and attach depth buffer (renderbuffer)
	GLuint rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// - Finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "GBuffer Framebuffer not complete!" << std::endl;

	// Also create framebuffer to hold SSAO processing stage 
	GLuint ssaoFBO, ssaoBlurFBO;
	glGenFramebuffers(1, &ssaoFBO);  glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	GLuint ssaoColorBuffer, ssaoColorBufferBlur;
	// - SSAO color buffer
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Framebuffer not complete!" << std::endl;
	// - and blur stage
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glGenTextures(1, &ssaoColorBufferBlur);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	std::uniform_real_distribution<GLfloat> randomFloats(0.0f, 1.0f);
	std::default_random_engine generator;
	std::vector<glm::vec3> ssaoKernel;
	for (GLuint i = 0; i < 64; ++i)
	{
		glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		GLfloat scale = GLfloat(i) / 64.0;

		// Scale samples s.t. they're more aligned to center of kernel
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	// Noise texture
	std::vector<glm::vec3> ssaoNoise;
	for (GLuint i = 0; i < 16; i++)
	{
		glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
		ssaoNoise.push_back(noise);
	}
	GLuint noiseTexture; glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

#pragma region "object_initialization"
	// Set the object data (buffers, vertex attributes)
	GLfloat cubeVertices[] = {
		// Positions          // Texture Coords
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
		-0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

		-0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
		-0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

		0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
		0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
		-0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f
	};
	GLfloat floorVertices[] = {
		// Positions          // Texture Coords (note we set these higher than 1 that together with GL_REPEAT as texture wrapping mode will cause the floor texture to repeat)
		5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
		-5.0f, -0.5f, 5.0f, 0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f, 0.0f, 2.0f,

		5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
		-5.0f, -0.5f, -5.0f, 0.0f, 2.0f,
		5.0f, -0.5f, -5.0f, 2.0f, 2.0f
	};
	GLfloat quadVertices[] = {   // Vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// Positions   // TexCoords
		-1.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,

		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	while (!glfwWindowShouldClose(window))
	{
		//检查及调用事件
		glfwPollEvents();		
		do_movement();			

		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		calcFPS(window, lastFrame);

		// 1. Geometry Pass: render scene's geometry/color data into gbuffer
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 50.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model;
		shaderGeometryPass.Use();
		glUniformMatrix4fv(glGetUniformLocation(shaderGeometryPass.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shaderGeometryPass.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		// Floor cube
		model = glm::translate(model, glm::vec3(0.0, -1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(20.0f, 1.0f, 20.0f));
		glUniformMatrix4fv(glGetUniformLocation(shaderGeometryPass.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		RenderCube();
		// Nanosuit model on the floor
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 5.0));
		model = glm::rotate(model, -90.0f, glm::vec3(1.0, 0.0, 0.0));
		model = glm::scale(model, glm::vec3(0.5f));
		glUniformMatrix4fv(glGetUniformLocation(shaderGeometryPass.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		nanosuit.Draw(shaderGeometryPass);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// 2. Create SSAO texture
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		glClear(GL_COLOR_BUFFER_BIT);
		shaderSSAO.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPositionDepth);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		// Send kernel + rotation 
		for (GLuint i = 0; i < 64; ++i)
			glUniform3fv(glGetUniformLocation(shaderSSAO.Program, ("samples[" + std::to_string(i) + "]").c_str()), 1, &ssaoKernel[i][0]);
		glUniformMatrix4fv(glGetUniformLocation(shaderSSAO.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		RenderQuad();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// 3. Blur SSAO texture to remove noise
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
		glClear(GL_COLOR_BUFFER_BIT);
		shaderSSAOBlur.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		RenderQuad();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// 4. Lighting Pass: traditional deferred Blinn-Phong lighting now with added screen-space ambient occlusion
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderLightingPass.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPositionDepth);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedo);
		glActiveTexture(GL_TEXTURE3); // Add extra SSAO texture to lighting pass
		glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
		// Also send light relevant uniforms
		glm::vec3 lightPosView = glm::vec3(camera.GetViewMatrix() * glm::vec4(lightPos, 1.0));
		glUniform3fv(glGetUniformLocation(shaderLightingPass.Program, "light.Position"), 1, &lightPosView[0]);
		glUniform3fv(glGetUniformLocation(shaderLightingPass.Program, "light.Color"), 1, &lightColor[0]);
		// Update attenuation parameters
		const GLfloat constant = 1.0; // Note that we don't send this to the shader, we assume it is always 1.0 (in our case)
		const GLfloat linear = 0.09;
		const GLfloat quadratic = 0.032;
		glUniform1f(glGetUniformLocation(shaderLightingPass.Program, "light.Linear"), linear);
		glUniform1f(glGetUniformLocation(shaderLightingPass.Program, "light.Quadratic"), quadratic);
		glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "draw_mode"), draw_mode);
		RenderQuad();

		// Swap the buffers
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}
GLuint generateAttachmentTexture(GLboolean depth, GLboolean stencil)
{
	// What enum to use?
	GLenum attachment_type;
	if (!depth && !stencil)
		attachment_type = GL_RGB;
	else if (depth && !stencil)
		attachment_type = GL_DEPTH_COMPONENT;
	else if (!depth && stencil)
		attachment_type = GL_STENCIL_INDEX;

	//Generate texture ID and load texture data 
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	if (!depth && !stencil)
		glTexImage2D(GL_TEXTURE_2D, 0, attachment_type, SCR_WIDTH, SCR_HEIGHT, 0, attachment_type, GL_UNSIGNED_BYTE, NULL);
	else // Using both a stencil and depth test, needs special format arguments
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;

}
void RenderText(Shader &shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
	// Activate corresponding render state	
	shader.Use();
	glUniform3f(glGetUniformLocation(shader.Program, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	// Iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos, ypos + h, 0.0, 0.0 },
			{ xpos, ypos, 0.0, 1.0 },
			{ xpos + w, ypos, 1.0, 1.0 },

			{ xpos, ypos + h, 0.0, 0.0 },
			{ xpos + w, ypos, 1.0, 1.0 },
			{ xpos + w, ypos + h, 1.0, 0.0 }
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderScene(Shader &shader)
{
	// Room cube
	glm::mat4 model;
	model = glm::scale(model, glm::vec3(10.0));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glDisable(GL_CULL_FACE); // Note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.
	glUniform1i(glGetUniformLocation(shader.Program, "reverse_normals"), 1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
	RenderCube();
	glUniform1i(glGetUniformLocation(shader.Program, "reverse_normals"), 0); // And of course disable it
	glEnable(GL_CULL_FACE);

	// Cubes
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(4.0f, -3.5f, 0.0));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	RenderCube();
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(2.0f, 3.0f, 1.0));
	model = glm::scale(model, glm::vec3(1.5));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	RenderCube();
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-3.0f, -1.0f, 0.0));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	RenderCube();
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-1.5f, 1.0f, 1.5));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	RenderCube();
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-1.5f, 2.0f, -3.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(1.5));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	RenderCube();
}

// RenderQuad() Renders a 1x1 quad in NDC, best used for framebuffer color targets
// and post-processing effects.
GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
	if (quadVAO == 0)
	{
		GLfloat quadVertices[] = {
			// Positions        // Texture Coords
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// Setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// void RenderQuad()
// {
// 	if (quadVAO == 0)
// 	{
// 		GLfloat quadVertices[] = {
// 			// Positions       // Normals          // Texture Coords 
// 			-1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
// 			-1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
// 			 1.0f, 0.0f, -1.0f,  0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
// 			1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0, 1.0f, 0.0f,
// 		};
// 		// Setup plane VAO
// 		glGenVertexArrays(1, &quadVAO);
// 		glGenBuffers(1, &quadVBO);
// 		glBindVertexArray(quadVAO);
// 		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
// 		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
// 		glEnableVertexAttribArray(0);
// 		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
// 		glEnableVertexAttribArray(1);
// 		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
// 		glEnableVertexAttribArray(2);
// 		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
// 
// 	}
// 	glBindVertexArray(quadVAO);
// 	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
// 	glBindVertexArray(0);
// }

// RenderCube() Renders a 1x1 3D cube in NDC.
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;
void RenderCube()
{
	// Initialize (if necessary)
	if (cubeVAO == 0)
	{
		GLfloat vertices[] = {
			// Back face
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
			0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,  // top-right
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // bottom-left
			-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,// top-left
			// Front face
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
			0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
			0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // top-right
			0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
			-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom-left
			// Left face
			-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
			-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
			-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-left
			-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
			-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
			-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
			// Right face
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
			0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
			0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right         
			0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-right
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // top-left
			0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left     
			// Bottom face
			-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
			0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
			0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,// bottom-left
			0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
			-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
			-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
			// Top face
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
			0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
			0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right     
			0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
			-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// Fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// Link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// Render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

GLuint generateMultiSampleTexture(GLuint samples)
{
	GLuint texture;
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	return texture;
}

GLuint loadCubemap(std::vector<const GLchar*> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height;
	unsigned char* image;
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
	
}

GLuint loadTexture(GLchar* path, GLboolean alpha)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, alpha ? GL_RGBA : GL_RGB, width, height, 0, alpha ? GL_RGBA :GL_RGB, GL_UNSIGNED_BYTE, image);
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

	if (key == GLFW_KEY_B)
	{
		if (action == GLFW_PRESS)
		{
			gammaEnabled = gammaEnabled ? false : true;
		} 

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

	if (keys[GLFW_KEY_1])
		draw_mode = 1;
	if (keys[GLFW_KEY_2])
		draw_mode = 2;
	if (keys[GLFW_KEY_3])
		draw_mode = 3;
	if (keys[GLFW_KEY_4])
		draw_mode = 4;
	if (keys[GLFW_KEY_5])
		draw_mode = 5;
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

void calcFPS(GLFWwindow* window, GLfloat lastframe)
{
// 	GLfloat frame = 1000.0f / lastframe;
// 	stringstream ss;
// 	string title;
// 	ss << frame;
// 
// 	title = ss.str();
// 	glfwSetWindowTitle(window, title.c_str());

	static string temp = "";
	static GLfloat framesPerSecond = 0.0f;
	static GLfloat frames = 0.0f;
	static GLfloat lastTime = 0.0f;
	GLfloat currentTime = glfwGetTime();
	++frames;
	if (currentTime - lastTime > 1.0f)
	{
		framesPerSecond = frames;
		temp = "fps: " + std::to_string(framesPerSecond);
		lastTime = currentTime;
		frames = 0;
	}
	glfwSetWindowTitle(window, temp.c_str());
}