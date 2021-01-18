#version 330

in vec2 pass_uv;

out vec4 final_colour;

uniform sampler2D albedo;

void main()
{
 	vec3 colour = texture(albedo,pass_uv).rgb;
	final_colour = vec4(1 - colour.r,1 - colour.g,1 - colour.b,1.0);
}