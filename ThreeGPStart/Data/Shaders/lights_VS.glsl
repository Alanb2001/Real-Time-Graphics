#version 330

layout(std140) uniform PerModelUniforms
{
	mat4 projection_xform;
	mat4 model_xform;
	mat4 view_xform;
	vec3 material_colour;
	float pad1_;
};

in vec3 vertex_position;
in vec3 vertex_normal;

out vec3 colour;
out vec3 normal;
out vec3 position;

void main(void)
{
	colour = material_colour;
	normal = vec3(model_xform * vec4(vertex_normal, 0));
	position = vec3(model_xform * vec4(vertex_position, 1.0));

	mat4 combined_xform = projection_xform * view_xform * model_xform;
	gl_Position = combined_xform * vec4(vertex_position, 1.0);
}