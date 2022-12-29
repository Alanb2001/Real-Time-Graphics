#version 330

out vec4 FragColour;
in vec2 texCoords;

uniform sampler2D screenTexture;

void main(void)
{
    FragColour = texture(screenTexture, texCoords);
}