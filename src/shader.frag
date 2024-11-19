#version 330

in vec3 v_vertex;
in vec3 v_color;
in vec2 v_uv;
in vec3 v_normal;

out vec4 fragColor;

uniform vec3 u_color;
uniform sampler2D u_texture;

uniform vec3 u_light;
uniform float u_light_intensity;
uniform vec3 u_cam_pos;
uniform vec3 u_ambient;
uniform vec3 u_diffuse;
uniform vec3 u_specular;
uniform float u_shininess;

uniform float u_alpha;

void main(void)
{
	// material color
	vec3 material = texture(u_texture, v_uv).rgb;

	// ambient
	vec3 ambient = material * u_ambient * u_light_intensity;

	// diffuse
	vec3 normal = normalize(v_normal);
	vec3 light = normalize(u_light - v_vertex);
	float n_dot_l = max(dot(normal, light), 0.0f);
	vec3 diffuse = material * n_dot_l * u_diffuse * u_light_intensity;

	// specular (phong)
	vec3 reflection = normalize(-reflect(light, normal));
	vec3 eye = normalize(u_cam_pos - v_vertex);			// view, sometimes v (r_dot_v)
	float r_dot_e = max(dot(reflection, eye), 0.0f);
	vec3 specular = material * pow(r_dot_e, u_shininess) * u_specular * u_light_intensity;

	// specular (blinn-phong)
	vec3 half_vector = normalize(light + eye);			// half-vector between light-vector and eye-vector
	float n_dot_h = max(dot(normal, half_vector), 0.0f);
	vec3 specular_blinn = material * pow(n_dot_h, u_shininess) * u_specular * u_light_intensity;

	// blinn-phong
	vec3 final_color = ambient + diffuse + specular_blinn;
	// replace specular_blinn with specular for phong reflectance equation

	fragColor = vec4(final_color, u_alpha);

	// special case of alpha map and skybox
	if(u_alpha == -1.0f) {
		fragColor = texture(u_texture, v_uv);
	}

	// TEST CODES
	//fragColor = vec4(texture(u_texture, v_uv).rgb, 1.0);
	//fragColor = vec4(v_uv, 0.0f, 1.0f);

}
