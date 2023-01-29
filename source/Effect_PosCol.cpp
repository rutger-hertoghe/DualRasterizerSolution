#include "pch.h"
#include "Effect_PosCol.h"

namespace dae
{
	Effect_PosCol::Effect_PosCol(ID3D11Device* pDevice)
		: Effect(pDevice, L"Resources/PosCol3D.fx", "DefaultTechnique")
	{
		// Create Vertex Layout
		static constexpr uint32_t numElements{ 2 };
		D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};
		InitializeVertexDesc(vertexDesc);
		InitializeInputLayout(pDevice, vertexDesc, numElements);
	}

	EffectType dae::Effect_PosCol::GetFxType()
	{
		return EffectType::PosCol;
	}

	void Effect_PosCol::InitializeVertexDesc(D3D11_INPUT_ELEMENT_DESC* vertexDesc)
	{
		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = 0;
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[1].SemanticName = "COLOR";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[1].AlignedByteOffset = 12;
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	}
}

