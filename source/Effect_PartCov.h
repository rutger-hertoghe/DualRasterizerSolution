#pragma once
#include "Effect.h"
namespace dae {
	class Effect_PartCov : public Effect
	{
	public:
		explicit Effect_PartCov(ID3D11Device* pDevice);

		virtual EffectType GetFxType() override;

	private:
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;

		virtual void InitializeVertexDesc(D3D11_INPUT_ELEMENT_DESC* vertexDesc) override;
	};
}

