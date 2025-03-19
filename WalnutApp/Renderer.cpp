#include "Renderer.h"
#include <Walnut/Random.h> //for static Image
#include <execution>
#include <vector>
#include <algorithm>

namespace Utils {

	static uint32_t ConvertToRGA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f); //for red channel
		uint8_t g = (uint8_t)(color.g * 255.0f); //for green channel
		uint8_t b = (uint8_t)(color.b * 255.0f); //for blue channel
		uint8_t a = (uint8_t)(color.a * 255.0f); //for alpha channel
		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}

}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage) 
	{
		//no resize necessary
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;

		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}
	
	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];

	delete[] m_AccumulationData;
	m_AccumulationData = new glm::vec4[width * height];

	m_ImageHorizontalIter.resize(width);
	m_ImageVerticalIter.resize(height);
	for (uint32_t i = 0; i < width; i++)
		m_ImageHorizontalIter[i] = i;

	for (uint32_t i = 0; i < height; i++)
		m_ImageVerticalIter[i] = i;
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	//const glm::vec3& rayOrigin = camera.GetPosition(); //the camera is at the origin (refactored for PerPixel method)
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	if (m_frameIndex == 1)
		memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));


#define MT 0 //for defining multi-threaded
#if MT

	std::for_each(std::execution::par, m_ImageVerticalIter.begin(), m_ImageHorizontalIter.end(),
		[this](uint32_t y)
		{
			std::for_each(std::execution::par, m_ImageVerticalIter.begin(), m_ImageHorizontalIter.end(),
				[this, y](uint32_t x)
				{
					//glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(), (float)y / (float)m_FinalImage->GetHeight() }; //(this isnt needed since we already calculated
			// inside the ray direction)
			//coord = coord * 2.0f - 1.0f; //convert to -1 to 1 range //refactored for perpixel method

			//glm::vec4 color = PerPixel(coord); //the camerra ray directions refactor this parameter of a coord, so we can remove it

					glm::vec4 color = PerPixel(x, y);
					m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

					glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
					accumulatedColor /= (float)m_frameIndex;

					accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f)); //clamp the color to 0 to 1 range (needs to be 0 for the first parameter and 1 for the 2nd)
					m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGA(accumulatedColor);

					//m_ImageData[i] = Walnut::Random::UInt(); //for random colored screen
					//m_ImageData[i] = 0xff48c650; //for a solid colored screen
				});
		});
#else
	//renders every pixel
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) //go y coord first because its
															//easier on the cpu caches memory to allocate y first then x.
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec4 color = PerPixel(x, y);
			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

			glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
			accumulatedColor /= (float)m_frameIndex;

			accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f)); //clamp the color to 0 to 1 range (needs to be 0 for the first parameter and 1 for the 2nd)
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGA(accumulatedColor);
		}
		
	}
#endif

	m_FinalImage->SetData(m_ImageData);

	if (m_Settings.Accumulate)
		m_frameIndex++;
	else
		m_frameIndex = 1;
}

glm::vec4 Renderer::PerPixel(uint32_t x,uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	glm::vec3 light(0.0f);

	glm::vec3 contribution(1.0f);


	int bounces = 5;
	for (int i = 0; i < bounces; i++)
	{
		Renderer::HitPayLoad payload = TraceRay(ray); //send out as many rays as we want per pixel
		if (payload.HitDistance < 0.0f)
		{
			const Skybox& skybox = m_ActiveScene->Skyboxes[0]; 
			light += skybox.GetSkyColor() * contribution;
			break; //so we don't keep tracing rays
		}
		//glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1)); //this is the direction of the light
		//float lightIntensity = glm::max(glm::dot(payload.WorldNormal, -lightDir), 0.0f); //==cos(angle)
		//if you dot product the light direction and normal, and try to output the sphere color, it will be black.
		//Because the dot product is negative, we need to clamp it to 0 to 1 range by using glm::max

		const Sphere& sphere = m_ActiveScene->Spheres[payload.objectIndex];
		const Material& material = m_ActiveScene->Materials[sphere.materialIndex];

		//glm::vec3 sphereColor = material.Albedo; //(1, 0, 1); //this is the color of the sphere //don't need this after setting up the sphere struct in Scene.h
		//sphereColor *= lightIntensity; //this is the dot product of the normal and the light direction

		light += material.GetEmission();

		contribution *= material.Albedo; //so the light from rays dont get brighter each time

		ray.Origin = payload.WorldPosition + payload.WorldNormal + 0.0001f;
		//ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5f, 0.5f));
		ray.Direction = glm::normalize(payload.WorldNormal + Walnut::Random::InUnitSphere()); //this is for randomizing the direction of the ray

		//sphereColor = normal * 0.5f + 0.5f; //vectors are being hit and reflected off the surface (normals to the surface hitpoints)
		//because the normal is a unit vector that shoots out to -1 to 1, 
		//we need to convert it to 0 to 1 range by multiplying by 0.5 and adding 0.5 to see the the full spectrum of colors since we set the
		//range to be 0 to 1 in line 49
	}
	return glm::vec4(light, 1.0f); //this returns the color of the sphere
	//0xff000000 | (g << 8) | r; //for the same fragment shader on shadertoy (0xff00000 returns black)
	// return glm::vec4(1, 0, 0, 1); //this returns red
}

//uint32_t Renderer::PerPixel(glm::vec2 coord)
Renderer::HitPayLoad Renderer::TraceRay(const Ray& ray) //this is so we can see the range of vectors being hit on the surface
 
{
	//glm::vec3 rayOrigin(0.0f); //the camera is at the origin
	//glm::vec3 rayOrigin(0.0f, 0.0f, 1.0f); //the camera is moved back 2 units
	//glm::vec3 rayDirection(coord.x, coord.y, -1.0f);//the z coord is -1 because the camera is looking down the negative z axis
	//rayDirection = glm::normal(rayDirection);
	//(bx^2 + by^2)t^2 + (2(axbx +ayby)t + (ax^2 + ay^2)) = 0
	//where
	//a = ray origin
	//b = ray direction
	//r = radius of sphere
	//t = hit distance

	int closestSphere = -1; //for if there are multiple spheres in the scene
	float hitDistance = std::numeric_limits<float>::max(); //(maximum float value) //for if there are multiple spheres in the scene

	for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i++)
	{
		const Sphere& sphere = m_ActiveScene->Spheres[i];
		glm::vec3 origin = ray.Origin - sphere.position; //glm::vec3(-0.5f, 0.0f, 0.0f); //moves the camera to the left 0.5 units
		//and replaces ray.Origin with just origin

		//float radius = 0.5f; //this is the radius of the sphere //don't need this after setting up the sphere struct in Scene.h
		float a = glm::dot(ray.Direction, ray.Direction); //this is the a term in the quadratic equation
		float b = 2.0f * glm::dot(origin, ray.Direction); //this is the b term in the quadratic equation
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius; //this is the c term in the quadratic equation
		float discriminant = b * b - 4.0f * a * c; //this is the discriminant of the quadratic equation (b^2 - 4ac)


		//the rest of the quadratic formula (-b+- sqrt(b^2 - 4ac)) / 2a

		if (discriminant < 0.0f) //if the discriminant is greater than or equal to 0, then the ray intersects the sphere

			continue;  //return glm::vec4(0, 0, 0, 1); //this returns black if the ray does not intersect the sphere

		//float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a); //this is the first hit distance (currently used)
		float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a); //this is the second hit distance
		if (closestT > 0.0f && closestT < hitDistance) //ignore results that are negative as the object is in front, not behind
		{
			hitDistance = closestT;
			closestSphere = (int)i; //since the closest sphere would have the closest hit distance (smallest value)
		}

		//this is the dot product of the ray direction with itself which can be written as
		//rayDirection.x * rayDirection.x + rayDirection.y * rayDirection.y + rayDirection.z * rayDirection.z;

		//uint8_t r = (uint8_t)(coord.x * 255.0f); //the red channel is x coord
		//uint8_t g = (uint8_t)(coord.y * 255.0f); //the green channel is y coord

		//we can use glm libraries api instead of declaring a float for hit position
		//hit position

		//glm::vec3 h0 = rayOrigin + rayDirection * t0; //this is the (ax + bx * t) that we first initialized as a vector (start of sphere)
		//glm::vec3 h1 = rayOrigin + rayDirection * t1; //(end of sphere)
	}

	if (closestSphere < 0)
	{
		return Miss(ray); //this returns black if there is no sphere in the scene
	}

	return ClosestHit(ray, hitDistance, closestSphere);

	//if (scene.Spheres.size() == 0) //for if there are no spheres in the scene
		//return glm::vec4(0, 0, 0, 1); 

	const Sphere& sphere = m_ActiveScene->Spheres[0]; //for if there is only one sphere in the scene
}

Renderer::HitPayLoad Renderer::ClosestHit(const Ray& ray, float HitDistance, int objectIndex)
{
	Renderer::HitPayLoad payload;
	payload.HitDistance = HitDistance;
	payload.objectIndex = objectIndex;


	const Sphere& closestSphere = m_ActiveScene->Spheres[objectIndex];

	glm::vec3 origin = ray.Origin - closestSphere.position;
	payload.WorldPosition = origin + ray.Direction * HitDistance;
	payload.WorldNormal = glm::normalize(payload.WorldPosition); //this is the normal of the sphere

	payload.WorldPosition += closestSphere.position; //add back the world position

	return payload;
}

Renderer::HitPayLoad Renderer::Miss(const Ray& ray)
{
	Renderer::HitPayLoad payload;
	payload.HitDistance = -1.0f;
	return payload;
}
