#version 330 core
out uvec4 FragColor;

uniform usampler2D dataTexture;
uniform uvec2 rnd;

void main()
{
    FragColor = texelFetch(dataTexture, ivec2(rnd), 0);
}