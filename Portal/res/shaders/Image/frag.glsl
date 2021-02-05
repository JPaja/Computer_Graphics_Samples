#version 330

in vec3 pass_colour;
in vec2 pass_uv;
in vec3 pass_normal;

out vec4 final_colour;

uniform sampler2D albedo;

void main()
{
	vec3 albedo_colour = texture(albedo, pass_uv).rgb;
	final_colour = vec4(albedo_colour,1.0);
}