#include "Renderer.h"

#include "Ray.h"
#include "Vector.h"

Camera Renderer::m_Camera;
RenderSettings Renderer::m_Settings = { .NumberOfSamples=1 };

void Renderer::Render(const Scene& scene, Image& img, const Camera& cam)
{
	m_Camera = cam;
	for (int y = img.Height - 1; y >= 0; y--)
	{
		for (int x = 0; x < (int64_t)img.Width; x++)
		{
			Vector3<Float> res = 0.0;
			for (int i = 0; i < m_Settings.NumberOfSamples; i++)
			{
				auto u = double(x + Utils::RandomFloat()) / (img.Width - 1); // transform the x coordinate to 0 -> 1 (rather than 0 -> image_width)
				auto v = double(y + Utils::RandomFloat()) / (img.Height - 1); // transform the y coordinate to 0 -> 1 (rather than 0 -> image_height)
				res += TraceRay(scene, { u, v });
			}

			img.Data[x + y * img.Width] = Utils::VectorToUInt32(res / m_Settings.NumberOfSamples);
		}
	}
}

Vector3<Float> Renderer::TraceRay(const Scene& scene, Vector2<Float>&& coord)
{
	const Ray<Float> r = Ray(m_Camera.GetOrigin(), m_Camera.GetLowerLeftCorner() + coord.x * m_Camera.GetHorizontal() + coord.y * m_Camera.GetVertical() - m_Camera.GetOrigin());

	HitRecord hit = { 0 };
	for (const Shape* shape : scene.shapes)
	{
		if (shape->Hit(r, hit))
		{
			return hit.albedo;
		}
	}

	Vector3 unit_direction = Normalize(r.GetDirection()); // the unit vector (magnitude == 1) of the rays direction
	auto t = 0.5 * (unit_direction.y + 1.0); // make t 0 -> 1
	
	// This is opposite RaytracingInAWeekend, we instead flip the values to lerp to/from because of reversed y UV system
	return Utils::Lerp(Vector3<Float>(0.5, 0.7, 1.0), Vector3<Float>(1.0), t);
}

uint32_t Renderer::PerPixel(Vector2<Float>&& coord)
{
	const Ray<Float> r = Ray(m_Camera.GetOrigin(), m_Camera.GetLowerLeftCorner() + coord.x * m_Camera.GetHorizontal() + coord.y * m_Camera.GetVertical() - m_Camera.GetOrigin());

	return 0xff000000;
	/*
	constexpr Vector3<Float> sphere_origin = Vector3<Float>(0, 0, 0);
	Float radius = 0.5;
	Vector3<Float> oc = r.GetOrigin() - sphere_origin; // origin of ray - origin of sphere
	auto a = Dot(r.GetDirection(), r.GetDirection()); // square the direction
	auto b = 2.0 * Dot(oc, r.GetDirection());
	auto c = Dot(oc, oc) - radius * radius;
	auto discriminant = b * b - 4.0 * a * c; // quadratic formula
	if (discriminant >= 0.0) // if we hit the sphere
	{
		double t0 = (-b - sqrt(discriminant)) / (2.0 * a); // CLOSEST T (SMALLEST)
		double t1 = (-b + sqrt(discriminant)) / (2.0 * a); // second "hit' is the ray leaving the object, in our case a sphere

		Vector3<Float> h0 = r.At(t0); // CLOSEST HITPOINT (SMALLEST)
		Vector3<Float> h1 = r.At(t1);
		//if (h0 > h1) std::swap(h0, h1);

		const Vector3<Float> normal = Normalize(h0 - sphere_origin);
		Vector3<Float> light_dir = Normalize(Vector3<Float>(-1, 1, 1));
		Float light_intensity = Utils::Max(Dot(normal, -light_dir), 0.0); // == cos(angle)
		auto color = light_intensity * Vector3<Float>(1.0, 0.5, 0.4);
		color = Utils::Clamp(color, Vector3(0.0), Vector3(1.0));
		return Utils::VectorToUInt32(color);
	}

	// we only reach here if we didn't hit the sphere
	Vector3 unit_direction = Normalize(r.GetDirection()); // the unit vector (magnitude == 1) of the rays direction
	auto t = 0.5 * (unit_direction.y + 1.0); // make t 0 -> 1
	return Utils::VectorToUInt32(Utils::Lerp(Vector3<Float>(1.0), Vector3<Float>(0.5, 0.7, 1.0), t)); // lerp the values to white -> light blue based on y coord
	*/
}
