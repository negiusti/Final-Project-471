#include "BoundingSphere.h"
#include <iostream>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>

#include "GLSL.h"
#include "Program.h"


using namespace std;

BoundingSphere::BoundingSphere()
{}

BoundingSphere::BoundingSphere(Eigen::Vector3f bcenter, float bradius)
{
	center = bcenter;
	radius = bradius;
}

BoundingSphere::~BoundingSphere()
{
}

bool BoundingSphere::willCollide(Eigen::Vector3f eye)
{	
	Eigen::Vector3f eyeCopy;
	eyeCopy[0] = eye[0];
	eyeCopy[1] = 1;
	eyeCopy[2] = eye[2];
	Eigen::Vector3f distVec = eye-center;
	float dist = sqrt(distVec[0]*distVec[0] + distVec[1]*distVec[1] + distVec[2]*distVec[2]);
	if (dist > radius + 1) 
	{
		return false;
	}
	return true;
}


