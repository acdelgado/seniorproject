#version 330 core 
out vec4 color;
in vec3 pos;
in vec2 fragTex;


uniform sampler2D tex;
uniform vec2 offset;
uniform float animate;
uniform float diffColor;

void main()
{
	if(animate == 0)
		color = texture(tex, fragTex);
	else if(animate > 0){
		color = texture(tex, vec2((fragTex.x/3+offset.x),fragTex.y/3+offset.y));
	}
	else{
		color = texture(tex, vec2((fragTex.x/3-offset.x-(1/3.0f))*-1,fragTex.y/3+offset.y));
	}

	if(diffColor == 0)
	{
		color *= 1.3;
		if(color.a > 0.1)
			color.a = 0.8;
	}
	
}