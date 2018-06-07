#version 330 core 
out vec4 color;
in vec3 pos;
in vec2 fragTex;


uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec2 offset;
uniform float wave;
uniform float flip;
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
	if(wave != 0)
	{
		vec2 displacement = texture(tex2, fragTex / 6.0).xy;
		float t = fragTex.y + displacement.y * 0.1+(sin(fragTex.x * 80.0+wave*2)*0.01);
		if(flip == 1)
			t = fragTex.y + displacement.y * 0.1+(sin(fragTex.x * 60.0-wave*2)*0.01);
			color = texture(tex,vec2(fragTex.x,t));
	}
}