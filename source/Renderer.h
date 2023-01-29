#pragma once
#include <unordered_map>

#include "Structs.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	template<typename T_Vertex>
	class Mesh;
	struct Camera;
	class Texture;
	enum class EffectType;

	enum class ShadingMode
	{
		observedArea,
		diffuse,
		specular,
		combined,
		ENUM_END
	};

	enum class CullingMode
	{
		back,
		front,
		none,
		ENUM_END
	};

	enum class FilteringMode
	{
		point,
		linear,
		anisotropic,
		ENUM_END
	};

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;

		template<typename T_Vertex>
		Mesh<T_Vertex>* GetMesh() const;

	private:
		// DUAL RASTERIZER
		const ColorRGB m_UniformClearColor{ .1f, .1f, .1f };

		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };
		bool m_IsMeshRotating{ true };
		bool m_UseSoftwareRasterizer{ false };
		bool m_UseUniformColor{ false };

		ShadingMode m_ShadingMode = ShadingMode::combined;
		CullingMode m_CullingMode = CullingMode::none;

		Texture* m_pVehicleDiffuse;
		Texture* m_pVehicleGloss;
		Texture* m_pVehicleSpecular;
		Texture* m_pVehicleNormal;

		Mesh<Vertex_PosTex>* m_pMesh;
		Mesh<Vertex_PosTex>* m_pVehicle;
		Mesh<Vertex_PosTex>* m_pFireEffect;
		Mesh<Vertex_PosCol>* m_pTestMesh;
		Camera* m_pCamera;

		// HARDWARE RASTERIZER
		const ColorRGB m_HardwareClearColor{ .39f, .59f, .93f };

		bool m_UseFireFX{true};

		FilteringMode m_FilteringMode{ FilteringMode::anisotropic };

		ID3D11Device* m_pDevice{};
		ID3D11DeviceContext* m_pDeviceContext{};
		IDXGISwapChain* m_pSwapChain{};
		ID3D11Texture2D* m_pDepthStencilBuffer{};
		ID3D11DepthStencilView* m_pDepthStencilView{};
		ID3D11Resource* m_pRenderTargetBuffer{};
		ID3D11RenderTargetView* m_pRenderTargetView{};
		
		Texture* m_pFireDiffuse;

		// DirectX
		HRESULT InitializeDirectX();
		HRESULT InitializeDeviceAndDeviceContext();
		HRESULT InitializeSwapChain(IDXGIFactory* pIdxgiFactory);
		HRESULT InitializeDepthStencil();
		HRESULT InitializeDepthStencilView();
		void SetViewport();

		void ReleaseDirectXResources();

		void HardwareRender() const;

		// SOFTWARE RASTERIZER

		const ColorRGB m_SoftwareClearColor{ .39f, .39f, .39f };

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		int m_NrOfPixels{};
		float m_AspectRatio{};

		bool m_IsUsingNormalMap{ true };
		bool m_ShowOnlyDepthBuffer{ false };
		bool m_ShowOnlyBoundingBoxes{ false };

		void InitializeSoftwareRasterizer();
		void SoftwareRender() const;
		void VertexProjectionToScreenSpace(Vertex_Out& vertex) const;

		bool IsTriangleCulled(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2) const;

		void RenderPixel(int px, int py, Vertex_Out& v0, Vertex_Out& v1, Vertex_Out& v2, float area) const;
		ColorRGB PixelShading(const Vertex_Out& v) const;
	};

	//--------------------------
	// Template definitions
	//--------------------------
	template<typename T_Vertex>
	Mesh<T_Vertex>* Renderer::GetMesh() const
	{
		return m_pMesh;
	}
}
