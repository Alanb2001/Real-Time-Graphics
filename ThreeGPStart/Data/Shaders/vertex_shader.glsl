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

out vec3 position;

void main(void)
{
	position = vec3(model_xform * vec4(vertex_position, 1.0));
	
	mat4 combined_xform = projection_xform * view_xform * model_xform;
	gl_Position = combined_xform * vec4(vertex_position, 1.0);
}