#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 colour;
layout (location = 2) in float alpha;
layout (location = 3) in vec2 uv;
layout (location = 4) in vec3 normal;

out vec3 pass_colour;
out vec2 pass_uv;
out vec3 pass_normal;


uniform mat4 uni_M;
uniform mat4 uni_VP;

uniform vec3 uni_object_colour;
uniform vec3 uni_light_colour;
uniform vec3 uni_light_direction;
uniform vec3 uni_ambient;


void main()
{
	vec4 world_position = uni_M * vec4(position, 1.0);

	gl_Position = uni_VP * world_position;

//	float light_factor = clamp(dot(normalize((uni_M * vec4(normal, 0.0)).xyz), normalize(uni_light_direction * (-1))), 0, 1);
//    pass_colour = (uni_ambient * uni_object_colour) + (uni_object_colour * (uni_light_colour * light_factor));
	pass_colour = gl_Position.xyz;
	//pass_colour = colour;

	pass_uv = uv;
	pass_normal = (uni_M * vec4(normal, 0.0)).xyz;
	
}