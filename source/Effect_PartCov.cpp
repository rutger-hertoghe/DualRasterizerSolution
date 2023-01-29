#include "pch.h"
#include "Effect_PartCov.h"

#include "Texture.h"

namespace dae
{
	Effect_PartCov::Effect_PartCov(ID3D11Device* pDevice)
		: Effect{pDevice, L"Resources/PartCov.fx", "DefaultTechnique"}
	{
		// Create Vertex Layout
		static constexpr uint32_t numElements{2};
		D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};
		InitializeVertexDesc(vertexDesc);

		// Create Input Layout
		InitializeInputLayout(pDevice, vertexDesc, numElements);
	}

	EffectType Effect_PartCov::GetFxType()
	{
		return EffectType::PartCov;
	}

	void Effect_PartCov::InitializeVertexDesc(D3D11_INPUT_ELEMENT_DESC* vertexDesc)
	{
		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = 0;
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[1].SemanticName = "TEXCOORD";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertexDesc[1].AlignedByteOffset = 12;
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	}
}
