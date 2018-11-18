#version 330 core
out uvec4 FragColor;

uniform usampler2D dataTexture;

void main()
{
    FragColor = uvec4(15, uvec3(1));
}