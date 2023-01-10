#version 330

in vec4 FragPos;

uniform vec3 lightPos;
uniform float farPlane;

void main(void)
{
	gl_FragDepth = length(FragPos.xyz - lightPos) / farPlane;
}