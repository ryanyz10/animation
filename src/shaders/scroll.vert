R"zzz(#version 330 core
in vec4 vertex_position;
in vec2 tex_coord_in;
uniform float scroll_shift;
uniform float scroll_scale;
out vec2 tex_coord;
void main()
{
	tex_coord = tex_coord_in;
	vec4 pos = vertex_position;
    pos.y *= scroll_scale;
    pos.y += 1 - scroll_scale;
	pos.y -= scroll_shift;
	gl_Position = pos;
}
)zzz"
