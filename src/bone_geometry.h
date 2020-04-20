#ifndef BONE_GEOMETRY_H
#define BONE_GEOMETRY_H

#include "config.h"
#include "texture_to_render.h"

#include <ostream>
#include <vector>
#include <string>
#include <map>
#include <limits>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <mmdadapter.h>

class TextureToRender;

struct BoundingBox
{
	BoundingBox()
		: min(glm::vec3(-std::numeric_limits<float>::max())),
		  max(glm::vec3(std::numeric_limits<float>::max())) {}
	glm::vec3 min;
	glm::vec3 max;
};

struct Joint
{
	Joint()
		: joint_index(-1),
		  parent_index(-1),
		  position(glm::vec3(0.0f)),
		  init_position(glm::vec3(0.0f))
	{
	}
	Joint(int id, glm::vec3 wcoord, int parent)
		: joint_index(id),
		  parent_index(parent),
		  position(wcoord),
		  init_position(wcoord) //,
								//   init_rel_position(wcoord)
	{
		if (id == 0)
		{
			// only for root
			init_rel_position = wcoord;
		}
	}

	int joint_index;
	int parent_index;
	glm::vec3 position;			 // current wcoord position of the joint, updated when rotated
	glm::fquat orientation;		 // current rotation w.r.t. wcoord initial configuration, start as I
	glm::fquat rel_orientation;	 // this is T_i, rotation w.r.t. it's parent. Used for animation.
	glm::vec3 init_position;	 // initial wcoord position of this joint
	glm::vec3 init_rel_position; // initial relative position to its parent, used for B_x,y
	std::vector<int> children;

	glm::mat3 ti;
	glm::mat3 orient;
};

struct Configuration
{
	std::vector<glm::vec3> trans;
	std::vector<glm::fquat> rot;

	std::vector<glm::mat4> u_mats;
	std::vector<glm::mat4> d_mats;

	const auto &transData() const { return trans; }
	const auto &rotData() const { return rot; }

	// access U and D matrices for shaders
	const auto &uData() const { return u_mats; }
	const auto &dData() const { return d_mats; }
};

struct KeyFrame
{
	std::vector<glm::fquat> rel_rot;
	TextureToRender *texture;

	static void interpolate(const KeyFrame &from,
							const KeyFrame &to,
							float tau,
							KeyFrame &target);
};

struct LineMesh
{
	std::vector<glm::vec4> vertices;
	std::vector<glm::uvec2> indices;
};

struct Skeleton
{

	// bones are identified by their child jid
	std::vector<Joint> joints;

	std::vector<glm::mat4> u_mats;
	std::vector<glm::mat4> d_mats;

	Configuration cache;

	void refreshCache(Configuration *cache = nullptr);
	const glm::vec3 *collectJointTrans() const;
	const glm::fquat *collectJointRot() const;

	// corrects all attributes of the joints
	void initializeJoints();

	// returns index of nearest bone collision, or -1
	int intersectBones(glm::vec4 ray_start, glm::vec4 ray_dir);

	// returns time to collision with bone jid, or -1.0
	float checkBone(glm::vec4 ray_start, glm::vec4 ray_dir, int jid);

	// update position and D for the given bone and recursively for its children
	void fixDmatPosOrient(int jid);

	// save the skeleton to a KeyFrame object
	KeyFrame getKeyFrame();
	void updateFromKeyFrame(const KeyFrame &keyframe);
};

struct Mesh
{
	Mesh();
	~Mesh();
	std::vector<glm::vec4> vertices;

	// Static per-vertex attrributes for Shaders
	std::vector<int32_t> joint0;
	std::vector<int32_t> joint1;
	std::vector<float> weight_for_joint0;
	std::vector<glm::vec3> vector_from_joint0;
	std::vector<glm::vec3> vector_from_joint1;
	std::vector<glm::vec4> vertex_normals;
	std::vector<glm::vec4> face_normals;
	std::vector<glm::vec2> uv_coordinates;
	std::vector<glm::uvec3> faces;

	std::vector<Material> materials;
	BoundingBox bounds;
	Skeleton skeleton;

	void loadPmd(const std::string &fn);
	int getNumberOfBones() const;
	glm::vec3 getCenter() const { return 0.5f * glm::vec3(bounds.min + bounds.max); }
	const Configuration *getCurrentQ() const; // Configuration is abbreviated as Q
	void updateAnimation(float t = -1.0);

	void saveAnimationTo(const std::string &fn);
	void loadAnimationFrom(const std::string &fn);

	void saveToKeyFrame();
	int getNumKeyFrames() { return keyframes.size(); }
	std::vector<KeyFrame> &getKeyFrames() { return keyframes; }

private:
	void computeBounds();
	void computeNormals();
	Configuration currentQ_;

	std::vector<KeyFrame> keyframes;
};

#endif
