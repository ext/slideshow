#version 330 core

uniform sampler2D texture_0;
uniform sampler2D texture_1;
uniform float s;

in vec2 uv;
out vec4 ocolor;

void main(void){
	vec4 t0 = texture2D(texture_0, uv);
	vec4 t1 = texture2D(texture_1, uv);
	ocolor = mix(t0, t1, s);
}
