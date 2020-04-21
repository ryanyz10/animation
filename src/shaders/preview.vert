R"zzz(#version 330 core
in vec4 vertex_position;
in vec2 tex_coord_in;
uniform float frame_shift;
uniform bool padding;
out vec2 tex_coord;
void main()
{
	tex_coord = tex_coord_in;
	vec4 pos = vertex_position;
	if (padding)
	{
		float scale = 1.0 / 15.0;
		pos.y *= scale;
		pos.y += 1 - scale;
	}
	pos.y += frame_shift;
	gl_Position = pos;
}
)zzz"
