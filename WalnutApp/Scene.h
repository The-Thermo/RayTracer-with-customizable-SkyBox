#pragma once
//making a scene for multipple objects
#include <glm/glm.hpp>
#include <vector>

struct Material
{
	glm::vec3 Albedo{ 1.0f }; //nonlit diffuse part of a material is an Albedo //where 1.0 is white
	float Roughness = 1.0f;
	float Metallic = 0.0f;
	glm::vec3 EmissionColor{ 0.0f };
	float EmissionPower = 0;

	glm::vec3 GetEmission() const { return EmissionColor * EmissionColor; }
};

struct Sphere
{
	glm::vec3 position; //position in 3d space
	float Radius = 0.0f;

	int materialIndex = 0; //so all materials can use the single material
};

struct Skybox
{
	glm::vec3 Albedo{ 1.0f };
	glm::vec3 Skycolor{ 0.0f }; //default color of the skybox

	glm::vec3 GetSkyColor() const { return Skycolor * Skycolor; }

	int skyIndex = 0;
};

struct Scene
{
	std::vector<Sphere> Spheres;
	std::vector<Material> Materials;
	Skybox skybox;
	std::vector<Skybox> Skyboxes;
};