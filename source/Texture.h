#pragma once

namespace dae
{
	class Texture
	{
	public:
		// Dual Rasterizer
		Texture(const std::string& path, ID3D11Device* pDevice);
		~Texture();

		Texture(const Texture& other) = delete;
		Texture operator=(const Texture& other) = delete;
		Texture(Texture&& other) = delete;
		Texture operator=(Texture&& other) = delete;

		// Hardware Rasterizer
		ID3D11ShaderResourceView* GetSRV();

		// Software Rasterizer
		ColorRGB Sample(const Vector2& uv) const;

	private:
		// Hardware Rasterizer
		ID3D11Texture2D* m_pResource;
		ID3D11ShaderResourceView* m_pShaderResourceView;

		// Software Rasterizer
		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };
	};
}

