#pragma once
#include <unordered_map>

namespace dae
{
	class Texture;
	enum class EffectType
	{
		PosCol,
		PosTex,
		PartCov
	};

	class Effect
	{
	public:
		explicit Effect(ID3D11Device* pDevice, const std::wstring& fxPath, const LPCSTR& technique);
		virtual ~Effect();

		virtual EffectType GetFxType() = 0;

		void SetWorldViewProjMatrix(const Matrix& worldViewProjMatrix);
		void SetWorldMatrix(const Matrix& worldMatrix);
		void SetInvViewMatrix(const Matrix& invViewMatrix);

		ID3DX11Effect* GetEffect();
		ID3DX11EffectTechnique* GetTechnique();
		ID3D11InputLayout* GetInputLayout();

		void SetFilterMode(int mode);

		void AddMap(const LPCSTR& mapName, Texture* texture);
		ID3DX11EffectTechnique* AddTechnique(const LPCSTR& techniqueName);

	protected:
		ID3DX11Effect* m_pEffect;
		ID3DX11EffectTechnique* m_pCurrentTechnique;

		ID3D11InputLayout* m_pInputLayout;

		ID3DX11EffectScalarVariable* m_pFilterMode;

		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
		ID3DX11EffectMatrixVariable* m_pMatWorldVariable;
		ID3DX11EffectMatrixVariable* m_pMatInvViewVariable;

		//std::unordered_map<LPCSTR, ID3DX11EffectMatrixVariable*> m_pMatrixVariables;
		std::unordered_map<LPCSTR, ID3DX11EffectShaderResourceVariable*> m_pMapVariables;
		std::unordered_map<LPCSTR, ID3DX11EffectTechnique*> m_pTechniques;

		void InitializeMatrixVariables();
		virtual void InitializeVertexDesc(D3D11_INPUT_ELEMENT_DESC* vertexDesc) = 0;
		void InitializeInputLayout(ID3D11Device* pDevice, D3D11_INPUT_ELEMENT_DESC* vertexDesc, uint32_t numElements);

		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

		void ReleaseMaps();
		void ReleaseMatrices();
		void ReleaseTechniques();
	};
}


