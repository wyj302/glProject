#ifndef _SHADER_H_
#define _SHADER_H_
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;
#include <GL/glew.h>

class Shader
{
public:
	//id
	GLuint Program;
	Shader(const GLchar* vertexSourcePath, const GLchar* fragmentSourcePath, const GLchar* geometrySourcePath = nullptr)
	{
		std::string vertexCode;
		std::string fragmentCode;
		std::string geometryCode;

		try
		{
			std::ifstream vShaderFile(vertexSourcePath);
			std::ifstream fShaderFile(fragmentSourcePath);

			std::stringstream vShaderStream, fShaderStream;

			//读取文件到流
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();

			vShaderFile.close();
			fShaderFile.close();

			//流转换GLchar数组
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "error read shader file" << std::endl;
		}

		if (geometrySourcePath != nullptr)
		{
			std::ifstream gShaderFile(geometrySourcePath);
			std::stringstream  gShaderStream;
			gShaderStream << gShaderFile.rdbuf();
			gShaderFile.close();
			geometryCode = gShaderStream.str();
		}

		const GLchar* vShaderCode = vertexCode.c_str();
		const GLchar* fShaderCode = fragmentCode.c_str();
		const GLchar* gShaderCode;
		if (geometrySourcePath != nullptr)
		{
			gShaderCode = geometryCode.c_str();
		}

		GLuint vertex, fragment, geometry;

		//vertex shader 
		vertex = glCreateShader(GL_VERTEX_SHADER);
		/**
		\brief 着色器对象
		\brief 指定了源码中有多少个字符串
		\brief 顶点着色器源码
		\brief NULL
		*/
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX");

		//fragment shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		checkCompileErrors(vertex, "FRAGMENT");

		if (geometrySourcePath != nullptr)
		{
			//geometry shader
			geometry = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry, 1, &gShaderCode, NULL);
			glCompileShader(geometry);
			checkCompileErrors(vertex, "GEOMETRY");
		}

		//着色器程序
		this->Program = glCreateProgram();
		glAttachShader(this->Program, vertex);

		if (geometrySourcePath != nullptr)
		{
			glAttachShader(this->Program, geometry);
		}
		glAttachShader(this->Program, fragment);
		glLinkProgram(this->Program);

		checkCompileErrors(this->Program, "PROGRAM");

		//删除着色器
		glDeleteShader(vertex);
		glDeleteShader(fragment);

		if (geometrySourcePath != nullptr)
		{
			glDeleteShader(geometry);
		}

	}
	void Use()
	{
		glUseProgram(this->Program);
	}
private:
	void checkCompileErrors(GLuint shader, std::string type)
	{
		GLint success;
		GLchar infoLog[512];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

			if (!success)
			{
				glGetShaderInfoLog(shader, 512, NULL, infoLog);
				std::cout << infoLog << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 512, NULL, infoLog);
				std::cout << infoLog << std::endl;
			}
		}
	}

};
#endif//_SHADER_H_
