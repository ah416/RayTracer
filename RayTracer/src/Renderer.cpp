#include "Renderer.h"

#include "Ray.h"
#include "Vector.h"

#include <future>
#include <execution>
#include <array>

void Renderer::Render(const Scene& scene, const Camera& cam)
{
	m_Camera = &cam;
	m_Scene = &scene;

	if (!m_Settings.Accumulate)
		m_Settings.AccumulateMax = 1;

#define MT 1

#if MT
#if RT_WINDOWS

	// This doesn't work on linux apparently...
	std::for_each(std::execution::par, m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(), [this](uint32_t y)
		{ std::for_each(std::execution::par, m_ImageHorizontalIter.begin(), m_ImageHorizontalIter.end(), [this, y](uint32_t x)
			{
				for (int i = 0; i < m_Settings.AccumulateMax; i++)
				{
					auto color = PerPixel(Vec2f((Float)x, (Float)y));
					m_AccumulationData[x + y * m_Image->Width] += color;
				}
	auto accumulated_color = m_AccumulationData[x + y * m_Image->Width];
	accumulated_color /= (Float)m_Settings.AccumulateMax;

	m_Image->Data[x + y * m_Image->Width] = Utils::VectorToUInt32(accumulated_color); }); });

#else // RT_WINDOWS
#define NUM_THREADS 8

	struct ImgBlock
	{
		uint32_t start_w, start_h, end_w, end_h;
	};
	std::vector<ImgBlock> blocks;
	/*
	 * Following section from https://superkogito.github.io/blog/2020/10/01/divide_image_using_opencv.html
	 * Adapted by ah416 for use without opencv
	 */

	 // init image dimensions
	uint32_t imgWidth = this->m_Image->Width;
	uint32_t imgHeight = this->m_Image->Height;

	std::cout << "IMAGE SIZE: "
		<< "(" << imgWidth << "," << imgHeight << ")" << std::endl;

	// init block dimensions
	uint32_t bwSize;
	uint32_t bhSize;

	uint32_t blockWidth = imgWidth / (NUM_THREADS / 2);
	uint32_t blockHeight = imgHeight / (2);

	uint32_t y0 = 0;
	while (y0 < imgHeight)
	{
		// compute the block height
		bhSize = ((y0 + blockHeight) > imgHeight) * (blockHeight - (y0 + blockHeight - imgHeight)) + ((y0 + blockHeight) <= imgHeight) * blockHeight;
		int x0 = 0;
		while (x0 < imgWidth)
		{
			// compute the block height
			bwSize = ((x0 + blockWidth) > imgWidth) * (blockWidth - (x0 + blockWidth - imgWidth)) + ((x0 + blockWidth) <= imgWidth) * blockWidth;

			// crop block
			blocks.push_back(ImgBlock(x0, y0, x0 + bwSize, y0 + bhSize));

			// update x-coordinate
			x0 = x0 + blockWidth;
		}

		// update y-coordinate
		y0 = y0 + blockHeight;
	}

	std::array<std::future<void>, NUM_THREADS> threads;
	for (int i = 0; i < NUM_THREADS; i++)
	{
		threads[i] = std::async(
			std::launch::async, [&](int i)
			{
				uint32_t start_w = blocks[i].start_w, start_h = blocks[i].start_h, end_w = blocks[i].end_w, end_h = blocks[i].end_h;
		printf("start: (%u, %u), end: (%u, %u)\n", start_w, start_h, end_w, end_h);

		for (int y = start_h; y < end_h; y++)
		{
			for (int x = start_w; x < end_w; x++)
			{
				for (int i = 0; i < m_Settings.AccumulateMax; i++)
				{
					auto color = PerPixel({ (Float)x, (Float)y });
					m_AccumulationData[x + y * this->m_Image->Width] += color;
				}
				auto accumulated_color = m_AccumulationData[x + y * this->m_Image->Width];
				accumulated_color /= (Float)m_Settings.AccumulateMax;

				m_Image->Data[x + y * this->m_Image->Width] = Utils::VectorToUInt32(accumulated_color);
			}
		} },
			i);
	}

	for (int i = 0; i < NUM_THREADS; i++)
	{
		while (!threads[i].valid())
		{
		}
	}

#endif // RT_WINDOWS

#else // MT

	for (int y = 0; y < (int)m_Image->Height; y++)
	{
		for (int x = 0; x < (int)m_Image->Width; x++)
		{
			for (int i = 0; i < m_Settings.AccumulateMax; i++)
			{
				auto color = PerPixel({ (Float)x, (Float)y });
				m_AccumulationData[x + y * m_Image->Width] += color;
			}
			auto accumulated_color = m_AccumulationData[x + y * m_Image->Width];
			accumulated_color /= (Float)m_Settings.AccumulateMax;

			m_Image->Data[x + y * m_Image->Width] = Utils::VectorToUInt32(accumulated_color);
		}
	}

#endif // MT
}

void Renderer::SetImage(Image& image)
{
	m_Image = &image;
	delete[] m_AccumulationData;
	m_AccumulationData = new Vec3f[m_Image->Width * m_Image->Height];

	m_ImageVerticalIter.resize(m_Image->Height);
	m_ImageHorizontalIter.resize(m_Image->Width);

	for (uint32_t x = 0; x < m_Image->Height; x++)
		m_ImageVerticalIter[x] = x;

	for (uint32_t y = 0; y < m_Image->Width; y++)
		m_ImageHorizontalIter[y] = y;
}

Vec3f Renderer::PerPixel(const Vec2f&& coord)
{
	Vec3f res = 0.0;
	for (int i = 0; i < m_Settings.NumberOfSamples; i++)
	{
		Float u = Float(coord.x + Utils::RandomFloat()) / (m_Image->Width - 1);	 // transform the x coordinate to 0 -> 1 (rather than 0 -> image_width)
		Float v = Float(coord.y + Utils::RandomFloat()) / (m_Image->Height - 1); // transform the y coordinate to 0 -> 1 (rather than 0 -> image_height)
		// auto u = Float(coord.x) / (m_Image->Width - 1);
		// auto v = Float(coord.y) / (m_Image->Height - 1);
		Ray<Float> r = Ray(m_Camera->GetOrigin(), m_Camera->CalculateRayDirection({ u, v }));

		auto multiplier = 1.0;
		for (int i = 0; i < m_Settings.NumberOfBounces; i++)
		{
			auto payload = TraceRay(r);
			if (payload.HitDistance < 0)
			{
				Vector3 unit_direction = Normalize(r.Direction); // the unit vector (magnitude == 1) of the rays direction
				auto t = 0.5 * (unit_direction.y + 1.0);		 // make t 0 -> 1

				res += Utils::Lerp(Vec3f(1.0), Vec3f(0.5, 0.7, 1.0), t) * multiplier;

				continue;
			}

			const Shape* shape = m_Scene->Shapes[payload.ObjectIndex];
			const Material& material = m_Scene->Materials[shape->MaterialIndex];

			Vec3f light_dir = Normalize(Vec3f(0, -1, -2));
			Float light_intensity = Utils::Max(Dot(payload.WorldNormal, -light_dir), 0.0); // == cos(angle)

			auto shape_color = material.Albedo;
			shape_color *= light_intensity;

			res += Utils::Clamp(shape_color * multiplier, Vec3f(0.0), Vec3f(1.0));

			multiplier *= 0.5;

			r.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001;
			r.Direction = Reflect(r.Direction, payload.WorldNormal + material.Roughness * Utils::RandomVector(-0.5, 0.5));
		}
	}

	return res / (m_Settings.NumberOfSamples * m_Settings.NumberOfBounces);
}

HitPayload Renderer::TraceRay(const Ray<Float>& ray)
{
	int objectIndex = -1;
	Float hitDistance = std::numeric_limits<Float>::max();

	for (int i = 0; i < m_Scene->Shapes.size(); i++)
	{
		const Shape* shape = m_Scene->Shapes[i];
		Float newDistance = 0;

		if (shape->Hit(ray, 0, hitDistance, newDistance))
		{
			if (newDistance > 0.0 && newDistance < hitDistance)
			{
				hitDistance = newDistance;
				objectIndex = i;
			}
		}
	}

	if (objectIndex < 0)
		return Miss(ray);

	return ClosestHit(ray, hitDistance, objectIndex);
}

constexpr HitPayload Renderer::ClosestHit(const Ray<Float>& ray, Float hitDistance, int objectIndex)
{
	HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	const Shape* closestShape = m_Scene->Shapes[objectIndex];

	Vec3f origin = ray.Origin - closestShape->Origin;
	payload.WorldPosition = origin + hitDistance * ray.Direction;
	payload.WorldNormal = Normalize(payload.WorldPosition);
	payload.WorldPosition += closestShape->Origin;

	return payload;
}

constexpr HitPayload Renderer::Miss(const Ray<Float>& ray)
{
	constexpr HitPayload payload = { .HitDistance = -1 };
	return payload;
}
