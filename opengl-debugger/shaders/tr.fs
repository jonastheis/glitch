#version 300 es
out uvec4 FragColor;

uniform usampler2D dataTexture;
uniform uvec2 rnd;

void main()
{
    // fetch value at position of rnd and set for every fragment
    // FragColor = texelFetch(dataTexture, ivec2(rnd), 0);
    FragColor = texelFetch(dataTexture, ivec2(0,0), 0);
}