#version 330

uniform mat4 combined_xform;
uniform mat4 model_xform;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_texturecoord;

vec3 light_position;
vec3 intensity;

out vec3 varying_position; 
out vec3 varying_normal;
out vec2 varying_coord;
out float varying_gouraud;

void main(void)
{
	light_position.y = 1000;

	varying_position = mat4x3(model_xform) * vec4(vertex_position, 1.0);
	varying_coord = vertex_texturecoord;
	varying_normal = mat3(model_xform) * vertex_normal;
	
	vec3 P = varying_position;
	vec3 N = varying_normal;
	
	vec3 light_direction = light_position - P;
	vec3 L = normalize(light_direction);
	
	float intensity = max(0, dot(L, N));
	
	varying_gouraud = intensity;

	gl_Position = combined_xform * model_xform * vec4(vertex_position, 1.0);
}