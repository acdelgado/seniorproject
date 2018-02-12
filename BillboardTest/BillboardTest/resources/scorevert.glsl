#version  450 core
layout(location = 0) in vec4 vertPos;
layout(location = 2) in vec2 vertTex;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
out vec2 fragTex;
out vec3 pos;
uniform sampler2D tex;

void main()
{
gl_Position = P * V * M * vertPos;
fragTex = vertTex;	
}
