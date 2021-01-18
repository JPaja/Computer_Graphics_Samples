#version 330

in vec3 tex_cords;
in mat4 out_P;
in mat4 out_V;

out vec4 final_colour;

uniform samplerCube albedo;

void main()
{
 	//vec3 colour = texture(albedo,pass_uv).rgb;
	//final_colour = vec4(1 - colour.r,1 - colour.g,1 - colour.b,1.0);
	vec3 tex = texture(albedo, tex_cords).rgb;
	final_colour = vec4(tex,0.5);
}