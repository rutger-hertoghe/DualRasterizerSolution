//-----------------------------
// Globals
//-----------------------------
float4x4 gWorldViewProj : WorldViewProjection;

RasterizerState gRasterizerState
{
	CullMode = back;
	FrontCounterClockwise = false;
};

BlendState gBlendState
{
	BlendEnable[0] = false;
	SrcBlend = one;
	DestBlend = zero;
	BlendOp = add;
	SrcBlendAlpha = one;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = true;
	DepthFunc = less;
	StencilEnable = false;
};


//-----------------------------
// Input/Output Structs
//-----------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Color : COLOR;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 Color : COLOR;
};

//-----------------------------
// Vertex Shader
//-----------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0; // C-style cast of zero to zero output
	output.Position = mul(float4(input.Position, 1.f) , gWorldViewProj);
	output.Color = input.Color;
	return output;
}

//-----------------------------
// Pixel Shader
//-----------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	return float4(input.Color, 1.f);
}

//-----------------------------
// Technique
//-----------------------------
technique11 DefaultTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS() ) );
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS() )  );
	}
}