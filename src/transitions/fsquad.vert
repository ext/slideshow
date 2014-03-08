#version 330 core

in vec2 in_pos;
out vec2 uv;

void main() {
  uv = in_pos.xy * vec2(0.5,-0.5) + vec2(0.5,0.5); // [-1,1] -> [0,1]
  gl_Position = vec4(in_pos.xy,0.0,1.0);
}
