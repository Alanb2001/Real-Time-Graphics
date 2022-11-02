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

in vec3 colour;
in vec3 normal;
in vec3 position; 

void main(void)
{	
	vec3 Ka = colour;
	vec3 Kd = colour;
	vec3 Ks = vec3(1, 1, 1);
	vec3 Ia = ambeint_light;

	vec3 mycolour = (Ia * Ka);

	vec3 P = position;
	vec3 N = normalize(normal);
	vec3 V = (cam_pos - P);

	float atten = 1 - smoothstep(0, light_range, distance(P, light_position));
	vec3 Il = vec3(0.5, 0.5, 0.5) * atten;
	vec3 L = normalize(light_position - P);
	vec3 R = reflect(-L, N);
	float specular = 0;

	vec3 lambert_col = Il * (Kd * max (dot(N, L), 0.0) + Ks * specular);

	mycolour += lambert_col;

	fragment_colour = vec4(mycolour, 1.0);
}