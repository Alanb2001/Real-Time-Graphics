#version 330

//layout(std140) uniform PerFrameUniforms
//{
//	vec3 light_position;
//	float light_range;
//	vec3 light_direction;
//	vec3 cam_pos;
//	vec3 ambeint_light;
//};
//
//out vec4 fragment_colour;
//
//in vec3 position;
//
//void main(void)
//{
//	fragment_colour = vec4(ambeint_light, 1.0);
//}

uniform sampler2D sampler_tex;
uniform vec3 camera_position;

in vec3 varying_position; 
in vec3 varying_normal;
in vec2 varying_coord;

out vec4 fragment_colour;

struct lights 
{
	vec3 light_position;
	vec3 light_colour;
};

const int numberOfLights = 2;
lights light[numberOfLights];

lights light0 = lights(
vec3(500, 500, 0),
vec3(1, 1, 1)
);

lights light1 = lights(
vec3(-400, 200, 0),
vec3(0, 1, 1)
);

void main(void)
{	
	light[0] = light0;
	light[1] = light1;

	for(int i = 0; i < numberOfLights; i++)
	{
		vec3 light_ambient_colour = vec3(0.5);
		float light_range = 3000;
		float material_shininess = 20;

		vec3 material_diffuse_colour = texture(sampler_tex, varying_coord).rgb;
		vec3 fragment_position = varying_position;
		vec3 fragment_normal = normalize(varying_normal);
	
		vec3 material_ambient_colour = material_diffuse_colour;
		vec3 material_specular_colour = material_diffuse_colour;
	
		vec3 light_direction = normalize(light[i].light_position - fragment_position);
		vec3 camera_direction = normalize(camera_position - fragment_position);

		float light_distance = length(light_direction);
		float attenuation = 1.0 - smoothstep(0, light_range, light_distance);
	
		float light_amount = max(0, dot(light_direction, fragment_normal));
		vec3 diffuse_colour = material_diffuse_colour * light_amount;
	
		vec3 rV = reflect(-light_direction, fragment_normal);
		float LR = max(0, dot(camera_direction, rV));
		vec3 specular_colour = material_specular_colour * pow(LR, material_shininess);
	
		vec3 ambient_colour = light_ambient_colour * material_ambient_colour;
	
		vec3 final_colour = ambient_colour + light[i].light_colour * (diffuse_colour + specular_colour) * attenuation;

		fragment_colour = vec4(light_amount * final_colour, 1.0);
	}
}