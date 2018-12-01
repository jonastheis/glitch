#version 300 es
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0

uniform sampler2D HTex00;
uniform sampler2D HTex01;
uniform sampler2D HTex02;
uniform sampler2D HTex03;
uniform sampler2D HTex04;
uniform sampler2D HTex05;
uniform sampler2D HTex06;
uniform sampler2D HTex07;
uniform sampler2D HTex08;
uniform sampler2D HTex09;

void main()
{   
    float val; 
    val += texelFetch(HTex00, ivec2(0,0), 0).r ;
    val += texelFetch(HTex01, ivec2(0,0), 0).r ;
    val += texelFetch(HTex02, ivec2(0,0), 0).r ;
    val += texelFetch(HTex03, ivec2(0,0), 0).r ;
    val += texelFetch(HTex04, ivec2(0,0), 0).r ;
    val += texelFetch(HTex05, ivec2(0,0), 0).r ;
    val += texelFetch(HTex06, ivec2(0,0), 0).r ;
    val += texelFetch(HTex07, ivec2(0,0), 0).r ;
    val += texelFetch(HTex08, ivec2(0,0), 0).r ;
    
    val += texelFetch(HTex00, ivec2(5,0), 0).r ;

    // val += texelFetch(HTex09, ivec2(0,0), 0).r ;
    gl_Position = vec4(aPos, val);
}