R"zzz(#version 330 core
uniform mat4 bone_transform;
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
flat out vec4 color;
in vec4 vertex_position;
void main() {
	mat4 mvp = projection * view * bone_transform * model;
	gl_Position = mvp * vertex_position;
	color = vec4(1, 0, 0, 1);
})zzz"
