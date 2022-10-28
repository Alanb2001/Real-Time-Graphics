#version 330

layout(std140) uniform PerFrameUniforms
{
	vec3 light_position;
	float light_range;
	vec3 light_direction;
	vec3 cam_pos;
	vec3 ambeint_light;
};

out vec4 fragment_colour;

in vec3 position;

void main(void)
{
	fragment_colour = vec4(ambeint_light, 1.0);
}