#pragma once
#ifndef _BOUNDING_SPHERE_H_
#define _BOUNDING_SPHERE_H_
#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>
#include <string>
#include <vector>
#include <memory>

class Program;

class BoundingSphere
{
public:
	BoundingSphere();
	BoundingSphere(Eigen::Vector3f bcenter, float bradius);
	virtual ~BoundingSphere();
	bool willCollide(Eigen::Vector3f eye);
	
	
private:
	Eigen::Vector3f center;
	float radius;
};

#endif
