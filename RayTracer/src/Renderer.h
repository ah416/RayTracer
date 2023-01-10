#pragma once

#include "RayTracer.h"

#include "Image.h"
#include "Camera.h"
#include "Vector.h"
#include "Scene.h"

struct RenderSettings
{
	uint32_t NumberOfSamples = 1;
	uint32_t NumberOfBounces = 1;
	bool Accumulate = true;
	uint32_t AccumulateMax = 1;
};

class Renderer
{
public:
	Renderer() = default;
	~Renderer() { delete[] m_AccumulationData; }

	void Render(const Scene& scene, const Camera& cam);

	constexpr void SetSettings(const RenderSettings&& settings) { m_Settings = settings; }

	void SetImage(Image& image);

private:
	Vec3f PerPixel(const Vec2f&& coord); // comparable to RayGen shader in GPU ray tracing

	HitPayload TraceRay(const Ray<Float>& ray);

	HitPayload ClosestHit(const Ray<Float>& ray, Float hitDistance, int objectIndex);

	constexpr HitPayload Miss(const Ray<Float>& ray);

	Image* m_Image = nullptr;
	Vec3f* m_AccumulationData = nullptr;

	std::vector<uint32_t> m_ImageVerticalIter;
	std::vector<uint32_t> m_ImageHorizontalIter;

	const Camera* m_Camera = nullptr;
	const Scene* m_Scene = nullptr;

	RenderSettings m_Settings;
};