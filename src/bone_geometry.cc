#include "config.h"
#include "bone_geometry.h"
#include "texture_to_render.h"
#include <fstream>
#include <queue>
#include <iostream>
#include <stdexcept>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glm/gtx/string_cast.hpp>

/*
 * For debugging purpose.
 */
template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &v)
{
	size_t count = std::min(v.size(), static_cast<size_t>(10));
	for (size_t i = 0; i < count; ++i)
		os << i << " " << v[i] << "\n";
	os << "size = " << v.size() << "\n";
	return os;
}

std::ostream &operator<<(std::ostream &os, const BoundingBox &bounds)
{
	os << "min = " << bounds.min << " max = " << bounds.max;
	return os;
}

const glm::vec3 *Skeleton::collectJointTrans() const
{
	return cache.trans.data();
}

const glm::fquat *Skeleton::collectJointRot() const
{
	return cache.rot.data();
}

// FIXME: Implement bone animation.

void Skeleton::refreshCache(Configuration *target)
{
	if (target == nullptr)
		target = &cache;
	target->rot.resize(joints.size());
	target->trans.resize(joints.size());
	target->u_mats.resize(joints.size());
	target->d_mats.resize(joints.size());
	for (size_t i = 0; i < joints.size(); i++)
	{
		target->rot[i] = joints[i].orientation;
		target->trans[i] = joints[i].position;
		target->u_mats[i] = u_mats[i];
		target->d_mats[i] = d_mats[i];
	}
}

void Skeleton::initializeJoints()
{
	// traverse vector and fix children
	const int J = joints.size();
	for (int i = 0; i < J; i++)
	{
		assert(i == joints[i].joint_index); // just a sanity check
		int pid = joints[i].parent_index;
		if (pid == -1)
			continue;
		joints[pid].children.push_back(i);
	}

	// fix `init_rel_position`, initialize U and D, orientations
	// starting from root, BFS traversal
	u_mats.resize(J);
	d_mats.resize(J);

	std::queue<int> q;
	q.push(0);

	while (!q.empty())
	{
		int jid = q.front();
		q.pop();
		Joint &current = joints[jid];

		// init (rel_)orientation
		current.rel_orientation = glm::quat(1, 0, 0, 0);
		current.ti = glm::mat3(1);
		current.orient = glm::mat3(1);
		current.orientation = glm::quat(1, 0, 0, 0);

		if (jid == 0)
		{
			// root case

			// init U and D
			glm::mat4 B(1.0);
			B[3] = glm::vec4(current.init_rel_position, 1.0);

			u_mats[jid] = B;
			d_mats[jid] = B;
		}
		else
		{
			// precondition: current's parent has been corrected

			current.init_rel_position = current.init_position - joints[current.parent_index].init_position;

			// initialize U and D
			glm::mat4 B(1.0);
			B[3] = glm::vec4(current.init_rel_position, 1.0);

			u_mats[jid] = u_mats[current.parent_index] * B;
			d_mats[jid] = d_mats[current.parent_index] * B /* * current.ti */;
		}

		// add jid's children to queue
		for (int child : current.children)
		{
			q.push(child);
		}
	}
}

// ray in wcoords
int Skeleton::intersectBones(glm::vec4 ray_start, glm::vec4 ray_dir)
{
	int closest_bone = -1;
	float bestT = std::numeric_limits<float>::max();

	const int J = joints.size();
	for (int i = 1; i < J; i++)
	{
		float t = checkBone(ray_start, ray_dir, i);
		// ignores collision at t=0.0, way too close
		if (t > 0.0 && t < bestT)
		{
			// DEBUG
			// std::cout << "hitting bone " << i << " at time " << t << std::endl;
			bestT = t;
			closest_bone = i;
		}
	}

	return closest_bone;
}

// ray in wcoords
float Skeleton::checkBone(glm::vec4 ray_start, glm::vec4 ray_dir, int jid)
{
	// choose bone specified by jid -- extends to current.parent
	Joint &current = joints[jid];
	glm::vec3 bottomPos = current.position;
	glm::vec3 cylinderAxis = joints[current.parent_index].position - current.position;
	float length = glm::length(cylinderAxis);

	////// transform everything from wcoord to system where cylinder is along z-axis //////

	// find 2 orthonormal axes to the cylinder axis
	glm::vec3 bone_z = glm::normalize(cylinderAxis);
	glm::vec3 bone_x = glm::normalize(glm::cross(bone_z, glm::vec3(1, 0, 0)));
	// if we're unlucky and bone_z is parallel to 1,0,0
	if (glm::dot(bone_z, glm::vec3(1, 0, 0)) > 0.99)
	{
		bone_x = glm::normalize(glm::cross(bone_z, glm::vec3(0, 1, 0)));
	}
	glm::vec3 bone_y = glm::normalize(glm::cross(bone_z, bone_x));

	// set up matrices
	glm::mat4 translate_mat(1.0f); // makes identity matrix
	translate_mat[3] = glm::vec4(-1.0f * bottomPos, 1.0f);
	// this rotation goes from bone -> world
	glm::mat4 rotate_mat(glm::vec4(bone_x, 0.0f),
						 glm::vec4(bone_y, 0.0f),
						 glm::vec4(bone_z, 0.0f),
						 glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

	// transform to new coord
	glm::mat4 world2bone = glm::inverse(rotate_mat) * translate_mat;

	ray_start = world2bone * ray_start;
	ray_dir = world2bone * ray_dir;

	////// project onto xy-plane, solve circle-ray intersection //////

	// DON'T NORMALIZE -- i think. keeps velocity uniform across bones
	glm::vec2 p(ray_start); // ray xy start
	glm::vec2 v(ray_dir);	// ray xy dir

	// solve || (p_x + t * v_x, p_y + t * v_y) ||^2 = r^2 *** r = KCylinderRadius
	float A = v.x * v.x + v.y * v.y;
	float B = 2 * p.x * v.x + 2 * p.y * v.y;
	float C = p.x * p.x + p.y * p.y - kCylinderRadius * kCylinderRadius;

	if (A == 0.0)
	{
		// ray runs parallel to cylinder axis
		return -1.0f;
	}

	float det = B * B - 4 * A * C;

	if (det < 0.0)
	{
		// no intersection
		return -1.0f;
	}

	// check closer intersection point
	float t1 = glm::min((-B + sqrt(det)) / 2 / A, (-B - sqrt(det)) / 2 / A);
	float t2 = glm::max((-B + sqrt(det)) / 2 / A, (-B - sqrt(det)) / 2 / A);

	if (t1 > 0.0)
	{
		// see if z-coord within bounds
		glm::vec3 final_pos = ray_start + t1 * ray_dir;
		if (final_pos.z >= 0.0 && final_pos.z <= length)
		{
			return t1;
		}
	}

	if (t2 > 0.0)
	{
		// see if z-coord within bounds
		glm::vec3 final_pos = ray_start + t2 * ray_dir;
		if (final_pos.z >= 0.0 && final_pos.z <= length)
		{
			return t2;
		}
	}

	return -1.0f;
}

// precondition: parent D is correct, t_i is correct
void Skeleton::fixDmatPosOrient(int jid)
{
	Joint &currentJ = joints[jid];

	// fix D
	if (jid != 0)
	{
		const int pid = currentJ.parent_index;
		Joint &parentJ = joints[pid];

		glm::mat4 B(1.0);
		B[3] = glm::vec4(glm::mat3(currentJ.ti) * currentJ.init_rel_position, 1.0);

		d_mats[jid] = d_mats[pid] * B * glm::mat4(currentJ.ti);

		currentJ.orient = currentJ.ti * parentJ.orient;

		currentJ.rel_orientation = glm::toQuat(currentJ.ti);

		currentJ.orientation = glm::toQuat(currentJ.orient);
	}

	// fix pos
	currentJ.position = d_mats[jid] * glm::vec4(0, 0, 0, 1);

	for (const auto &child : currentJ.children)
	{
		fixDmatPosOrient(child);
	}
}

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::loadPmd(const std::string &fn)
{
	MMDReader mr;
	mr.open(fn);
	mr.getMesh(vertices, faces, vertex_normals, uv_coordinates);
	computeBounds();
	mr.getMaterial(materials);

	// FIXME: load skeleton and blend weights from PMD file,
	//        initialize std::vectors for the vertex attributes,
	//        also initialize the skeleton as needed

	// joint IDs from 0 to some pos. #
	// 0 is root, joints form a single tree
	// ==> we will fetch joints as long as the IDs are valid
	int id = 0;
	glm::vec3 wcoord; // api differs from spec, went with api
	int parent;		  // -1 if root
	while (mr.getJoint(id, wcoord, parent))
	{
		// add joint to skeleton
		skeleton.joints.emplace_back(Joint(id, wcoord, parent));

		id++;
	}

	// initialize per-vertex attribute vectors
	const int V = vertices.size();
	joint0.reserve(V);
	joint1.reserve(V);
	weight_for_joint0.reserve(V);
	vector_from_joint0.reserve(V);
	vector_from_joint1.reserve(V);
	face_normals.reserve(V);

	// TODO initialize face_normal?

	// put blend weights into vertex attr.
	std::vector<SparseTuple> weights;
	mr.getJointWeights(weights);

	for (int i = 0; i < V; i++)
	{
		int vid = weights[i].vid;
		joint0.push_back(weights[i].jid0);
		joint1.push_back(weights[i].jid1);
		weight_for_joint0.push_back(weights[i].weight0);

		// initialize vector_from_joint#
		const int jid0 = weights[i].jid0;
		const int jid1 = weights[i].jid1;
		const glm::vec3 curr_vert = glm::vec3(vertices[vid]);
		vector_from_joint0.push_back(curr_vert - skeleton.joints[jid0].position);
		if (jid1 < 0)
		{
			vector_from_joint1.push_back(glm::vec3(0));
		}
		else
		{
			vector_from_joint1.push_back(curr_vert - skeleton.joints[jid1].position);
		}
	}

	// initialize the rest of the the joint attributes
	skeleton.initializeJoints();

	skeleton.refreshCache();
}

int Mesh::getNumberOfBones() const
{
	return skeleton.joints.size();
}

void Mesh::computeBounds()
{
	bounds.min = glm::vec3(std::numeric_limits<float>::max());
	bounds.max = glm::vec3(-std::numeric_limits<float>::max());
	for (const auto &vert : vertices)
	{
		bounds.min = glm::min(glm::vec3(vert), bounds.min);
		bounds.max = glm::max(glm::vec3(vert), bounds.max);
	}
}

void Mesh::updateAnimation(float t)
{
	skeleton.refreshCache(&currentQ_);
	// FIXME: Support Animation Here
}

const Configuration *
Mesh::getCurrentQ() const
{
	return &currentQ_;
}
