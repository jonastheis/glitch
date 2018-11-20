#version 300 es
out uvec4 FragColor;

uniform usampler2D dataTexture;
uniform ivec2 rnd;

void main()
{
    // fetch value at position of rnd and set for every fragment
    FragColor = uvec4(texelFetch(dataTexture, rnd, 0));
}