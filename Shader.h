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
		catch (std::ifstream::failure* e)
		{
			std::cout << "error read shader file" << std::endl;
		}
		
	}
	void Use();
};
#endif//_SHADER_H_
