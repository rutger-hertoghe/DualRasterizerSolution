#include "pch.h"
#include "Effect_PosTex.h"
#include <cassert>

#include "Texture.h"
#include "Renderer.h"

namespace dae
{
	Effect_PosTex::Effect_PosTex(ID3D11Device* pDevice)
		: Effect(pDevice, L"Resources/PosTex3D.fx", "TheGrandTechnique")
		, m_ShadingModeINT(3)
	{
		InitializePrimitives();

		// Create Vertex Layout
		static constexpr uint32_t numElements{ 4 };
		D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};
		InitializeVertexDesc(vertexDesc);

		InitializeInputLayout(pDevice, vertexDesc, numElements);
	}

	Effect_PosTex::~Effect_PosTex()
	{
		if (m_pShadingMode) m_pShadingMode->Release();
		if (m_pUsingNormalMap) m_pUsingNormalMap->Release();
	}

	void Effect_PosTex::SetCullingMode(CullingMode cullingMode)
	{
		switch(cullingMode)
		{
		case CullingMode::back:
			m_pCurrentTechnique = m_pTechniques.at("TheGrandTechnique");
			break;
		case CullingMode::front:
			m_pCurrentTechnique = m_pTechniques.at("TheGrandTechniqueFRONTCULL");
			break;
		case CullingMode::ENUM_END: // Warning suppression, should be impossible to have passed to function
			std::cout << "Illegal culling mode value! Using NOCULL!\n";
		case CullingMode::none:
			m_pCurrentTechnique = m_pTechniques.at("TheGrandTechniqueNOCULL");
			break;
		}
	}

	void Effect_PosTex::ToggleNormalMap()
	{
		bool currentState{};
		m_pUsingNormalMap->GetBool(&currentState);
		m_pUsingNormalMap->SetBool(!currentState);
	}

	void Effect_PosTex::SetUseNormalMap(bool isUsingNormalMap)
	{
		m_pUsingNormalMap->SetBool(isUsingNormalMap);
	}

	void Effect_PosTex::SetShadingMode(int shadingMode)
	{
		m_pShadingMode->SetInt(shadingMode);
	}

	EffectType Effect_PosTex::GetFxType()
	{
		return EffectType::PosTex;
	}

	void Effect_PosTex::InitializeVertexDesc(D3D11_INPUT_ELEMENT_DESC* vertexDesc)
	{
		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = 0;
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[1].SemanticName = "TEXCOORD";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertexDesc[1].AlignedByteOffset = 12;
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[2].SemanticName = "NORMAL";
		vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[2].AlignedByteOffset = 20;
		vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[3].SemanticName = "TANGENT";
		vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[3].AlignedByteOffset = 32;
		vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	}

	void Effect_PosTex::InitializePrimitives()
	{
		m_pShadingMode = m_pEffect->GetVariableBySemantic("SHADINGMODE")->AsScalar();
		m_pShadingMode->SetInt(m_ShadingModeINT);

		m_pUsingNormalMap = m_pEffect->GetVariableBySemantic("USENORMALMAP")->AsScalar();
		m_pUsingNormalMap->SetBool(true);
	}
}