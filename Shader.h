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
	Shader(const GLchar* vertexSourcePath, const GLchar* fragmentSourcePath)
	{
		std::string vertexCode;
		std::string fragmentCode;

		try
		{
			std::ifstream vShaderFile(vertexSourcePath);
			std::ifstream fShaderFile(fragmentSourcePath);

			std::stringstream vShaderStream, fShaderStream;

			//��ȡ�ļ�����
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();

			vShaderFile.close();
			fShaderFile.close();

			//��ת��GLchar����
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "error read shader file" << std::endl;
		}
		

		const GLchar* vShaderCode = vertexCode.c_str();
		const GLchar* fShaderCode = fragmentCode.c_str();
		//
		GLuint vertex, fragment;
		GLint success;
		GLchar infoLog[512];

		//vertex shader 
		vertex = glCreateShader(GL_VERTEX_SHADER);
		/**
			\brief ��ɫ������
			\brief ָ����Դ�����ж��ٸ��ַ���
			\brief ������ɫ��Դ��
			\brief NULL
		*/
		glShaderSource(vertex,1, &vShaderCode, NULL);
		glCompileShader(vertex);

		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << infoLog << std::endl;
		}

		//fragment shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << infoLog << std::endl;
		}

		//��ɫ������
		this->Program = glCreateProgram();
		glAttachShader(this->Program, vertex);
		glAttachShader(this->Program, fragment);
		glLinkProgram(this->Program);

		glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
			std::cout << infoLog << std::endl;
		}

		//ɾ����ɫ��
		glDeleteShader(vertex);
		glDeleteShader(fragment);

	}
	void Use()
	{
		glUseProgram(this->Program);
	}
};
#endif//_SHADER_H_
