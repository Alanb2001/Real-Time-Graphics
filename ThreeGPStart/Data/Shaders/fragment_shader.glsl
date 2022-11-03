#version 330

uniform sampler2D sampler_tex;
uniform vec3 camera_position;

in vec3 varying_position; 
in vec3 varying_normal;
in vec2 varying_coord;

out vec4 fragment_colour;

void main(void)
{
    vec3 ambient_colour = texture(sampler_tex, varying_coord).rgb;
    fragment_colour = vec4(ambient_colour * 0.1, 1.0);
}