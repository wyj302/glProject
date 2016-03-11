
//glew
#define		GLEW_STATIC
#include <GL/glew.h>

//glfw
#include <GLFW/glfw3.h>
#include <iostream>

#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

//shaders
const GLchar* vertexShaderSource = "#version 330 core \n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"out vec3 ourColor;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
"ourColor = color;\n"
"}\n\0";

const GLchar* fragmentShaderSource = "#version 330 core \n"
"in vec3 ourColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = vec4(ourColor, 1.0f);\n"
"}\n\0";

int main(int argc, char* argv[])
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	

	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);

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
	glViewport(0, 0, 800, 600);

	//vertex shader
	GLuint vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	//check error
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "Compilation vertex shader failed: " << infoLog <<std::endl;
	}

	//fragment shader
	GLuint fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "Compilation fragment shader failed: " << infoLog << std::endl;
	}

	//program
	GLuint shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	//check program
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "shader program failed: " << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//vertex 
	GLfloat vertices[] = 
	{
		// position		   //color
		0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // ���Ͻ�
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // ���½�
	   -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f // ���Ͻ�

// 		// �ڶ���������
// 		0.5f, -0.5f, 0.0f,  // ���½�
// 		-0.5f, -0.5f, 0.0f, // ���½�
// 		-0.5f, 0.5f, 0.0f   // ���Ͻ�
	};
// 	GLfloat vertices[] =
// 	{
// 		// ��һ��������
// 		0.5f, 0.5f, 0.0f,   // ���Ͻ�
// 		0.5f, -0.5f, 0.0f,  // ���½�
// 		-0.5f, -0.5f, 0.0f,  // ���½�
// 		-0.5f, 0.5f, 0.0f	// ����
// 	};
// 	GLuint indices[] =
// 	{
// 		0, 1, 3,
// 		1, 2, 3
// 	};

	//VBO
	GLuint VBO, VAO;
	//����ID
	glGenBuffers(1, &VBO);	
	glGenVertexArrays(1, &VAO);

	//��VAO
	glBindVertexArray(VAO);

	//�������󶨵� GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//�Ѷ������鸴�Ƶ��������ṩ��OpenGLʹ��
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//EBO
// 	GLuint EBO;
// 	glGenBuffers(1, &EBO);
// 
// 	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
// 	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//���ö�������
	/*
		\param ������һ����������,location = 0;
		\param �������Դ�С��vec3,3��
		\param ָ����������
		\param �����Ƿ񱻱�׼��
		\param ����
		\param λ�������ڻ�������ʼλ�õ�ƫ����
	*/
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//��ɫ����
	glVertexAttribPointer(1, 3,GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);	

	//���VAO
	glBindVertexArray(0);

	//����ģʽ
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	//
	while (!glfwWindowShouldClose(window))
	{
		//��鼰�����¼�
		glfwPollEvents();

		//��Ⱦ
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//������ɫ��
		glUseProgram(shaderProgram);
		
		//����uniform��ɫ
// 		GLfloat timeValue = glfwGetTime();
// 		GLfloat greenValue = (sin(timeValue / 2) + 0.5f);
//		GLint vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
//		glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

		//����
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		
		//��������
		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	//glDeleteBuffers(1, &EBO);

	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}