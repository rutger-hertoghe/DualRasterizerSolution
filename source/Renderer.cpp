#include "pch.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Structs.h"
#include "Camera.h"
#include "ConsoleColorCtrl.h"
#include "Effect.h"
#include "Effect_PartCov.h"
#include "Effect_PosTex.h"
#include "Effect_PosCol.h"	
#include "Texture.h"
#include "Utils.h"

namespace dae
{
	//--------------------------------------
	// DUAL RASTERIZER
	//--------------------------------------
#pragma region DUAL_RASTERIZER
	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(m_pWindow, &m_Width, &m_Height);

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

		InitializeSoftwareRasterizer();

		// VEHICLE
		// 1) Load textures
		m_pVehicleDiffuse	= new Texture{ "Resources/vehicle_diffuse.png", m_pDevice };
		m_pVehicleGloss		= new Texture{ "Resources/vehicle_gloss.png",	m_pDevice };
		m_pVehicleNormal	= new Texture{ "Resources/vehicle_normal.png",	m_pDevice };
		m_pVehicleSpecular	= new Texture{ "Resources/vehicle_specular.png", m_pDevice };

		// 2) Load mesh
		m_pVehicle = new Mesh<Vertex_PosTex>{ m_pDevice, "Resources/vehicle.obj", EffectType::PosTex };

		// 3) Set mesh translation
		m_pVehicle->SetWorldMatrix(Matrix::CreateTranslation(0, 0, 50.f));

		// 4) Link textures to effect
		m_pVehicle->GetEffect()->AddMap("gDiffuseMap", m_pVehicleDiffuse);
		m_pVehicle->GetEffect()->AddMap("gNormalMap", m_pVehicleNormal);
		m_pVehicle->GetEffect()->AddMap("gSpecularMap", m_pVehicleGloss);
		m_pVehicle->GetEffect()->AddMap("gGlossinessMap", m_pVehicleSpecular);

		// 5) Supplementary techniques for culling
		m_pVehicle->GetEffect()->AddTechnique("TheGrandTechniqueNOCULL");
		m_pVehicle->GetEffect()->AddTechnique("TheGrandTechniqueFRONTCULL");



		// FIRE EFFECT
		// 1) Load textures
		m_pFireDiffuse = new Texture{ "Resources/fireFX_diffuse.png", m_pDevice };

		// 2) Load mesh
		m_pFireEffect = new Mesh<Vertex_PosTex>{ m_pDevice, "Resources/fireFX.obj", EffectType::PartCov };

		// 3) Set translation
		m_pFireEffect->SetWorldMatrix(Matrix::CreateTranslation(0, 0, 50.f));

		// 4) Link textures to effect
		m_pFireEffect->GetEffect()->AddMap("gDiffuseMap", m_pFireDiffuse);



		// CAMERA
		m_pCamera = new Camera{};
		m_pCamera->Initialize(45.f, { 0.f, 0.f, 0.f }, static_cast<float>(m_Width) / static_cast<float>(m_Height));
		m_pCamera->CalculateProjectionMatrix();
		m_pCamera->CalculateViewMatrix();
	}

	Renderer::~Renderer()
	{
		delete[] m_pDepthBufferPixels;
		ReleaseDirectXResources();
		SAFE_DELETE(m_pMesh)
		SAFE_DELETE(m_pFireEffect)
		SAFE_DELETE(m_pCamera)
		SAFE_DELETE(m_pVehicle)
		SAFE_DELETE(m_pVehicleDiffuse)
		SAFE_DELETE(m_pVehicleGloss)
		SAFE_DELETE(m_pVehicleSpecular)
		SAFE_DELETE(m_pVehicleNormal)
		SAFE_DELETE(m_pFireDiffuse)
	}

	void Renderer::Update(const Timer* pTimer)
	{
		const float deltaTime{ pTimer->GetElapsed() };
		m_pCamera->Update(deltaTime);
		if (m_IsMeshRotating)
		{
			m_pVehicle->UpdateRotation(deltaTime);
			m_pFireEffect->UpdateRotation(deltaTime);
		}

		m_pVehicle->VerticesToProjectionSpace(m_pCamera->viewMatrix, m_pCamera->projectionMatrix, m_pCamera->origin);

		m_pVehicle->UpdateEffectMatrices(m_pCamera);
		m_pFireEffect->UpdateEffectMatrices(m_pCamera);
	}

	void Renderer::Render() const
	{
		if(m_UseSoftwareRasterizer)	SoftwareRender();
		else						HardwareRender();
	}

	void Renderer::CycleShadingModes()
	{
		// Cycling
		m_ShadingMode = static_cast<ShadingMode>(static_cast<int>(m_ShadingMode) + 1);
		if (m_ShadingMode == ShadingMode::ENUM_END)
		{
			m_ShadingMode = static_cast<ShadingMode>(0);
		}

		// Dynamic_cast seems justifiable due to it being only used on a keypress, and not having to implement SetShadingMode in the Effect base class
		dynamic_cast<Effect_PosTex*>(m_pVehicle->GetEffect())->SetShadingMode(static_cast<int>(m_ShadingMode));

		// Console logging:
		ConsoleColorCtrl::GetInstance()->SetConsoleColor(CNSL_YELLOW);
		std::cout << "**(SHARED) Shading Mode = ";

		switch (m_ShadingMode)
		{
		case ShadingMode::observedArea:
			std::cout << "OBSERVED_AREA\n";
			break;
		case ShadingMode::diffuse:
			std::cout << "DIFFUSE\n";
			break;
		case ShadingMode::specular:
			std::cout << "SPECULAR\n";
			break;
		case ShadingMode::combined:
			std::cout << "COMBINED\n";
			break;
		}
	}

	void Renderer::ToggleUseNormalMaps()
	{
		m_IsUsingNormalMap = !m_IsUsingNormalMap;
		dynamic_cast<Effect_PosTex*>(m_pVehicle->GetEffect())->SetUseNormalMap(m_IsUsingNormalMap);

		ConsoleColorCtrl::GetInstance()->SetConsoleColor(CNSL_YELLOW);
		std::cout << "**(SHARED) NormalMap " << (m_IsUsingNormalMap ? "ON" : "OFF") << std::endl;
	}

	void Renderer::ToggleRotateMesh()
	{
		m_IsMeshRotating = !m_IsMeshRotating;
		ConsoleColorCtrl::GetInstance()->SetConsoleColor(CNSL_YELLOW);

		std::cout << "**(SHARED) Vehicle Rotation " << (m_IsMeshRotating ? "ON" : "OFF") << std::endl;
	}

	void Renderer::ToggleRasterizer()
	{
		m_UseSoftwareRasterizer = !m_UseSoftwareRasterizer;

		ConsoleColorCtrl::GetInstance()->SetConsoleColor(CNSL_YELLOW);
		std::cout << "**(SHARED)Rasterizer Mode = " << (m_UseSoftwareRasterizer ? "SOFTWARE" : "HARDWARE") << std::endl;
	}

	void Renderer::ToggleUniformColor()
	{
		m_UseUniformColor = !m_UseUniformColor;

		ConsoleColorCtrl::GetInstance()->SetConsoleColor(CNSL_YELLOW);
		std::cout << "**(SHARED) Uniform ClearColor " << (m_UseUniformColor ? "ON" : "OFF") << std::endl;
	}

	void Renderer::CycleCullingMode()
	{
		// Software cycling
		// Cycle
		m_CullingMode = static_cast<CullingMode>(static_cast<int>(m_CullingMode) + 1);

		// Correct if set to ENUM_END in cycle
		if (m_CullingMode == CullingMode::ENUM_END) m_CullingMode = static_cast<CullingMode>(0);

		// Set Hardware culling mode
		dynamic_cast<Effect_PosTex*>(m_pVehicle->GetEffect())->SetCullingMode(m_CullingMode);

		// Console logging
		ConsoleColorCtrl::GetInstance()->SetConsoleColor(CNSL_YELLOW);
		std::cout << "**(SHARED) CullMode = ";
		switch(m_CullingMode)
		{
		case CullingMode::back:
			std::cout << "BACK\n";
			break;
		case CullingMode::front:
			std::cout << "FRONT\n";
			break;
		case CullingMode::ENUM_END:	// Should not occur, used for warning supression. If this state is selected, SoftwareRender will log it in console.
		case CullingMode::none:
			std::cout << "NONE\n";
			break;
		}
	}
#pragma endregion

	//--------------------------------------
	// HARDWARE RASTERIZER
	//--------------------------------------
#pragma region HARDWARE_RASTERIZER
	HRESULT Renderer::InitializeDirectX()
	{
		HRESULT result{ InitializeDeviceAndDeviceContext() };
		if (FAILED(result)) return result;

		// Create IDXGI Factory
		IDXGIFactory1* pIdxgiFactory{}; 
		result = CreateDXGIFactory1(_uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pIdxgiFactory));
		if (FAILED(result)) return result;

		result = InitializeSwapChain(pIdxgiFactory);
		pIdxgiFactory->Release();
		if (FAILED(result)) return result;

		result = InitializeDepthStencil();
		if (FAILED(result))	return result;

		result = InitializeDepthStencilView();
		if (FAILED(result)) return result;

		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result)) return result;

		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result)) return result;

		// Bind RTV & DSV to Output Merger State
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		// Set Viewport
		SetViewport();

		return S_OK;
	}

	HRESULT Renderer::InitializeDeviceAndDeviceContext()
	{
		D3D_FEATURE_LEVEL featureLevel{ D3D_FEATURE_LEVEL_11_1 };
		uint32_t createDeviceFlags{ 0 };
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		return D3D11CreateDevice(
			nullptr,						
			D3D_DRIVER_TYPE_HARDWARE,		
			0,								
			createDeviceFlags,
			&featureLevel,
			1,
			D3D11_SDK_VERSION,
			&m_pDevice,			
			nullptr,
			&m_pDeviceContext);
	}

	HRESULT Renderer::InitializeSwapChain(IDXGIFactory* pIdxgiFactory)
	{
		// Create swapchain
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

		SDL_SysWMinfo sysWMInfo{}; // SDL owns window handle
		SDL_VERSION(&sysWMInfo.version) SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);			
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;								

		return pIdxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);	
	}

	HRESULT Renderer::InitializeDepthStencil()
	{
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

		return m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
	}

	HRESULT Renderer::InitializeDepthStencilView()
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;		
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D; 
		depthStencilViewDesc.Texture2D.MipSlice = 0;						

		return m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	}

	void Renderer::SetViewport()
	{
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewport);
	}

	void Renderer::ReleaseDirectXResources()
	{
		if (m_pRenderTargetBuffer)	m_pRenderTargetBuffer->Release();
		if (m_pRenderTargetView)	m_pRenderTargetView->Release();
		if (m_pDepthStencilView)	m_pDepthStencilView->Release();
		if (m_pDepthStencilBuffer)	m_pDepthStencilBuffer->Release();
		if (m_pSwapChain)			m_pSwapChain->Release();
		if (m_pDevice)				m_pDevice->Release();

		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}
	}

	void Renderer::HardwareRender() const
	{
		if (!m_IsInitialized)
			return;
		// 1. Clear RTV & DSV
		ColorRGB clearColor{m_UseUniformColor ? m_UniformClearColor : m_HardwareClearColor};
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		// 2. Set pipeline & invoke drawcalls (= render)
		m_pVehicle->HardwareRender(m_pDeviceContext, m_pDevice);
		if(m_UseFireFX) m_pFireEffect->HardwareRender(m_pDeviceContext, m_pDevice);

		// 3. Present backbuffer (swap)
		m_pSwapChain->Present(0, 0);
	}

	void Renderer::CycleFilteringTechniques()
	{
		m_FilteringMode = static_cast<FilteringMode>(static_cast<int>(m_FilteringMode) + 1);
		if(m_FilteringMode == FilteringMode::ENUM_END)
		{
			m_FilteringMode = static_cast<FilteringMode>(0);
		}

		m_pVehicle->GetEffect()->SetFilterMode(static_cast<int>(m_FilteringMode));
		m_pFireEffect->GetEffect()->SetFilterMode(static_cast<int>(m_FilteringMode));

		ConsoleColorCtrl::GetInstance()->SetConsoleColor(CNSL_GREEN);

		std::cout << "**(HARDWARE) Sampler Filter = ";
		switch (m_FilteringMode)
		{
		case FilteringMode::point:
			std::cout << "POINT" << std::endl;
			break;
		case FilteringMode::linear:
			std::cout << "LINEAR" << std::endl;
			break;
		case FilteringMode::ENUM_END: // Impossible; in here for warning suppression
		case FilteringMode::anisotropic:
			std::cout << "ANISOTROPIC" << std::endl;
			break;
		}
	}

	void Renderer::ToggleFireFX()
	{
		m_UseFireFX = !m_UseFireFX;
		ConsoleColorCtrl::GetInstance()->SetConsoleColor(CNSL_GREEN);
		std::cout << "**(HARDWARE) FireFX " << (m_UseFireFX ? "ON" : "OFF") << std::endl;
	}
#pragma endregion


	//--------------------------------------
	// SOFTWARE RASTERIZER
	//--------------------------------------
#pragma region SOFTWARE_RASTERIZER
	void Renderer::InitializeSoftwareRasterizer()
	{
		//Create Buffers
		m_pFrontBuffer = SDL_GetWindowSurface(m_pWindow);
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

		m_NrOfPixels = m_Width * m_Height;
		m_pDepthBufferPixels = new float[m_NrOfPixels];
	}

	void Renderer::SoftwareRender() const
	{
		std::fill_n(m_pDepthBufferPixels, m_NrOfPixels, FLT_MAX);

		ColorRGB clearColor{ m_UseUniformColor ? m_UniformClearColor : m_SoftwareClearColor };
		SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, static_cast<int>(clearColor.r * 255), static_cast<int>(clearColor.g * 255), static_cast<int>(clearColor.b * 255)));
		//@START
		//Lock BackBuffer
		SDL_LockSurface(m_pBackBuffer);

		auto& vertsOut{ m_pVehicle->GetVertexOutVector() };
		const int lastTriangleStartIndex{ static_cast<int>(m_pVehicle->GetNumIndices() - 2) };
		auto vertIndices{ m_pVehicle->GetIndices() };

		// For every triangle
		for (int vertexIndex{}; vertexIndex < lastTriangleStartIndex; vertexIndex += 3)
		{
			int v1Index{ vertexIndex + 1 };
			int v2Index{ vertexIndex + 2 };

			Vertex_Out v0{ vertsOut[vertIndices[vertexIndex]] };
			Vertex_Out v1{ vertsOut[vertIndices[v1Index]] };
			Vertex_Out v2{ vertsOut[vertIndices[v2Index]] };

			// X & Y frustrum culling
			if (   v0.position.x > 1.f || v0.position.x < -1.f
				|| v1.position.x > 1.f || v1.position.x < -1.f
				|| v2.position.x > 1.f || v2.position.x < -1.f
				|| v0.position.y > 1.f || v0.position.y < -1.f
				|| v1.position.y > 1.f || v1.position.y < -1.f
				|| v2.position.y > 1.f || v2.position.y < -1.f)
			{
				continue;
			}

			VertexProjectionToScreenSpace(v0);
			VertexProjectionToScreenSpace(v1);
			VertexProjectionToScreenSpace(v2);

			// Check triangle against culling mode
			if (IsTriangleCulled(v0, v1, v2)) continue;

			const Pixel2D boundingBoxMin{ Utils::CalcBoundingBoxMin(v0, v1, v2) };
			const Pixel2D boundingBoxMax{ Utils::CalcBoundingBoxMax(v0, v1, v2, m_Width, m_Height) };

			const float areaParallelogram{Utils::CalcAreaParallelogram(v0, v1, v2) };

			//RENDER LOGIC
			// For every pixel of triangle
			for (int px{ boundingBoxMin.x }; px < boundingBoxMax.x; ++px)
			{
				for (int py{ boundingBoxMin.y }; py < boundingBoxMax.y; ++py)
				{
					RenderPixel(px, py, v0, v1, v2, areaParallelogram);
				}
			}
		}
	//@END
	//Update SDL Surface
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
	}

	void Renderer::VertexProjectionToScreenSpace(Vertex_Out& vertex) const
	{
		vertex.position.x = (vertex.position.x + 1) / 2 * static_cast<float>(m_Width);
		vertex.position.y = (1 - vertex.position.y) / 2 * static_cast<float>(m_Height);
	}

	bool Renderer::IsTriangleCulled(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2) const
	{
		if (m_CullingMode == CullingMode::none) return false; // No senses need to be checked, no culling

		const Vector3 v0toV1{ v1.position - v0.position };
		const Vector3 v0toV2{ v2.position - v0.position };
		const float sign{ Vector3::Cross(v0toV1, v0toV2).z };

		switch(m_CullingMode)
		{
		case CullingMode::back:
			if (sign <= 0 )return true;
			return false;
		case CullingMode::front:
			if (sign >= 0) return true;
			return false;
		default:
			std::cout << "Something went wrong! CullingMode seems to be set to value that's not in Enum!\n";
			return true; // Oh no! Everything gets culled
		}
	}

	void Renderer::RenderPixel(int px, int py, Vertex_Out& v0, Vertex_Out& v1, Vertex_Out& v2, float area) const
	{
		ColorRGB finalColor{};
		if(m_ShowOnlyBoundingBoxes)
		{
			finalColor = { 1.f, 1.f, 1.f };
			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
			return;
		}

		// Pixel vector for further calculations
		Vector2 pixel{ static_cast<float>(px), static_cast<float>(py) };

		// Weight calculations
		const float weightV0{ Utils::CalcWeight(v1, v2, pixel, area) };
		const float weightV1{ Utils::CalcWeight(v2, v0, pixel, area) };
		const float weightV2{ Utils::CalcWeight(v0, v1, pixel, area) };

		// Does pixel require rendering?
		const bool insideTriangle{ weightV0 >= 0.f && weightV1 >= 0.f && weightV2 >= 0.f };
		if (!insideTriangle)
		{
			return;
		}

		// Depth
		const float zDepth{ Utils::Interpolate(v0.position.z, v1.position.z, v2.position.z, weightV0, weightV1, weightV2) };
		// Z Frustrum culling
		if (zDepth > 1.f || zDepth < 0.f)
		{
			return;
		}

		const float wDepth{ Utils::Interpolate(v0.position.w, v1.position.w, v2.position.w, weightV0, weightV1, weightV2) };

		// Aliases for readablity
		float& depthBufferElement{ m_pDepthBufferPixels[px + (py * m_Width)] };
		const bool isCloserToCamera{ zDepth < depthBufferElement };

		if (!isCloserToCamera)
		{
			return;
		}

		depthBufferElement = zDepth;

		if (m_ShowOnlyDepthBuffer)
		{
			const float remappedValue{ Remap(zDepth, 0.9925f, 1.f) }; // I chose a slightly different value here, because I prefer the contrast this gives
			finalColor = { remappedValue, remappedValue, remappedValue };
		}
		else
		{
			Vertex_Out shadingVertex{};
			shadingVertex.position.x = pixel.x;
			shadingVertex.position.y = pixel.y;
			shadingVertex.position.z = zDepth;
			shadingVertex.position.w = wDepth;

			Vector2 uvInterpolated{ ((v0.uv / v0.position.w) * weightV0 + (v1.uv / v1.position.w) * weightV1 + (v2.uv / v2.position.w) * weightV2) * wDepth };
			shadingVertex.uv = uvInterpolated;

			Vector3 interpolatedNormal{ ((v0.normal / v0.position.w) * weightV0 + (v1.normal / v1.position.w) * weightV1 + (v2.normal / v2.position.w) * weightV2) * wDepth };
			interpolatedNormal.Normalize();
			shadingVertex.normal = interpolatedNormal;

			Vector3 interpolatedTangent{ ((v0.tangent / v0.position.w) * weightV0 + (v1.tangent / v1.position.w) * weightV1 + (v2.tangent / v2.position.w) * weightV2) * wDepth };
			interpolatedTangent.Normalize();
			shadingVertex.tangent = interpolatedTangent;

			Vector3 interpolatedViewDirection{ ((v0.viewDirection / v0.position.w) * weightV0 + (v1.viewDirection / v1.position.w) * weightV1 + (v2.viewDirection / v2.position.w) * weightV2) * wDepth };
			interpolatedViewDirection.Normalize();
			shadingVertex.viewDirection = interpolatedViewDirection;

			finalColor = PixelShading(shadingVertex);
		}

		//Update Color in Buffer
		finalColor.MaxToOne();

		m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
			static_cast<uint8_t>(finalColor.r * 255),
			static_cast<uint8_t>(finalColor.g * 255),
			static_cast<uint8_t>(finalColor.b * 255));
	}

	ColorRGB Renderer::PixelShading(const Vertex_Out& v) const
	{
		// Direction light
		const Vector3 lightDirection{ .577f, -.577f, .577f };
		constexpr float lightIntensity{ 7.f };

		Vector3 normal{ v.normal };

		if (m_IsUsingNormalMap)
		{
			Vector3 binormal(Vector3::Cross(v.normal, v.tangent));
			binormal.Normalize();

			const Matrix tangentSpaceAxis = Matrix{ v.tangent, binormal, v.normal, Vector3::Zero };

			const ColorRGB sampledNormalRGB = m_pVehicleNormal->Sample(v.uv);
			normal =
			{ 2 * sampledNormalRGB.r - 1.f,
			  2 * sampledNormalRGB.g - 1.f,
			  2 * sampledNormalRGB.b - 1.f
			};
			normal = tangentSpaceAxis.TransformVector(normal);
		}

		float observedArea{ Vector3::Dot(normal, -lightDirection) };

		// Observed area, light direction reversed because of "opposite senses"
		if (observedArea < 0.f)
		{
			observedArea = 0.f;
		}

		// Lambert
		constexpr float kd = lightIntensity;
		const ColorRGB cd = m_pVehicleDiffuse->Sample(v.uv);
		constexpr float pi{ static_cast<float>(M_PI) };
		const ColorRGB lambert{ kd * cd / pi };

		// Phong
		const Vector3 reflect{ lightDirection - 2 * Vector3::Dot(lightDirection, normal) * normal };
		float cosAlpha{ Vector3::Dot(reflect, -v.viewDirection) };
		if (cosAlpha < 0.f)
		{
			cosAlpha = 0.f;
		}
		const ColorRGB ks = m_pVehicleSpecular->Sample(v.uv);
		const float exp = m_pVehicleGloss->Sample(v.uv).r;
		constexpr float shininess{ 25.f };
		ColorRGB phongSpecularReflection{ ks * powf(cosAlpha, exp * shininess) };

		switch (m_ShadingMode)
		{
		case ShadingMode::observedArea:
			return { observedArea, observedArea, observedArea };

		case ShadingMode::diffuse:
			return lambert * observedArea;

		case ShadingMode::specular:
			return phongSpecularReflection * observedArea;

		case ShadingMode::ENUM_END: // Should not be possible, is in here for warning suppression
		case ShadingMode::combined:
			return (lambert + phongSpecularReflection) * observedArea + ColorRGB{ .025f, .025f, .025f };
		}

		std::cout << "Oh no! Rendering pixel with correct colors failed!\n";
		return ColorRGB{}; // If this is returned (= completely black), something went wrong. In here for warning suppression
	}

	void Renderer::ToggleShowDepthBuffer()
	{
		m_ShowOnlyDepthBuffer = !m_ShowOnlyDepthBuffer;
		ConsoleColorCtrl::GetInstance()->SetConsoleColor(CNSL_PURPLE);
		std::cout << "**(SOFTWARE) DepthBuffer Visualization " << (m_ShowOnlyDepthBuffer ? "ON" : "OFF") << std::endl;
	}

	void Renderer::ToggleShowBoundingBoxes()
	{
		m_ShowOnlyBoundingBoxes = !m_ShowOnlyBoundingBoxes;
		ConsoleColorCtrl::GetInstance()->SetConsoleColor(CNSL_PURPLE);
		std::cout << "**(SOFTWARE) BoundingBox Visualization " << (m_ShowOnlyBoundingBoxes ? "ON" : "OFF") << std::endl;
	}
#pragma endregion
}
