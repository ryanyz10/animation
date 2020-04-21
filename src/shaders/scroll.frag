R"zzz(#version 330 core
in vec2 tex_coord;
out vec4 fragment_color;
void main() {
    float d_x = min(tex_coord.x, 1.0 - tex_coord.x);

	if (d_x >= 0.2)
	{
		fragment_color = vec4(0.521568627, 0.521568627, 0.521568627, 1.0);
	}
	else
	{
		fragment_color = vec4(0.0, 0.0, 0.0, 1.0);
	}
}
)zzz"
