#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out VS_OUT
{
	out vec2 TexCoords;
	out vec3 FragPos;
	out vec3 Normal;
}vs_out;


uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view  * vec4(position, 1.0f);
    vs_out.FragPos = position;
    vs_out.TexCoords = texCoords;
    vs_out.Normal =  normal;
}
