#include "pch.h"
#include "HardwareRenderer.h"
#include "Utils.h"


namespace dae
{
	HardwareRenderer::HardwareRenderer(SDL_Window* pWindow, Camera* pCamera) 
		:BaseRenderer(pWindow, pCamera)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}
	}

	HardwareRenderer::~HardwareRenderer()
	{
		delete m_pTexture;
		m_pTexture = nullptr;
		delete m_pNormalMap;
		m_pNormalMap = nullptr;
		delete m_pSpecularMap;
		m_pSpecularMap = nullptr;
		delete m_pGlossMap;
		m_pGlossMap = nullptr;

		delete m_pFireTexture;
		m_pFireTexture = nullptr;

		delete m_pFire;
		m_pFire = nullptr;

		delete m_pMesh;
		m_pMesh = nullptr;

		if (m_pRenderTargetView)
		{
			m_pRenderTargetView->Release();
		}

		if (m_pRenderTargetBuffer)
		{
			m_pRenderTargetBuffer->Release();
		}

		if (m_pDepthStencilView)
		{
			m_pDepthStencilView->Release();
		}

		if (m_pDepthStencilBuffer)
		{
			m_pDepthStencilBuffer->Release();
		}

		if (m_pSwapChain)
		{
			m_pSwapChain->Release();
		}

		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}

		if (m_pDevice)
		{
			m_pDevice->Release();
		}
	}

	void HardwareRenderer::Update(const Timer* pTimer)
	{
		m_pCamera->Update(pTimer);
		
		const float rotationSpeed{ PI/4 }; //45 degrees / sec
		if (m_IsRotating)
		{
			m_pMesh->m_WorldMatrix = Matrix::CreateRotationY(rotationSpeed * pTimer->GetElapsed()) * m_pMesh->m_WorldMatrix;
			m_pFire->m_WorldMatrix = Matrix::CreateRotationY(rotationSpeed * pTimer->GetElapsed()) * m_pFire->m_WorldMatrix;
		}
	}

	void HardwareRenderer::Render()
	{
		if (!m_IsInitialized)
			return;

		//1. Clear RTV & DSV
		if (m_ShouldUseUniformColor)
		{
			ColorRGB clearColor{ m_UniformColor };
			m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &m_UniformColor.r);
		}
		else
		{
			ColorRGB clearColor{ .39f, .59f, .93f };
			m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		}
		
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		//2. Set Pipeline & invoke DrawCalls (= render)
		m_pMesh->Render(m_pDeviceContext, *m_pCamera);
		if (m_RenderFire)
		{
			m_pFire->Render(m_pDeviceContext, *m_pCamera);
		}

		
		//3. Present BackBuffer (swap)
		m_pSwapChain->Present(0, 0);
	}

	HRESULT HardwareRenderer::InitializeDirectX()
	{
		//1. Create Device and DeviceContext
		//========================================================
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;

	#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel,
											1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);

		if (FAILED(result)) return result;

		//Create DXGI Factory
		IDXGIFactory1* pDxgiFactory{};
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));

		if (FAILED(result)) return result;

		//2. Create Swapchain
		//=======================================================
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		//Get the handle (HWND) from the SDL Backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_VERSION(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		//Create Swapchain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result)) return result;

		//3. Create DepthStencil (DS) & DepthStencilView (DSV)
		//===================================================================================
		//Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result)) return result;

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		
		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result)) return result;

		//4. Create RenderTarget & RenderTargetView
		//===================================================================
		//Resource
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));

		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result)) return result;

		//5. Build RTV & DSV to Output Merger State
		//====================================================================
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//6. Set Viewport
		//====================================================================
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewport);

		pDxgiFactory->Release();
	}

	void HardwareRenderer::CycleSamplerState()
	{
		m_pMesh->CycleSamplerState();
		m_pFire->CycleSamplerState();
	}

	void HardwareRenderer::CycleCullingMode()
	{
		m_pMesh->CycleCullingMode();
	}

	void HardwareRenderer::ToggleFireFX()
	{
		m_RenderFire = !m_RenderFire;
	}
}