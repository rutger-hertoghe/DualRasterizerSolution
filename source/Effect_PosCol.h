#pragma once
#include "Effect.h"
namespace dae
{
	class Effect_PosCol : public Effect
	{
	public:
		explicit Effect_PosCol(ID3D11Device* pDevice);
		virtual ~Effect_PosCol() override = default;

		Effect_PosCol(const Effect_PosCol& other) = delete;
		Effect_PosCol operator=(const Effect_PosCol& other) = delete;
		Effect_PosCol(Effect_PosCol&& other) = delete;
		Effect_PosCol operator=(Effect_PosCol&& other) = delete;

		virtual EffectType GetFxType() override;

	private:
		virtual void InitializeVertexDesc(D3D11_INPUT_ELEMENT_DESC* vertexDesc) override;
	};
}