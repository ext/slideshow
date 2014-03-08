#version 330
uniform sampler2D texture_0;
uniform sampler2D texture_1;
uniform float s;
in vec2 uv;
out vec4 ocolor;

vec2 screenspace_uv(sampler2D t){
  ivec2 size = textureSize(t, 0);
  return vec2(gl_FragCoord.x / size.x, gl_FragCoord.y / size.y);
}

void main(void){
  vec2 sp = screenspace_uv(texture_0);
  float y = smoothstep(0, 1, clamp(sp.y - 1 + s * 2, 0.0f, 1.0f));
  vec4 t0 = texture2D(texture_0, uv);
  vec4 t1 = texture2D(texture_1, uv);
  ocolor = mix(t0,t1,y);
}
