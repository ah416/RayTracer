#pragma once

#include "RayTracer.h"
#include "Shape.h"
#include "Light.h"
#include "Vector.h"

#include <vector>

struct Scene
{
	std::vector<Shape*> shapes;
	std::vector<Light*> lights;
};