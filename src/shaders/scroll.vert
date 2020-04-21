R"zzz(#version 330 core
in vec4 vertex_position;
uniform float scroll_shift;
uniform float scroll_scale;
void main()
{
	vec4 pos = vertex_position;
    pos.y *= scroll_scale;
    pos.y += 1 - scroll_scale;
	pos.y -= scroll_shift;
	gl_Position = pos;
}
)zzz"
