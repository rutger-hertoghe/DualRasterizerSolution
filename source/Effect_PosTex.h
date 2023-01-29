#pragma once
#include "Effect.h"

namespace dae
{
	enum class CullingMode;

	class Effect_PosTex : public dae::Effect
	{
	public:
		explicit Effect_PosTex(ID3D11Device* pDevice);
		virtual ~Effect_PosTex() override;

		Effect_PosTex(const Effect_PosTex& other) = delete;
		Effect_PosTex operator=(const Effect_PosTex& other) = delete;
		Effect_PosTex(Effect_PosTex&& other) = delete;
		Effect_PosTex operator=(Effect_PosTex&& other) = delete;

		void ToggleNormalMap();
		void SetUseNormalMap(bool isUsingNormalMap);
		void CycleFilterTechnique();
		void SetCullingMode(CullingMode cullingMode);
		void SetShadingMode(int shadingMode);
		EffectType GetFxType() override;

	private:
		UINT m_ShadingModeINT;

		ID3DX11EffectScalarVariable* m_pShadingMode;
		ID3DX11EffectScalarVariable* m_pUsingNormalMap;
		ID3DX11EffectScalarVariable* m_pShininess;

		void InitializeVertexDesc(D3D11_INPUT_ELEMENT_DESC* vertexDesc) override;
		void InitializePrimitives();
	};
}
