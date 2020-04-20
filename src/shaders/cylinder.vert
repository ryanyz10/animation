R"zzz(#version 330 core
uniform mat4 bone_transform; // transform the cylinder to the correct configuration
const float kPi = 3.1415926535897932384626433832795;
const float R = 0.25; // oops this is hard coded
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
in vec4 vertex_position;

void main() {
	vec4 cyl_pos = vertex_position;
	cyl_pos.x = R * cos(2 * kPi * vertex_position.x);
	cyl_pos.y = R * sin(2 * kPi * vertex_position.x);
	mat4 mvp = projection * view * bone_transform * model;
	gl_Position = mvp * cyl_pos;
}

)zzz"
