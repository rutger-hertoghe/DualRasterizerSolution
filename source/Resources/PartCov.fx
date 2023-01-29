//-----------------------------
// Globals
//-----------------------------
Texture2D gDiffuseMap : DIFFUSEMAP;
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : WORLD;
float4x4 gViewInvMatrix : VIEWINVERSE;

int gFilter : FILTERMODE;

RasterizerState gRasterizerState
{
	CullMode = none;
	FrontCounterClockwise = false;
};

BlendState gBlendState
{
	BlendEnable[0] = true;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	BlendOp = add;
	SrcBlendAlpha = zero;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = zero;
	DepthFunc = less;
	StencilEnable = false;

	// Others were redundant so not added
};

//-----------------------------
// Sampling Methods
//-----------------------------
SamplerState gSamPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};

SamplerState gSamLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

SamplerState gSamAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Wrap;
	AddressV = Wrap;
};

//-----------------------------
// Input/Output Structs
//-----------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float2 Uv : TEXCOORD;
	/*float3 Normal : NORMAL;
	float3 Tangent : TANGENT;*/
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	//float4 WorldPosition: COLOR;
	float2 Uv: TEXCOORD;
	/*float3 Normal : NORMAL;
	float3 Tangent : TANGENT;*/
};

//-----------------------------
// Vertex Shader
//-----------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0; // C-style cast of zero to zero output
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.Uv = input.Uv;
	/*output.Normal = mul(normalize(input.Normal), (float3x3)gWorldMatrix);
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorldMatrix);*/
	return output;
}

//-----------------------------
// Pixel Shader
//-----------------------------

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	if (gFilter == 0)
	{
		return gDiffuseMap.Sample(gSamPoint, input.Uv);
	}

	if (gFilter == 1)
	{
		return gDiffuseMap.Sample(gSamLinear, input.Uv);
	}

	return gDiffuseMap.Sample(gSamAnisotropic, input.Uv);
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
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
};