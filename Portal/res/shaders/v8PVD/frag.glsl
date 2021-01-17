#version 330

in vec3 pass_colour;
in vec2 pass_uv;
in vec3 pass_normal;

out vec4 final_colour;

uniform sampler2D albedo;

void main()
{
	vec3 normalized_normal = normalize(pass_normal);
	//vec3 normalized_normal = normalize(pass_normal);

//	// Uzimamo uzorak iz bajndovane teksture na koordinatama iz pass_uv i to
//	// direktno upisujemo kao konacnu boju piksela
	vec4 albedo_colour = texture(albedo, pass_uv * 0.5 + vec2(0.25, 0.25));
	final_colour = albedo_colour;

	//final_colour = vec4(pass_colour,1.0f);
	//final_colour = vec4(1.0f);
}