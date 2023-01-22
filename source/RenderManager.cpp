#include "pch.h"
#include "RenderManager.h"
#include "Utils.h"
#include "DataTypes.h"

namespace dae
{

	RenderManager::RenderManager(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize Camera
		m_pCamera = new Camera();

		int width{}, height{};
		SDL_GetWindowSize(pWindow, &width, &height);
		float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

		m_pCamera->Initialize(aspectRatio, 45.f, { .0f, 0.f, 0.f });

		//load needed textures
		std::vector<Texture*> pTextures{};
		Texture* pDiffuse = Texture::LoadFromFile("Resources/vehicle_diffuse.png");
		Texture* pNormal = Texture::LoadFromFile("Resources/vehicle_normal.png");
		Texture* pSpecular = Texture::LoadFromFile("Resources/vehicle_specular.png");
		Texture* pGloss = Texture::LoadFromFile("Resources/vehicle_gloss.png");
		pTextures.push_back(pDiffuse);
		pTextures.push_back(pNormal);
		pTextures.push_back(pSpecular);
		pTextures.push_back(pGloss);

		//Create the different renderers
		m_pRendererSoftware = new SoftwareRenderer(pWindow, m_pCamera, pTextures);
		m_pRendererHardware = new HardwareRenderer(pWindow, m_pCamera);
		m_pCurrentRenderer = m_pRendererHardware;

		//Initialize objects (now that we have devices)

		std::vector<Vertex_PosTex> vertices{ };
		std::vector<uint32_t> indices{ };

		Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);

		const Vector3 position{ Vector3{0.f, 0.f, 50.f} };
		const Vector3 rotation{ };
		const Vector3 scale{ Vector3{ 1.f, 1.f, 1.f } };
		Matrix worldMatrix = Matrix::CreateScale(scale) * Matrix::CreateRotation(rotation) * Matrix::CreateTranslation(position);

		m_pSoftwareMesh = new Mesh_PosTexSoftwareVehicle(vertices, indices, worldMatrix, PrimitiveTopology::TriangleList);

		m_pRendererSoftware->SetMesh(m_pSoftwareMesh);

		std::vector<Texture*> pHardwareTextures{};

		Texture* pHardwareDiffuse = Texture::LoadFromFile("Resources/vehicle_diffuse.png", m_pRendererHardware->GetDevice());
		Texture* pHardwareNormal = Texture::LoadFromFile("Resources/vehicle_normal.png", m_pRendererHardware->GetDevice());
		Texture* pHardwareSpecular = Texture::LoadFromFile("Resources/vehicle_specular.png", m_pRendererHardware->GetDevice());
		Texture* pHardwareGloss = Texture::LoadFromFile("Resources/vehicle_gloss.png", m_pRendererHardware->GetDevice());
		pHardwareTextures.push_back(pHardwareDiffuse);
		pHardwareTextures.push_back(pHardwareNormal);
		pHardwareTextures.push_back(pHardwareSpecular);
		pHardwareTextures.push_back(pHardwareGloss);
		Texture* pFireDiffuse = Texture::LoadFromFile("Resources/fireFX_diffuse.png", m_pRendererHardware->GetDevice());
		m_pRendererHardware->SetTextures(pTextures, pFireDiffuse);

		m_pHardwareMesh = new Mesh_PosTexVehicle(m_pRendererHardware->GetDevice(), vertices, indices, worldMatrix, pHardwareTextures);

		m_pRendererHardware->SetMesh(m_pHardwareMesh);

		Utils::ParseOBJ("Resources/fireFX.obj", vertices, indices);
		m_pFire = new Mesh_PosTexFire(m_pRendererHardware->GetDevice(), vertices, indices, worldMatrix, pFireDiffuse);

		m_pRendererHardware->SetFire(m_pFire);
	}

	RenderManager::~RenderManager()
	{
		delete m_pFire;
		m_pFire = nullptr;

		delete m_pHardwareMesh;
		m_pHardwareMesh = nullptr;

		delete m_pSoftwareMesh;
		m_pSoftwareMesh = nullptr;

		delete m_pCamera;
		m_pCamera = nullptr;

		delete m_pRendererSoftware;
		m_pRendererSoftware = nullptr;
	}

	void RenderManager::Update(const Timer* pTimer)
	{
		//Check if the current Renderer is not a nullptr
		m_pRendererHardware->Update(pTimer);
		m_pRendererSoftware->Update(pTimer);
	}


	void RenderManager::Render() const
	{
		//Check if the current Renderer is not a nullptr
		if (m_pCurrentRenderer) {
			m_pCurrentRenderer->Render();
		}
	}
	void RenderManager::ToggleRenderType()
	{
		switch (m_CurrentRenderType)
		{
		case RenderType::Software:
			m_pCurrentRenderer = m_pRendererHardware;
			m_CurrentRenderType = RenderType::Hardware;
			std::cout << "Switched to hardware renderer" << std::endl;
			break;
		case RenderType::Hardware:
			m_pCurrentRenderer = m_pRendererSoftware;
			m_CurrentRenderType = RenderType::Software;
			std::cout << "Switched to software renderer" << std::endl;
			break;
		}
	}

	void RenderManager::CycleRotation()
	{
		m_pRendererHardware->CycleRotation();
		m_pRendererSoftware->CycleRotation();
	}
}