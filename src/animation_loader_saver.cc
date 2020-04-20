#include "bone_geometry.h"
#include "texture_to_render.h"
#include <fstream>
#include <iostream>
#include <glm/gtx/io.hpp>
#include <unordered_map>
/*
 * We put these functions to a separated file because the following jumbo
 * header consists of 20K LOC, and it is very slow to compile.
 */
#include "json.hpp"

using json = nlohmann::json;
namespace
{
const glm::fquat identity(1.0, 0.0, 0.0, 0.0);
}

void Mesh::saveAnimationTo(const std::string &fn)
{
	json jayson;
	const int FRAMES = keyframes.size();
	const int JOINTS = skeleton.joints.size();
	for (int f = 0; f < FRAMES; f++)
	{
		for (int j = 0; j < JOINTS; j++)
		{
			glm::quat q = keyframes[f].rel_rot[j];
			jayson[f][j] = {q.w, q.x, q.y, q.z};
		}
	}
	std::ofstream out(fn);
	out << std::setw(4) << jayson << std::endl;
	std::cout << "Saved animation to " << fn << std::endl;
}

void Mesh::loadAnimationFrom(const std::string &fn)
{
	std::ifstream in(fn);
	json jayson;
	in >> jayson;

	const int FRAMES = jayson.size();
	const int JOINTS = jayson[0].size();

	// handle if wrong number of joints
	if (skeleton.joints.size() != JOINTS)
	{
		std::cerr << "## Joints in json (" << JOINTS << ") differed from joints in model ("
				  << skeleton.joints.size() << ") ##\n## Incompatible models, exiting ##" << std::endl;
		throw 0;
	}

	for (int f = 0; f < FRAMES; f++)
	{
		KeyFrame kf;
		for (int j = 0; j < JOINTS; j++)
		{
			glm::fquat q(jayson[f][j][0], jayson[f][j][1], jayson[f][j][2], jayson[f][j][3]);
			kf.rel_rot.push_back(q);
		}
		kf.texture = nullptr;
		keyframes.push_back(kf);
	}
}
