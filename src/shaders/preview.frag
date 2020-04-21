R"zzz(#version 330 core
out vec4 fragment_color;
in vec2 tex_coord;
uniform sampler2D sampler;
uniform bool show_border;
uniform bool padding;
void main() {
	float d_x = min(tex_coord.x, 1.0 - tex_coord.x);
	float d_y = min(tex_coord.y, 1.0 - tex_coord.y);
	if (padding)
	{
		if (show_border)
		{
			fragment_color = vec4(0.0, 1.0, 0.0, 1.0);
		}
		else
		{
			fragment_color = vec4(0.0, 0.0, 0.0, 1.0);
		}
	}
	else
	{
		if (show_border && (d_x < 0.02 || d_y < 0.02))
		{
			fragment_color = vec4(0.0, 1.0, 0.0, 1.0);
		}
		else
		{
			fragment_color = vec4(texture(sampler, tex_coord).xyz, 1.0);
		}
	}

}
)zzz"
