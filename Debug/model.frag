#version 330 core


struct Material
{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;    
};


struct PointLight
{
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

};


#define NR_POINT_LIGHTS 2

in vec3 fragPosition;
in vec3 Normal;
in vec2 TexCoords;

uniform  PointLight pointLights[NR_POINT_LIGHTS];
uniform  Material material;
uniform  vec3 viewPos;

out vec4 color;

vec3 CalcPointLight(PointLight light, Material mat,vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{    
    vec3 result;
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 norm = normalize(Normal);

    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], material, norm, fragPosition, viewDir);

    color = vec4(result, 1.0f);
}

//calculates color when use a light point
vec3 CalcPointLight(PointLight light, Material material, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    //diffuse
    float diff = max(dot(normal, lightDir), 0.0);

    //specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);

}