#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 1) in vec3 normal;


out vec3 tex_cords;
out mat4 out_P;
out mat4 out_V;

uniform mat4 uni_P;
uniform mat4 uni_V;

void main()
{

	tex_cords = position;
	vec4 cam_space = uni_V * vec4(position,0.0);
	cam_space.w = 1.0;
	gl_Position =uni_P *cam_space;

}