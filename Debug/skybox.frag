#version 330 core
in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;

out vec4 color;

uniform samplerCube skybox;

void main()
{
	vec3 I = normalize(Position - cameraPos);
	vec3 R = reflect(I, normalize(Normal));
	color = texture(skybox, R);
}
