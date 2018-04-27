#version 330 core 
out vec4 color;
in vec3 pos;
in vec2 fragTex;


uniform sampler2D tex;
uniform vec2 offset;


void main()
{
	
	color = texture(tex, fragTex);
	//color.a= (color.r + color.g + color.b)/3.0;
	//color=vec4(1,0,0,1);
	color.a=1;
}
