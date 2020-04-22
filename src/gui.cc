#include "gui.h"
#include "config.h"
#include <jpegio.h>
#include "bone_geometry.h"
#include <iostream>
#include <algorithm>
#include <debuggl.h>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <chrono>
#include "tinyfiledialogs.h"

namespace
{
// FIXME: Implement a function that performs proper
//        ray-cylinder intersection detection
// TIPS: The implement is provided by the ray-tracer starter code.
}

GUI::GUI(GLFWwindow *window, int view_width, int view_height, int preview_height)
	: window_(window), preview_height_(preview_height)
{
	glfwSetWindowUserPointer(window_, this);
	glfwSetKeyCallback(window_, KeyCallback);
	glfwSetCursorPosCallback(window_, MousePosCallback);
	glfwSetMouseButtonCallback(window_, MouseButtonCallback);
	glfwSetScrollCallback(window_, MouseScrollCallback);

	glfwGetWindowSize(window_, &window_width_, &window_height_);
	if (view_width < 0 || view_height < 0)
	{
		view_width_ = window_width_;
		view_height_ = window_height_;
	}
	else
	{
		view_width_ = view_width;
		view_height_ = view_height;
	}
	float aspect_ = static_cast<float>(view_width_) / view_height_;
	projection_matrix_ = glm::perspective((float)(kFov * (M_PI / 180.0f)), aspect_, kNear, kFar);
}

GUI::~GUI()
{
}

void GUI::assignMesh(Mesh *mesh)
{
	mesh_ = mesh;
	center_ = mesh_->getCenter();
}

void GUI::keyCallback(int key, int scancode, int action, int mods)
{
#if 0
	if (action != 2)
		std::cerr << "Key: " << key << " action: " << action << std::endl;
#endif
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window_, GL_TRUE);
		return;
	}
	if (key == GLFW_KEY_J && action == GLFW_RELEASE)
	{
		unsigned char *pixels = new unsigned char[window_width_ * window_height_ * 3];
		glReadPixels(0, 0, window_width_, window_height_, GL_RGB, GL_UNSIGNED_BYTE, pixels);

		// get a unique name
		const auto p1 = std::chrono::system_clock::now();
		std::string filename = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count()) + ".jpg";

		// save jpeg
		SaveJPEG(filename, window_width_, window_height_, pixels);

		delete[] pixels;
	}
	if (key == GLFW_KEY_S && (mods & GLFW_MOD_CONTROL))
	{
		if (action == GLFW_RELEASE)
		{
			if (mods & GLFW_MOD_SHIFT)
			{
				const char *input = tinyfd_inputBox("Enter filename", "Filename to save to:", "animation");

				if (!input)
					return;

				std::string filename(input);
				int length = filename.length();

				if (length < 5 || filename.substr(length - 5) != ".json")
				{
					filename += ".json";
				}

				mesh_->saveAnimationTo(filename);
			}
			else
			{
				mesh_->saveAnimationTo("animation.json");
			}
		}

		return;
	}

	if (mods == 0 && captureWASDUPDOWN(key, action))
		return;
	if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT)
	{
		if (current_bone_ == -1)
			return;

		float roll_speed;
		if (key == GLFW_KEY_RIGHT)
			roll_speed = -roll_speed_;
		else
			roll_speed = roll_speed_;

		Joint &currentJ = mesh_->skeleton.joints[current_bone_];
		Joint &parentJ = mesh_->skeleton.joints[currentJ.parent_index];
		glm::vec3 axis = currentJ.position - parentJ.position;

		glm::mat3 b_rot = glm::rotate(roll_speed, glm::normalize(axis));

		// update T_i for current_bone_
		currentJ.ti = b_rot * currentJ.ti;

		// TODO optimization by setting dirty_joint, start update from there?
		pose_changed_ = true;

		calcBoneTransform();

		return;
	}
	else if (key == GLFW_KEY_C && action != GLFW_RELEASE)
	{
		fps_mode_ = !fps_mode_;
	}
	else if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_RELEASE)
	{
		current_bone_--;
		current_bone_ += mesh_->getNumberOfBones();
		current_bone_ %= mesh_->getNumberOfBones();
		calcBoneTransform();
	}
	else if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_RELEASE)
	{
		current_bone_++;
		current_bone_ += mesh_->getNumberOfBones();
		current_bone_ %= mesh_->getNumberOfBones();
		calcBoneTransform();
	}
	else if (key == GLFW_KEY_T && action != GLFW_RELEASE)
	{
		if (mods & GLFW_MOD_SHIFT && (selected_keyframe & 1) == 0)
		{
			// add time delay
			const char *input = tinyfd_inputBox("Enter duration", "Duration of delay:", "0");

			if (!input)
				return;

			std::string time_str(input);
			try
			{
				int time = std::stoi(time_str);
			}
			catch (const std::exception &e)
			{
				std::cerr << e.what() << std::endl;
				return;
			}
		}
		else
		{
			transparent_ = !transparent_;
		}
	}
	else if (key == GLFW_KEY_F && action == GLFW_RELEASE)
	{
		if (selected_keyframe & 1)
			mesh_->saveToKeyFrame();
		else
			mesh_->insertKeyFrame(selected_keyframe / 2);
	}
	else if (key == GLFW_KEY_P && action == GLFW_RELEASE)
	{
		play_ = !play_;
	}
	else if (key == GLFW_KEY_R && action == GLFW_RELEASE)
	{
		play_ = false;
		current_play_time = 0.0f;
		mesh_->updateAnimation(current_play_time);
	}
	else if (key == GLFW_KEY_U && action == GLFW_RELEASE)
	{
		if (selected_keyframe == -1 || (selected_keyframe & 1) == 0)
			return;

		mesh_->updateKeyFrame(selected_keyframe >> 1);
	}
	else if (key == GLFW_KEY_DELETE && action == GLFW_RELEASE)
	{
		if (selected_keyframe == -1 || (selected_keyframe & 1) == 0)
			return;

		mesh_->deleteKeyFrame(selected_keyframe >> 1);
	}
	else if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
	{
		if (selected_keyframe == -1 || (selected_keyframe & 1) == 0)
			return;

		// TODO should we pause?
		mesh_->updateAnimation(selected_keyframe >> 1);
	}
	else if (key == GLFW_KEY_PAGE_UP && action == GLFW_RELEASE)
	{
		selected_keyframe -= 1;
		if (selected_keyframe < 0)
			selected_keyframe = 0;

		// TODO fix this
		// if (current_preview_row > 240 * selected_keyframe)
		// 	current_preview_row = 240 * selected_keyframe;
	}
	else if (key == GLFW_KEY_PAGE_DOWN && action == GLFW_RELEASE)
	{
		selected_keyframe += 1;
		if (selected_keyframe >= 2 * (mesh_->getNumKeyFrames() + 1))
			selected_keyframe -= 1;

		// TODO fix this
		// if (current_preview_row + window_height_ < 240 * (selected_keyframe + 1))
		// 	current_preview_row = 240 * (selected_keyframe + 1) - window_height_;
	}

	// FIXME: implement other controls here.
}

void GUI::mousePosCallback(double mouse_x, double mouse_y)
{
	last_x_ = current_x_;
	last_y_ = current_y_;
	current_x_ = mouse_x;
	current_y_ = window_height_ - mouse_y;
	float delta_x = current_x_ - last_x_;
	float delta_y = current_y_ - last_y_;
	if (sqrt(delta_x * delta_x + delta_y * delta_y) < 1e-15)
		return;
	if (mouse_x > view_width_)
		return;
	glm::vec3 mouse_direction = glm::normalize(glm::vec3(delta_x, delta_y, 0.0f));
	glm::vec2 mouse_start = glm::vec2(last_x_, last_y_);
	glm::vec2 mouse_end = glm::vec2(current_x_, current_y_);
	glm::uvec4 viewport = glm::uvec4(0, 0, view_width_, view_height_);

	bool drag_camera = drag_state_ && current_button_ == GLFW_MOUSE_BUTTON_RIGHT;
	bool drag_bone = drag_state_ && current_button_ == GLFW_MOUSE_BUTTON_LEFT;

	if (drag_camera)
	{
		glm::vec3 axis = glm::normalize(
			orientation_ *
			glm::vec3(mouse_direction.y, -mouse_direction.x, 0.0f));
		orientation_ =
			glm::mat3(glm::rotate(rotation_speed_, axis) * glm::mat4(orientation_));
		tangent_ = glm::column(orientation_, 0);
		up_ = glm::column(orientation_, 1);
		look_ = glm::column(orientation_, 2);
	}
	else if (drag_bone && current_bone_ != -1)
	{
		Joint &currentJ = mesh_->skeleton.joints[current_bone_];

		// find the axis of rotation and 'speed' (cross of bone axis and mouse direction?)
		// line from eye to parent joint
		Joint &parentJ = mesh_->skeleton.joints[currentJ.parent_index];
		glm::vec3 axis(eye_ - parentJ.position); // both in wcoords

		// angle/speed regulated by cross product of bone axis and mouse direction
		// project bone axis to NDC
		glm::vec3 bone = mesh_->skeleton.joints[current_bone_].position - parentJ.position;
		float length = glm::length(bone);
		bone = projection_matrix_ * view_matrix_ * glm::vec4(bone, 0);
		glm::vec3 bone_dir(glm::vec2(bone), 0);
		bone_dir = glm::normalize(bone_dir);
		float angle = (-0.05 / length) * glm::cross(glm::vec3(delta_x, delta_y, 0.0f), bone_dir).z; // should be radians

		glm::mat3 b_rot = glm::rotate(angle, glm::normalize(axis));

		// update T_i for current_bone_
		currentJ.ti = b_rot * currentJ.ti;

		// TODO optimization by setting dirty_joint, start update from there?
		pose_changed_ = true;

		calcBoneTransform();

		return;
	}

	// highlight bones that have been moused over

	////// make a ray going towards mouse //////
	glm::vec4 ray_start(eye_, 1.0); // in wcoords

	// let's try unProject ==> https://stackoverflow.com/questions/9901453/using-glms-unproject
	glm::vec3 win(current_x_, current_y_, kNear);
	glm::vec4 ray_end(glm::unProject(win,
									 view_matrix_ * model_matrix_,
									 projection_matrix_,
									 viewport),
					  1.0);
	// VERY IMPORTANT unproject returns in wcoords

	////// perform ray-cylinder intersection detection ////
	glm::vec4 ray_dir = glm::normalize(ray_end - ray_start);

	// sets to -1 if no intersection
	current_bone_ = mesh_->skeleton.intersectBones(ray_start, ray_dir);

	if (current_bone_ == -1)
		return;

	calcBoneTransform();
}

void GUI::calcBoneTransform()
{
	// make bone_transform matrix - this should translate + scale cylinder mesh to the current bone
	Joint &currentJ = mesh_->skeleton.joints[current_bone_];
	glm::vec3 bottomPos = currentJ.position;
	glm::vec3 cylinderAxis = mesh_->skeleton.joints[currentJ.parent_index].position - currentJ.position;
	float length = glm::length(cylinderAxis);

	// find 2 orthonormal axes to the cylinder axis
	glm::vec3 bone_z = glm::normalize(cylinderAxis);
	glm::vec3 bone_x = glm::normalize(glm::cross(bone_z, currentJ.orient[0]));
	// if we're unlucky and bone_z is parallel to 1,0,0
	if (glm::dot(bone_z, glm::vec3(1, 0, 0)) > 0.99)
	{
		bone_x = glm::normalize(glm::cross(bone_z, currentJ.orient[1]));
	}
	glm::vec3 bone_y = glm::normalize(glm::cross(bone_z, bone_x));

	// set up matrices
	glm::mat4 translate_mat(1.0f);
	translate_mat[3] = glm::vec4(-1.0f * bottomPos, 1.0f);
	// this rotation goes from bone -> world
	glm::mat4 rotate_mat(glm::vec4(bone_x, 0.0f),
						 glm::vec4(bone_y, 0.0f),
						 glm::vec4(bone_z, 0.0f),
						 glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	glm::mat4 scale_mat(1.0f);
	scale_mat[2] = glm::vec4(0, 0, 1.0 / length, 0);

	// transforms bone to world origin, 1 unit high
	bone_transform = scale_mat * glm::inverse(rotate_mat) * translate_mat;
	bone_transform = glm::inverse(bone_transform);
}

void GUI::mouseButtonCallback(int button, int action, int mods)
{
	if (current_x_ <= view_width_)
	{
		drag_state_ = (action == GLFW_PRESS);
		current_button_ = button;
		return;
	}

	if (action != GLFW_RELEASE)
		return;

	int first_index = current_preview_row / 240;
	int top_offset = current_preview_row % 240;

	//                  ______         ___
	//                  __|___ buffer   |
	//    _____________   |            _|_ top_offset
	//   |            |   |    frame
	//   |            |   |
	//   |            | __|_
	//   |            |

	int test_keyframe = first_index + (((window_height_ - current_y_) + top_offset) / 240);
	bool in_buffer = ((int)((window_height_ - current_y_) + top_offset) % 240) <= 15;

	test_keyframe = 2 * test_keyframe + (in_buffer ? 0 : 1);

	if (test_keyframe > 2 * mesh_->getNumKeyFrames())
		return;

	if (test_keyframe == selected_keyframe)
	{
		if (selected_keyframe & 1)
		{
			// selected keyframe is a preview frame
			if (in_buffer)
				selected_keyframe = test_keyframe * 2;
			else
				selected_keyframe = -1;
		}
		else
		{
			if (in_buffer)
				selected_keyframe = -1;
			else
				selected_keyframe = test_keyframe * 2 + 1;
		}
	}
	else
	{
		selected_keyframe = test_keyframe;
	}
}

void GUI::mouseScrollCallback(double dx, double dy)
{
	if (current_x_ < view_width_)
		return;

	if (mesh_->getNumKeyFrames() == 0)
		return;

	current_preview_row -= (int)(50 * dy);

	// check bounds
	if (current_preview_row < 0)
	{
		current_preview_row = 0;
	}

	if (current_preview_row > 240 * mesh_->getNumKeyFrames() + 15 - 120)
	{
		current_preview_row = 240 * mesh_->getNumKeyFrames() + 15 - 120;
	}
}

void GUI::updateMatrices()
{
	// Compute our view, and projection matrices.
	if (fps_mode_)
		center_ = eye_ + camera_distance_ * look_;
	else
		eye_ = center_ - camera_distance_ * look_;

	view_matrix_ = glm::lookAt(eye_, center_, up_);
	light_position_ = glm::vec4(eye_, 1.0f);

	aspect_ = static_cast<float>(view_width_) / view_height_;
	projection_matrix_ =
		glm::perspective((float)(kFov * (M_PI / 180.0f)), aspect_, kNear, kFar);
	model_matrix_ = glm::mat4(1.0f);
}

MatrixPointers GUI::getMatrixPointers() const
{
	MatrixPointers ret;
	ret.projection = &projection_matrix_;
	ret.model = &model_matrix_;
	ret.view = &view_matrix_;
	return ret;
}

bool GUI::setCurrentBone(int i)
{
	if (i < 0 || i >= mesh_->getNumberOfBones())
		return false;
	current_bone_ = i;
	return true;
}

void GUI::incCurrentPlayTime(float secs)
{
	current_play_time += secs;
}

bool GUI::captureWASDUPDOWN(int key, int action)
{
	if (key == GLFW_KEY_W)
	{
		if (fps_mode_)
			eye_ += zoom_speed_ * look_;
		else
			camera_distance_ -= zoom_speed_;
		return true;
	}
	else if (key == GLFW_KEY_S)
	{
		if (fps_mode_)
			eye_ -= zoom_speed_ * look_;
		else
			camera_distance_ += zoom_speed_;
		return true;
	}
	else if (key == GLFW_KEY_A)
	{
		if (fps_mode_)
			eye_ -= pan_speed_ * tangent_;
		else
			center_ -= pan_speed_ * tangent_;
		return true;
	}
	else if (key == GLFW_KEY_D)
	{
		if (fps_mode_)
			eye_ += pan_speed_ * tangent_;
		else
			center_ += pan_speed_ * tangent_;
		return true;
	}
	else if (key == GLFW_KEY_DOWN)
	{
		if (fps_mode_)
			eye_ -= pan_speed_ * up_;
		else
			center_ -= pan_speed_ * up_;
		return true;
	}
	else if (key == GLFW_KEY_UP)
	{
		if (fps_mode_)
			eye_ += pan_speed_ * up_;
		else
			center_ += pan_speed_ * up_;
		return true;
	}
	return false;
}

// Delegrate to the actual GUI object.
void GUI::KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	GUI *gui = (GUI *)glfwGetWindowUserPointer(window);
	gui->keyCallback(key, scancode, action, mods);
}

void GUI::MousePosCallback(GLFWwindow *window, double mouse_x, double mouse_y)
{
	GUI *gui = (GUI *)glfwGetWindowUserPointer(window);
	gui->mousePosCallback(mouse_x, mouse_y);
}

void GUI::MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
	GUI *gui = (GUI *)glfwGetWindowUserPointer(window);
	gui->mouseButtonCallback(button, action, mods);
}

void GUI::MouseScrollCallback(GLFWwindow *window, double dx, double dy)
{
	GUI *gui = (GUI *)glfwGetWindowUserPointer(window);
	gui->mouseScrollCallback(dx, dy);
}
