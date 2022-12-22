#pragma once

#include "RayTracer.h"
#include "Shape.h"

class Sphere : public Shape
{
public:
	Sphere(Vec3f position = { 0 }, Float radius = 0.5, Vec3f albedo = { 0 }) : Shape(position, albedo), Position(position), Radius(radius) {}

	virtual bool Hit(const Ray<Float>& r, Float tMin, Float tMax, Float& hitDistance) const override
	{
		Vec3f oc = r.GetOrigin() - Position; // origin of ray - origin of sphere
		auto a = Dot(r.GetDirection(), r.GetDirection()); // square the direction
		auto b = 2.0 * Dot(oc, r.GetDirection());
		auto c = Dot(oc, oc) - Radius * Radius;
		auto discriminant = b * b - 4.0 * a * c; // quadratic formula
		if (discriminant >= 0.0) // if we hit the sphere
		{
			auto sqrtd = sqrt(discriminant);
			Float t0 = (-b - sqrtd) / (2.0 * a); // CLOSEST T (SMALLEST)
			Float t1 = (-b + sqrtd) / (2.0 * a); // second "hit' is the ray leaving the object, in our case a sphere

			//Vec3f h0 = r.At(t0); // CLOSEST HITPOINT (SMALLEST)
			//Vec3f h1 = r.At(t1);
			//if (h0 > h1) std::swap(h0, h1);

			hitDistance = t0;

			return true;
		}

		// we only reach here if we didn't hit the sphere
		//Vector3 unit_direction = Normalize(r.GetDirection()); // the unit vector (magnitude == 1) of the rays direction
		//auto t = 0.5 * (unit_direction.y + 1.0); // make t 0 -> 1
		//return Utils::VectorToUInt32(Utils::Lerp(Vector3<Float>(1.0), Vector3<Float>(0.5, 0.7, 1.0), t)); // lerp the values to white -> light blue based on y coord
		return false;
	}

public:
	Vec3f Position;
	Float Radius;
};