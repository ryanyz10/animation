#include <GL/glew.h>

#include "bone_geometry.h"
#include "procedure_geometry.h"
#include "render_pass.h"
#include "config.h"
#include "gui.h"
#include "texture_to_render.h"
// #include "fonts.h"

#include <memory>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>

#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/io.hpp>
#include <debuggl.h>

#include <stdio.h>

int window_width = 1280;
int window_height = 720;

int main_view_width = 960;
int main_view_height = 720;

int scroll_bar_width = 20;
int scroll_bar_height = 720;

// 300 x 225
int preview_width = window_width - main_view_width - scroll_bar_width;
int preview_height = preview_width / 4 * 3;

// 300 x 15
int padding_width = preview_width;
int padding_height = (window_height - 3 * preview_height) / 3;

// 300 x 720
int preview_bar_width = preview_width;
int preview_bar_height = main_view_height;

const std::string window_title = "Animation";

const char *vertex_shader =
#include "shaders/default.vert"
	;

const char *blending_shader =
#include "shaders/blending.vert"
	;

const char *geometry_shader =
#include "shaders/default.geom"
	;

const char *fragment_shader =
#include "shaders/default.frag"
	;

const char *floor_fragment_shader =
#include "shaders/floor.frag"
	;

const char *bone_vertex_shader =
#include "shaders/bone.vert"
	;

const char *bone_fragment_shader =
#include "shaders/bone.frag"
	;

const char *cylinder_vertex_shader =
#include "shaders/cylinder.vert"
	;

const char *cylinder_fragment_shader =
#include "shaders/cylinder.frag"
	;

const char *axes_vertex_shader =
#include "shaders/axes.vert"
	;

const char *axes_fragment_shader =
#include "shaders/axes.frag"
	;

const char *preview_vertex_shader =
#include "shaders/preview.vert"
	;

const char *preview_fragment_shader =
#include "shaders/preview.frag"
	;

const char *scroll_vertex_shader =
#include "shaders/scroll.vert"
	;

const char *scroll_fragment_shader =
#include "shaders/scroll.frag"
	;

void ErrorCallback(int error, const char *description)
{
	std::cerr << "GLFW Error: " << description << "\n";
}

GLFWwindow *init_glefw()
{
	if (!glfwInit())
		exit(EXIT_FAILURE);
	glfwSetErrorCallback(ErrorCallback);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	// glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_FALSE);
	auto ret = glfwCreateWindow(window_width, window_height, window_title.data(), nullptr, nullptr);
	CHECK_SUCCESS(ret != nullptr);
	glfwMakeContextCurrent(ret);
	glewExperimental = GL_TRUE;
	CHECK_SUCCESS(glewInit() == GLEW_OK);
	glGetError(); // clear GLEW's error for it
	glfwSwapInterval(1);
	const GLubyte *renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte *version = glGetString(GL_VERSION);	// version as a string
	std::cout << "Renderer: " << renderer << "\n";
	std::cout << "OpenGL version supported:" << version << "\n";

	return ret;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cerr << "Input model file is missing" << std::endl;
		std::cerr << "Usage: " << argv[0] << " <PMD file>" << std::endl;
		return -1;
	}

	// create the main window
	GLFWwindow *window = init_glefw();
	GUI gui(window, main_view_width, main_view_height, preview_height);

	// load/generate vertices and faces
	std::vector<glm::vec4> floor_vertices;
	std::vector<glm::uvec3> floor_faces;
	create_floor(floor_vertices, floor_faces);

	LineMesh cylinder_mesh;
	LineMesh axes_mesh;

	create_cylinder_mesh(cylinder_mesh);
	create_axes_mesh(axes_mesh);

	std::vector<glm::vec4> quad_vertices;
	std::vector<glm::uvec3> quad_indices;

	quad_vertices.push_back(glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f));
	quad_vertices.push_back(glm::vec4(1.0f, 1.0f, -1.0f, 1.0f));
	quad_vertices.push_back(glm::vec4(1.0f, 3.0f / 8.0f, -1.0f, 1.0f));
	quad_vertices.push_back(glm::vec4(-1.0f, 3.0f / 8.0f, -1.0f, 1.0f));

	quad_indices.push_back(glm::uvec3(0, 1, 2));
	quad_indices.push_back(glm::uvec3(2, 3, 0));

	std::vector<glm::vec2> quad_uv;
	quad_uv.push_back(glm::vec2(0.0f, 1.0f));
	quad_uv.push_back(glm::vec2(1.0f, 1.0f));
	quad_uv.push_back(glm::vec2(1.0f, 0.0f));
	quad_uv.push_back(glm::vec2(0.0f, 0.0f));

	std::vector<glm::vec4> scroll_vertices;
	std::vector<glm::uvec3> scroll_indices;
	scroll_vertices.push_back(glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f));
	scroll_vertices.push_back(glm::vec4(1.0f, 1.0f, -1.0f, 1.0f));
	scroll_vertices.push_back(glm::vec4(1.0f, -1.0f, -1.0f, 1.0f));
	scroll_vertices.push_back(glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f));

	scroll_indices.push_back(glm::uvec3(0, 1, 2));
	scroll_indices.push_back(glm::uvec3(2, 3, 0));

	Mesh mesh;
	mesh.loadPmd(argv[1]);
	std::cout << "Loaded object  with  " << mesh.vertices.size()
			  << " vertices and " << mesh.faces.size() << " faces.\n";

	glm::vec4 mesh_center = glm::vec4(0.0f);
	for (size_t i = 0; i < mesh.vertices.size(); ++i)
	{
		mesh_center += mesh.vertices[i];
	}
	mesh_center /= mesh.vertices.size();

	gui.assignMesh(&mesh);

	glm::vec4 light_position = glm::vec4(0.0f, 100.0f, 0.0f, 1.0f);
	MatrixPointers mats;

	// create variables backing uniforms
	int top_offset = 0;
	int current_index = 0;
	int selected_index = 0;
	unsigned texture_id = 0;
	unsigned sampler_id = 0;

	bool is_padding = false;

	bool render_texture = false;

	float scroll_bar_shift = 0.0f;
	float scroll_bar_scale = 1.0f;

	int max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);

	// create uniforms
	std::function<const glm::mat4 *()> model_data = [&mats]() {
		return mats.model;
	};
	std::function<glm::mat4()> view_data = [&mats]() { return *mats.view; };
	std::function<glm::mat4()> proj_data = [&mats]() { return *mats.projection; };
	std::function<glm::mat4()> identity_mat = []() { return glm::mat4(1.0f); };
	std::function<glm::vec3()> cam_data = [&gui]() { return gui.getCamera(); };
	std::function<glm::vec4()> lp_data = [&light_position]() { return light_position; };

	std::function<glm::mat4()> bone_mat = [&gui]() { return gui.getBoneTransform(); };
	std::function<std::vector<glm::mat4>()> u_data = [&mesh]() { return mesh.getCurrentQ()->uData(); };
	std::function<std::vector<glm::mat4>()> d_data = [&mesh]() { return mesh.getCurrentQ()->dData(); };

	std::function<float()> offset_data = [&top_offset]() { return ((float)(2.0f * top_offset) / (float)preview_bar_height); };
	std::function<bool()> border_data = [&current_index, &selected_index]() { return current_index == selected_index; };
	std::function<bool()> padding_data = [&is_padding] { return is_padding; };
	std::function<unsigned()> texture_data = [&texture_id] { return texture_id; };
	std::function<unsigned()> sampler_data = [&sampler_id] { return sampler_id; };

	std::function<float()> scroll_shift_data = [&scroll_bar_shift] { return scroll_bar_shift; };
	std::function<float()> scroll_scale_data = [&scroll_bar_scale] { return scroll_bar_scale; };

	auto std_model = std::make_shared<ShaderUniform<const glm::mat4 *>>("model", model_data);
	auto floor_model = make_uniform("model", identity_mat);
	auto std_view = make_uniform("view", view_data);
	auto std_camera = make_uniform("camera_position", cam_data);
	auto std_proj = make_uniform("projection", proj_data);
	auto std_light = make_uniform("light_position", lp_data);

	auto bone_transform = make_uniform("bone_transform", bone_mat);
	auto u_mats = make_uniform("u_mats", u_data);
	auto d_mats = make_uniform("d_mats", d_data);

	auto frame_shift = make_uniform("frame_shift", offset_data);
	auto show_border = make_uniform("show_border", border_data);
	auto padding = make_uniform("padding", padding_data);
	auto sampler = make_texture("sampler", sampler_data, 0, texture_data);

	auto scroll_shift = make_uniform("scroll_shift", scroll_shift_data);
	auto scroll_scale = make_uniform("scroll_scale", scroll_scale_data);

	std::function<float()>
		alpha_data = [&gui, &render_texture]() {
			static const float transparent = 0.5; // Alpha constant goes here
			static const float non_transparent = 1.0;
			if (render_texture)
				return non_transparent;

			if (gui.isTransparent())
				return transparent;
			else
				return non_transparent;
		};
	auto object_alpha = make_uniform("alpha", alpha_data);

	std::function<std::vector<glm::vec3>()> trans_data = [&mesh]() { return mesh.getCurrentQ()->transData(); };
	std::function<std::vector<glm::fquat>()> rot_data = [&mesh]() { return mesh.getCurrentQ()->rotData(); };
	auto joint_trans = make_uniform("joint_trans", trans_data);
	auto joint_rot = make_uniform("joint_rot", rot_data);

	// Floor render pass
	RenderDataInput floor_pass_input;
	floor_pass_input.assign(0, "vertex_position", floor_vertices.data(), floor_vertices.size(), 4, GL_FLOAT);
	floor_pass_input.assignIndex(floor_faces.data(), floor_faces.size(), 3);
	RenderPass floor_pass(-1,
						  floor_pass_input,
						  {vertex_shader, geometry_shader, floor_fragment_shader},
						  {floor_model, std_view, std_proj, std_light},
						  {"fragment_color"});

	// PMD Model render pass
	std::vector<glm::vec2> &uv_coordinates = mesh.uv_coordinates;
	RenderDataInput object_pass_input;
	object_pass_input.assign(0, "jid0", mesh.joint0.data(), mesh.joint0.size(), 1, GL_INT);
	object_pass_input.assign(1, "jid1", mesh.joint1.data(), mesh.joint1.size(), 1, GL_INT);
	object_pass_input.assign(2, "w0", mesh.weight_for_joint0.data(), mesh.weight_for_joint0.size(), 1, GL_FLOAT);
	object_pass_input.assign(3, "vector_from_joint0", mesh.vector_from_joint0.data(), mesh.vector_from_joint0.size(), 3, GL_FLOAT);
	object_pass_input.assign(4, "vector_from_joint1", mesh.vector_from_joint1.data(), mesh.vector_from_joint1.size(), 3, GL_FLOAT);
	object_pass_input.assign(5, "normal", mesh.vertex_normals.data(), mesh.vertex_normals.size(), 4, GL_FLOAT);
	object_pass_input.assign(6, "uv", uv_coordinates.data(), uv_coordinates.size(), 2, GL_FLOAT);
	object_pass_input.assign(7, "vert", mesh.vertices.data(), mesh.vertices.size(), 4, GL_FLOAT);
	object_pass_input.assignIndex(mesh.faces.data(), mesh.faces.size(), 3);
	object_pass_input.useMaterials(mesh.materials);
	RenderPass object_pass(-1,
						   object_pass_input,
						   {blending_shader,
							geometry_shader,
							fragment_shader},
						   {std_model, std_view, std_proj,
							std_light,
							std_camera, object_alpha,
							joint_trans, joint_rot,
							u_mats, d_mats},
						   {"fragment_color"});

	// Setup the render pass for drawing bones
	std::vector<int> bone_vertex_id;
	std::vector<glm::uvec2> bone_indices;
	for (int i = 0; i < (int)mesh.skeleton.joints.size(); i++)
	{
		bone_vertex_id.emplace_back(i);
	}

	for (const auto &joint : mesh.skeleton.joints)
	{
		if (joint.parent_index < 0)
			continue;
		bone_indices.emplace_back(joint.joint_index, joint.parent_index);
	}

	// Skeleton render pass
	RenderDataInput bone_pass_input;
	bone_pass_input.assign(0, "jid", bone_vertex_id.data(), bone_vertex_id.size(), 1, GL_UNSIGNED_INT);
	bone_pass_input.assignIndex(bone_indices.data(), bone_indices.size(), 2);
	RenderPass bone_pass(-1, bone_pass_input,
						 {bone_vertex_shader, nullptr, bone_fragment_shader},
						 {std_model, std_view, std_proj, joint_trans},
						 {"fragment_color"});

	// Cylinder render pass
	RenderDataInput cylinder_pass_input;
	cylinder_pass_input.assign(0, "vertex_position", cylinder_mesh.vertices.data(), cylinder_mesh.vertices.size(), 4, GL_FLOAT);
	cylinder_pass_input.assignIndex(cylinder_mesh.indices.data(), cylinder_mesh.indices.size(), 2);
	RenderPass cylinder_pass(-1,
							 cylinder_pass_input,
							 {cylinder_vertex_shader, nullptr, cylinder_fragment_shader},
							 {std_model, std_view, std_proj, bone_transform},
							 {"fragment_color"});

	// Axes render pass
	RenderDataInput axes_pass_input;
	axes_pass_input.assign(0, "vertex_position", axes_mesh.vertices.data(), axes_mesh.vertices.size(), 4, GL_FLOAT);
	axes_pass_input.assignIndex(axes_mesh.indices.data(), axes_mesh.indices.size(), 2);
	RenderPass axes_pass(-1,
						 axes_pass_input,
						 {axes_vertex_shader, nullptr, axes_fragment_shader},
						 {std_model, std_view, std_proj, bone_transform},
						 {"fragment_color"});

	// Preview render pass
	RenderDataInput preview_pass_input;
	preview_pass_input.assign(0, "vertex_position", quad_vertices.data(), quad_vertices.size(), 4, GL_FLOAT);
	preview_pass_input.assign(1, "tex_coord_in", quad_uv.data(), quad_uv.size(), 2, GL_FLOAT);
	preview_pass_input.assignIndex(quad_indices.data(), quad_indices.size(), 3);
	RenderPass preview_pass(-1,
							preview_pass_input,
							{preview_vertex_shader, nullptr, preview_fragment_shader},
							{frame_shift, show_border, sampler, padding},
							{"fragment_color"});

	// Scrollbar render pass
	RenderDataInput scroll_pass_input;
	scroll_pass_input.assign(0, "vertex_position", scroll_vertices.data(), scroll_vertices.size(), 4, GL_FLOAT);
	scroll_pass_input.assign(1, "tex_coord_in", quad_uv.data(), quad_uv.size(), 2, GL_FLOAT);
	scroll_pass_input.assignIndex(scroll_indices.data(), scroll_indices.size(), 3);
	RenderPass scroll_pass(-1,
						   scroll_pass_input,
						   {scroll_vertex_shader, nullptr, scroll_fragment_shader},
						   {scroll_shift, scroll_scale},
						   {"fragment_color"});

	float aspect = 0.0f;
	std::cout << "center = " << mesh.getCenter() << "\n";

	bool draw_floor = true;
	bool draw_skeleton = true;
	bool draw_object = true;
	bool draw_cylinder = true;

	if (argc >= 3)
	{
		try
		{
			mesh.loadAnimationFrom(argv[2]);
		}
		catch (int err)
		{
			glfwDestroyWindow(window);
			glfwTerminate();
			exit(EXIT_SUCCESS);
		}
	}

	// load font and generate textures for needed characters
	// Font font;
	// font.loadFont("../assets/fonts/pixelmix.ttf");
	// glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// for (char c = '0'; c <= '9'; c++)
	// {
	// 	font.generateChar(c);
	// }

	// font.generateChar('s');
	// font.generateChar('.');
	// font.unload();

	// multismpling texture for anti-aliasing
	TextureToRender main_multisample;
	main_multisample.create(main_view_width, main_view_height, true, max_samples);
	main_multisample.unbind();

	// timer for animating
	auto prev = std::chrono::high_resolution_clock::now();

	// setup ffmpeg
	FILE *vid_out = NULL;
	const char *cmd = "ffmpeg -r 30 -f rawvideo -pix_fmt rgba -s 960x720 -i - "
					  "-threads 0 -preset fast -y -pix_fmt yuv420p -crf 21 -vf vflip ";

	// vid_out = popen(cmd, "w");

	bool recording = false;

	int *pixels = new int[main_view_width * main_view_height];

	// main window loop
	while (!glfwWindowShouldClose(window))
	{
		// Setup some basic window stuff.
		glfwGetFramebufferSize(window, &window_width, &window_height);
		glViewport(0, 0, main_view_width, main_view_height);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDepthFunc(GL_LESS);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glCullFace(GL_BACK);

		main_multisample.bind();

		glViewport(0, 0, main_view_width, main_view_height);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDepthFunc(GL_LESS);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glCullFace(GL_BACK);

		gui.updateMatrices();
		mats = gui.getMatrixPointers();

		std::stringstream title;
		float cur_time = gui.getCurrentPlayTime();
		title << window_title;

		if (gui.isPlaying())
		{
			title << " Playing: "
				  << std::setprecision(2)
				  << std::setfill('0') << std::setw(6)
				  << cur_time << " sec";

			mesh.updateAnimation(cur_time);

			auto now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> diff = now - prev;
			prev = std::chrono::high_resolution_clock::now();
			gui.incCurrentPlayTime(diff.count());
		}
		else if (gui.isPoseDirty())
		{
			title << " Editing";
			mesh.updateAnimation();
			gui.clearPose();
			prev = std::chrono::high_resolution_clock::now();
		}
		else
		{
			title << " Paused: "
				  << std::setprecision(2)
				  << std::setfill('0') << std::setw(6)
				  << cur_time << " sec";

			prev = std::chrono::high_resolution_clock::now();
		}

		glfwSetWindowTitle(window, title.str().data());

		int current_bone = gui.getCurrentBone();

		// render skeleton
		if (draw_skeleton && gui.isTransparent())
		{
			bone_pass.setup();
			CHECK_GL_ERROR(glDrawElements(GL_LINES,
										  bone_indices.size() * 2,
										  GL_UNSIGNED_INT, 0));
		}
		draw_cylinder = (current_bone != -1 && gui.isTransparent());

		// render selected bone
		if (draw_cylinder)
		{
			cylinder_pass.setup();
			CHECK_GL_ERROR(glDrawElements(GL_LINES,
										  cylinder_mesh.indices.size() * 2,
										  GL_UNSIGNED_INT, 0));
			axes_pass.setup();
			CHECK_GL_ERROR(glDrawElements(GL_LINES,
										  axes_mesh.indices.size() * 2,
										  GL_UNSIGNED_INT, 0));
		}

		// render the floor
		if (draw_floor)
		{
			floor_pass.setup();
			CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES,
										  floor_faces.size() * 3,
										  GL_UNSIGNED_INT, 0));
		}

		// render the mesh
		if (draw_object)
		{
			object_pass.setup();
			int mid = 0;
			while (object_pass.renderWithMaterial(mid))
				mid++;
		}

		main_multisample.unbind();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, main_multisample.getFrameBuffer());
		glDrawBuffer(GL_BACK);
		glBlitFramebuffer(0, 0, main_view_width, main_view_height, 0, 0, main_view_width, main_view_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		int curr_preview_row = gui.getCurrentPreviewRow();
		int first_keyframe_index = curr_preview_row / (preview_height + padding_height);
		top_offset = curr_preview_row % (preview_height + padding_height);
		selected_index = gui.getSelectedKeyframe();

		int num_keyframes = mesh.getNumKeyFrames();
		std::vector<KeyFrame> &keyframes = mesh.getKeyFrames();

		// make sure the texture exist
		render_texture = true;
		for (int i = first_keyframe_index; i < first_keyframe_index + 4; i++)
		{
			if (i >= num_keyframes)
			{
				break;
			}

			KeyFrame &keyframe = keyframes[i];
			if (keyframe.texture == nullptr)
			{
				TextureToRender multisampled;
				multisampled.create(main_view_width, main_view_height, true, max_samples);

				mesh.updateWithKeyFrame(i);

				if (draw_floor)
				{
					floor_pass.setup();
					CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES,
												  floor_faces.size() * 3,
												  GL_UNSIGNED_INT, 0));
				}

				if (draw_object)
				{
					object_pass.setup();
					int mid = 0;
					while (object_pass.renderWithMaterial(mid))
						mid++;
				}

				multisampled.unbind();

				TextureToRender *texture = new TextureToRender();
				texture->create(main_view_width, main_view_height);

				glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampled.getFrameBuffer());
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, texture->getFrameBuffer());
				glBlitFramebuffer(0, 0, main_view_width, main_view_height, 0, 0, main_view_width, main_view_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);

				keyframe.texture = texture;
			}
		}

		render_texture = false;

		// render preview bar
		glViewport(main_view_width, 0, preview_bar_width, preview_bar_height);
		glDisable(GL_CULL_FACE);
		glClear(GL_DEPTH_BUFFER_BIT);

		for (current_index = 2 * first_keyframe_index; current_index < 2 * (first_keyframe_index + 4); current_index++)
		{
			if (current_index & 1)
			{
				is_padding = false;
				if (current_index / 2 >= num_keyframes)
					break;

				KeyFrame &keyframe = keyframes[current_index / 2];
				texture_id = keyframe.texture->getTexture();

				preview_pass.setup();
				CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, quad_indices.size() * 3, GL_UNSIGNED_INT, 0));

				top_offset -= preview_height;
			}
			else
			{
				is_padding = true;
				texture_id = 0;

				preview_pass.setup();
				CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, quad_indices.size() * 3, GL_UNSIGNED_INT, 0));

				top_offset -= padding_height;
			}
		}

		// render scroll bar
		glViewport(main_view_width + preview_bar_width, 0, scroll_bar_width, scroll_bar_height);
		glClear(GL_DEPTH_BUFFER_BIT);
		scroll_bar_scale = glm::min(1.0f, (float)(preview_bar_height) / (float)(240 * num_keyframes + padding_height + 600));
		scroll_bar_shift = 2.0f * (float)(curr_preview_row) / (float)(240 * num_keyframes + padding_height + 600);

		scroll_pass.setup();
		CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, scroll_indices.size() * 3, GL_UNSIGNED_INT, 0));

		// Poll and swap.
		glfwPollEvents();
		glfwSwapBuffers(window);

		// send frame to video
		if (recording)
		{
			glReadPixels(0, 0, main_view_width, main_view_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
			fwrite(pixels, sizeof(int) * main_view_width * main_view_height, 1, vid_out);

			if (cur_time > mesh.totalRunningTime())
			{
				gui.setMakeVid(false);
				recording = false;

				// close pipe to video out
				pclose(vid_out);
			}
		}

		// start recording pipe
		if (!recording && gui.getMakeVid())
		{
			recording = true;

			const char *dest_fn = gui.getVideoFilename().c_str();
			char *cmd_copy = new char[strlen(cmd) + strlen(dest_fn) + 1];

			strcpy(cmd_copy, cmd);
			strcat(cmd_copy, dest_fn);

			vid_out = popen(cmd_copy, "w");

			delete[] cmd_copy;
		}
	}
	// close pipe to video out if still going
	if (recording)
		pclose(vid_out);

	delete[] pixels;

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
