#version 330 core

out vec4 FragColor;

in VS_OUT
{
	vec2 TexCoords;
	vec3 FragPos;
	vec3 Normal;
	
}fs_in;

uniform sampler2D floorTexture;
uniform  vec3 viewPos;
uniform	 vec3 lightPos;
uniform  bool blinn;



void main()
{    
	vec3 color = texture(floorTexture, fs_in.TexCoords).rgb;
	//ambient
	vec3 ambient = 0.5 * color;
	//diffuse
	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	vec3 normal  = normalize(fs_in.Normal);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * color;

	//specular
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = 0.0f;
	if(blinn)
	{
		vec3 halfwayDir = normalize(lightDir + viewDir);
		spec = pow(max(dot(normal, halfwayDir), 0.0), 0.5f);
	}
	else
	{
		vec3 reflectDir = reflect(-lightDir, normal);
		spec = pow(max(dot(viewDir, reflectDir), 0.0), 0.5f);
	}
	
	vec3 specular = vec3(0.3) * spec;
	FragColor = vec4(ambient + diffuse + specular, 1.0f);
}

