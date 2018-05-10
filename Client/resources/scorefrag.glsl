#version 330 core 
out vec4 color;
in vec3 pos;
in vec2 fragTex;


uniform sampler2D tex;
uniform vec2 offset;
uniform float animate;

void main()
{
	if(animate == 0)
		color = texture(tex, fragTex);
	else{
		color = texture(tex, vec2(fragTex.x/3+offset.x,fragTex.y/3+offset.y));
	}
	//color.a= (color.r + color.g + color.b)/3.0;
	//color=vec4(1,0,0,1);
	
}