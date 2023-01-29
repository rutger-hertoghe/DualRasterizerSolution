#include "pch.h"
#include "Effect.h"
#include <cassert>

#include "Texture.h"

namespace dae
{
	Effect::Effect(ID3D11Device* pDevice, const std::wstring& fxPath, const LPCSTR& technique)
	{
		m_pEffect = LoadEffect(pDevice, fxPath);

		m_pFilterMode = m_pEffect->GetVariableBySemantic("FILTERMODE")->AsScalar();
		if(m_pFilterMode->IsValid()) m_pFilterMode->SetInt(2); // 2 = anisotropic

		InitializeMatrixVariables();
		m_pCurrentTechnique = AddTechnique(technique);
	}

	Effect::~Effect()
	{
		ReleaseMaps();
		ReleaseMatrices();
		ReleaseTechniques();

		if (m_pFilterMode) m_pFilterMode->Release();
		if(m_pInputLayout) m_pInputLayout->Release();
		if (m_pEffect) m_pEffect->Release();
	}

	ID3DX11Effect* Effect::GetEffect()
	{
		return m_pEffect;
	}

	ID3DX11EffectTechnique* Effect::GetTechnique()
	{
		return m_pCurrentTechnique;
	}

	ID3D11InputLayout* Effect::GetInputLayout()
	{
		return m_pInputLayout;
	}

	void Effect::SetFilterMode(int mode)
	{
		if(m_pFilterMode->IsValid()) m_pFilterMode->SetInt(mode);
	}

	void Effect::AddMap(const LPCSTR& mapName, Texture* texture)
	{
		m_pMapVariables.emplace(mapName, m_pEffect->GetVariableByName(mapName)->AsShaderResource());

		if(!m_pMapVariables.at(mapName)->IsValid())
		{
			std::wcout << mapName << L" not valid!\n";
		}

		if(m_pMapVariables.at(mapName))
		{
			m_pMapVariables.at(mapName)->SetResource(texture->GetSRV());
		}
	}

	ID3DX11EffectTechnique* Effect::AddTechnique(const LPCSTR& techniqueName)
	{
		m_pTechniques.insert(std::make_pair(techniqueName, m_pEffect->GetTechniqueByName(techniqueName)));
		return m_pTechniques.at(techniqueName);
	}

	void Effect::ReleaseMaps()
	{
		for(auto& map : m_pMapVariables)
		{
			if(map.second) map.second->Release();
		}
	}

	void Effect::ReleaseMatrices()
	{
		if (m_pMatWorldViewProjVariable) m_pMatWorldViewProjVariable->Release();
		if (m_pMatWorldVariable) m_pMatWorldVariable->Release();
		if (m_pMatInvViewVariable) m_pMatInvViewVariable->Release();
	}

	void Effect::ReleaseTechniques()
	{
		for(auto& technique : m_pTechniques)
		{
			if(technique.second) technique.second->Release();
		}
	}

	void Effect::SetWorldViewProjMatrix(const Matrix& worldViewProjMatrix)
	{
		m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<const float*>(&worldViewProjMatrix));
	}

	void Effect::SetWorldMatrix(const Matrix& worldMatrix)
	{
		m_pMatWorldVariable->SetMatrix(reinterpret_cast<const float*>(&worldMatrix));
	}

	void Effect::SetInvViewMatrix(const Matrix& invViewMatrix)
	{
		m_pMatInvViewVariable->SetMatrix(reinterpret_cast<const float*>(&invViewMatrix));
	}

	void Effect::InitializeMatrixVariables()
	{
		m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
		if (!m_pMatWorldViewProjVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldViewProjVariable not valid!\n";
		}

		m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
		if (!m_pMatWorldVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldVariable not valid!\n";
		}

		m_pMatInvViewVariable = m_pEffect->GetVariableByName("gViewInvMatrix")->AsMatrix();
		if (!m_pMatInvViewVariable->IsValid())
		{
			std::wcout << L"m_pMatInvViewVariable not valid!\n";
		}
	}

	void Effect::InitializeInputLayout(ID3D11Device* pDevice, D3D11_INPUT_ELEMENT_DESC* vertexDesc, uint32_t numElements)
	{
		// Create Input Layout
		D3DX11_PASS_DESC passDesc{};
		m_pCurrentTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

		HRESULT result{ pDevice->CreateInputLayout(
			vertexDesc,
			numElements,
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&m_pInputLayout)
		};

		if (FAILED(result))
		{
			assert(false);
		}
	}

	ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	{
		HRESULT result;
		ID3D10Blob* pErrorBlob{ nullptr };
		ID3DX11Effect* pEffect;

		DWORD shaderFlags = 0;
#if defined(DEBUG) || defined (_DEBUG)
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		result = D3DX11CompileEffectFromFile(assetFile.c_str(), nullptr, nullptr, shaderFlags, 0, pDevice, &pEffect, &pErrorBlob);

		if (FAILED(result))
		{
			if (pErrorBlob != nullptr)
			{
				const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

				std::wstringstream ss;
				for (unsigned int i{ 0 }; i < pErrorBlob->GetBufferSize(); ++i)
				{
					ss << pErrors[i];
				}

				OutputDebugStringW(ss.str().c_str());
				pErrorBlob->Release();
				pErrorBlob = nullptr;

				std::wcout << ss.str() << std::endl;
			}
			else
			{
				std::wstringstream ss;
				ss << "EffectLoader: Failed to CreateEffectFromFile! \nPath: " << assetFile;
				std::wcout << ss.str() << std::endl;
				return nullptr;
			}
		}
		return pEffect;
	}
}



