#include "Renderer.h"
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Renderer.h"
#include "Walnut/Image.h"
#include <Walnut/Timer.h>
#include "Camera.h"
#include <glm/gtc/type_ptr.hpp>



using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer() :m_Camera(45.0f, 0.1f, 100.0f) { //the vield of view for out camera (45 degrees is the phi in vertical FOV and 0.1 is the near clip and 100 is the far clip)

		
		Skybox& skybox = m_Scene.Skyboxes.emplace_back();
		skybox.Albedo = { 1.0f, 0.0f, 1.0f };

		Material& pinkSphere = m_Scene.Materials.emplace_back();
		pinkSphere.Albedo = { 1.0f, 0.0f, 1.0f };
		pinkSphere.Roughness = 0.0f;

		Material& blueSphere = m_Scene.Materials.emplace_back();
		blueSphere.Albedo = { 0.3f, 0.2f, 1.0f };
		blueSphere.Roughness = 0.1f;

		Material& orangeSphere = m_Scene.Materials.emplace_back();
		orangeSphere.Albedo = { 0.8f, 0.5f, 0.2f };
		orangeSphere.Roughness = 0.1f;
		orangeSphere.EmissionColor = orangeSphere.Albedo;
		orangeSphere.EmissionPower = 2.0f;



		{
			Sphere sphere;
			sphere.position = { 0.0f, 0.0f, 0.0f };
			sphere.Radius = 1.0f;
			sphere.materialIndex = 0;
			m_Scene.Spheres.push_back(sphere);
		}

	
		{
			Sphere sphere;
			sphere.position = { 2.0f, 0.0f, 0.0f };
			sphere.Radius = 1.0f;
			sphere.materialIndex = 2;
			m_Scene.Spheres.push_back(sphere);
		}

		//multiple spheres
		{
			Sphere sphere;
			sphere.position = { 0.0f, -101.0f, 0.0f };
			sphere.Radius = 100.0f;
			sphere.materialIndex = 1;
			m_Scene.Spheres.push_back(sphere);
		}

		
		//m_Scene.Spheres.push_back(Sphere{ { 0.0f, 0.0f, 0.0f }, 0.5f, { 1.0f, 0.0f, 1.0f } }); //pushes in the position, radius, and color of the sphere
	} 
	
	virtual void OnUpdate(float ts) override
	{
		if (m_Camera.OnUpdate(ts))
			m_Renderer.ResetFrameIndex();
	}
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last Render: %3fms", m_LastRenderTime); //to display render time
		if (ImGui::Button("Render"))
		{
			Render();
		}

		ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().Accumulate);

		if (ImGui::Button("Reset"))
		{
			m_Renderer.ResetFrameIndex();
		}

		ImGui::End(); //only call this at the end of all ImGui button methods have been implements (end of stack)
		ImGui::Begin("Scene"); //scene panel
		//gives controls of position, radius, and color (albedo)
		for (size_t i = 0; i < m_Scene.Spheres.size(); i++)
		{
			ImGui::PushID(i); //The controls need to be seperate for each sphere so we must push the ID for each sphere
			Sphere& sphere = m_Scene.Spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.position), 0.1f);
			ImGui::DragFloat3("Radius", &sphere.Radius, 0.1f);
			ImGui::DragInt("Material", &sphere.materialIndex, 1.0f, 0, (int)m_Scene.Materials.size() - 1);

			ImGui::Separator();//adds a seperating line for each controls of the sphere

			ImGui::PopID(); //pops the ID (doesn't need argument i)
		}

		for (size_t i = 0; i < m_Scene.Materials.size(); i++)
		{
			ImGui::PushID(i);
			Material& material = m_Scene.Materials[i];
			ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo), 0.1f);
			ImGui::DragFloat("Roughness", &material.Roughness, 0.05f, 0.0f, 1.0f);
			ImGui::DragFloat("Metallic", &material.Metallic, 0.05f, 0.0f, 1.0f);
			ImGui::ColorEdit3("Emission Color", glm::value_ptr(material.EmissionColor));
			ImGui::DragFloat("Emission Power", &material.EmissionPower, 0.05f, 0.0f, FLT_MAX);

			ImGui::Separator();
			ImGui::PopID();
		}
		if (ImGui::CollapsingHeader("Skybox"))
		{
			ImGui::ColorEdit3("Sky Color", glm::value_ptr(m_Scene.skybox.Skycolor));
		}
		
		for (size_t i = 0; i < m_Scene.Skyboxes.size(); i++)
		{
			ImGui::PushID(i);
			Skybox& skybox = m_Scene.Skyboxes[i];
			ImGui::ColorEdit3("Sky Color", glm::value_ptr(skybox.Skycolor));

			ImGui::Separator();
			ImGui::PopID();
		}
		ImGui::End();

		ImGui::ShowDemoWindow();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		auto image = m_Renderer.GetFinalImage();

		if (image)
			ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() },
				ImVec2(0, 1), ImVec2(1, 0)); //reversed the y axis to match the image
											//the uv parameter is reversed to match the image

		ImGui::End();
		ImGui::PopStyleVar();

		Render(); //putting the render function here makes it render real time
		//since its not in an if statement for a button, it has free range to render whenever
	}

	void Render()
	{
		Timer timer;

		//renderer resize
		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight); //to resize the camera viewport
		//renderer render
		m_Renderer.Render(m_Scene, m_Camera);



		m_LastRenderTime = timer.ElapsedMillis();
	}




private:
	Scene m_Scene;
	Camera m_Camera; //camera object
	Renderer m_Renderer;
	uint32_t m_ViewportHeight = 0, m_ViewportWidth = 0;
	float m_LastRenderTime = 0;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Walnut Example";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit"))
				{
					app->Close();
				}
				ImGui::EndMenu();
			}
		});
	return app;
}