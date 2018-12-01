#version 300 es
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0

uniform sampler2D HTex00;
uniform sampler2D HTex01;
uniform sampler2D HTex02;
uniform sampler2D HTex03;
uniform sampler2D ITex04;
uniform sampler2D ITex05;
uniform sampler2D ITex06;
uniform sampler2D ITex07;
uniform sampler2D ITex08;
// bind dummy texture last to prevent last texture error - do not use
uniform sampler2D DTex09;

void main()
{   
    float val; 
    val += texelFetch(HTex00, ivec2(0,0), 0).r ;
    val += texelFetch(HTex01, ivec2(0,0), 0).r ;
    val += texelFetch(HTex02, ivec2(0,0), 0).r ;
    val += texelFetch(HTex03, ivec2(0,0), 0).r ;
    val += texelFetch(ITex04, ivec2(0,0), 0).r ;
    val += texelFetch(ITex05, ivec2(0,0), 0).r ;
    val += texelFetch(ITex06, ivec2(0,0), 0).r ;
    val += texelFetch(ITex07, ivec2(0,0), 0).r ;
    val += texelFetch(ITex08, ivec2(0,0), 0).r ;
    
    val += texelFetch(HTex00, ivec2(5,0), 0).r ;

    // val += texelFetch(HTex09, ivec2(0,0), 0).r ;
    gl_Position = vec4(aPos, 1);
}