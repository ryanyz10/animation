R"zzz(#version 330 core
out vec4 fragment_color;
in vec2 tex_coord;
uniform sampler2D sampler;
uniform bool show_border;
uniform bool padding;
void main() {
	float d_x = min(tex_coord.x, 1.0 - tex_coord.x);
	float d_y = min(tex_coord.y, 1.0 - tex_coord.y);

	// float border = padding ? 0.01 : 0.02;
	float border = 0.01;

	// if (show_border && (d_x < border || d_y < border))
	if (show_border && tex_coord.x < border)
	{
		fragment_color = vec4(0.0, 1.0, 0.0, 1.0);
	}
	else
	{
		fragment_color = padding ? vec4(0.0, 0.0, 0.0, 1.0) : vec4(texture(sampler, tex_coord).xyz, 1.0);
	}
}
)zzz"
