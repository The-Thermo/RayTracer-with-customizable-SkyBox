#pragma once
#include "Walnut/Image.h"
#include <memory>
#include <glm/glm.hpp>
#include "Camera.h"
#include "Ray.h"
#include "Scene.h"
#include <vector>


class Renderer
{
public:
	struct Settings
	{
		bool Accumulate = true;
	};
public: 
	Renderer() = default; 


	void OnResize(uint32_t width, uint32_t height);
	void Render(const Scene& scene, const Camera& camera); //using camera as parameters


	std::shared_ptr<Walnut::Image> GetFinalImage() { return m_FinalImage; }

	void ResetFrameIndex() { m_frameIndex = 1; }
	Settings& GetSettings() { return m_Settings; }
private:
	struct HitPayLoad
	{
		float HitDistance;
		glm::vec3 WorldPosition;
		glm::vec3 WorldNormal;

		int objectIndex;
	};
	glm::vec4 PerPixel(uint32_t x, uint32_t y); //RayGen

	HitPayLoad TraceRay(const Ray& ray);
	HitPayLoad ClosestHit(const Ray& ray, float HitDistance, int objectIndex);
	HitPayLoad Miss(const Ray& ray);
private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	Settings m_Settings;

	std::vector<uint32_t> m_ImageHorizontalIter, m_ImageVerticalIter;

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;

	uint32_t* m_ImageData = nullptr;
	glm::vec4* m_AccumulationData = nullptr;

	uint32_t m_frameIndex = 1; //every frame sample (gets divided)
};

