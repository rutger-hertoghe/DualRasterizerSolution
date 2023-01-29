#include "pch.h"
#include "Texture.h"

namespace dae
{
	//--------------------------------------
	// DUAL RASTERIZER
	//--------------------------------------
#pragma region Common_Functions
	Texture::Texture(const std::string& path, ID3D11Device* pDevice)
	{
		m_pSurface = IMG_Load(path.c_str());
		if(m_pSurface == nullptr)
		{
			std::cout << "SDL_Surface not created!\n Is the path correct?\n";
			return;
		}

		m_pSurfacePixels = static_cast<uint32_t*>(m_pSurface->pixels);

		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = m_pSurface->w;
		desc.Height = m_pSurface->h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = m_pSurface->pixels;
		initData.SysMemPitch = static_cast<UINT>(m_pSurface->pitch);
		initData.SysMemSlicePitch = static_cast<UINT>(m_pSurface->pitch * m_pSurface->h);

		HRESULT result{ pDevice->CreateTexture2D(&desc, &initData, &m_pResource) };

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;
		result = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pShaderResourceView);
	}

	Texture::~Texture()
	{
		// Release DirectX Resources
		if(m_pShaderResourceView) m_pShaderResourceView->Release();
		if(m_pResource) m_pResource->Release();

		// Release Software Shader SDL Surface
		if(m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}
#pragma endregion

	//--------------------------------------
	// HARDWARE RASTERIZER
	//--------------------------------------
#pragma region Hardware_Rasterizer
	ID3D11ShaderResourceView* Texture::GetSRV()
	{
		return m_pShaderResourceView;
	}
#pragma endregion

	//--------------------------------------
	// SOFTWARE RASTERIZER
	//--------------------------------------
#pragma region Software_Rasterizer
	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		const float uvX{ Clamp(uv.x, 0.f, 1.f) };
		const float uvY{ Clamp(uv.y, 0.f, 1.f) };
		const Uint32 px{ static_cast<Uint32>(uvX * (m_pSurface->w)) };
		const Uint32 py{ static_cast<Uint32>(uvY * (m_pSurface->h)) };
		const Uint32 index{ px + m_pSurface->w * py };

		Uint8 r{}, g{}, b{};
		SDL_GetRGB(m_pSurfacePixels[index], m_pSurface->format, &r, &g, &b);

		return { static_cast<float>(r) / 255.f, static_cast<float>(g) / 255.f, static_cast<float>(b) / 255.f };
	}
#pragma endregion
}
