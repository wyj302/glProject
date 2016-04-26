#version 400 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT
{
	vec2 texCoords;
}gs_in[];

out vec2 TexCoords;

uniform float time;

//求三角面法线
vec3 GetNormal()
{
	vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
	vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
	return normalize(cross(a, b));
}

//顶点沿着法线平移
vec4 explode(vec4 position, vec3 normal)
{
	float magnitude = 2.0f;
    vec3 direction = normal * ((sin(time) + 1.0f) / 2.0f) * magnitude; 
    return position + vec4(direction, 0.0f);
}

void main()
{
	vec3 normal = GetNormal();
	gl_Position = explode(gl_in[0].gl_Position, normal);
	TexCoords = gs_in[0].texCoords;
	EmitVertex();

	gl_Position = explode(gl_in[1].gl_Position, normal);
	TexCoords = gs_in[1].texCoords;
	EmitVertex();

	gl_Position = explode(gl_in[2].gl_Position, normal);
	TexCoords = gs_in[2].texCoords;
	EmitVertex();

	EndPrimitive();

}