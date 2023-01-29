//-----------------------------
// Globals
//-----------------------------

float4x4 gWorldViewProj : WorldViewProjection;
Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

float4x4 gWorldMatrix : WORLD;
float4x4 gViewInvMatrix : VIEWINVERSE;
int gShading : SHADINGMODE;
int gFilterMode : FILTERMODE;
int gCulling = 0;
bool gUsingNormalMap : USENORMALMAP;

// Consts
float3 gLightDirection = { 0.577f, -0.577f, 0.577f };
float gPi = 3.141592653589793f;
float gLightIntensity = 7.f;
float gShininess = 25.f;

RasterizerState gRasterizerState
{
	CullMode = back;
	FrontCounterClockwise = false;
};

RasterizerState gRasterizerStateFRONTCULL
{
	CullMode = front;
	FrontCounterClockwise = false;
};

RasterizerState gRasterizerStateNOCULL
{
	CullMode = none;
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
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition: COLOR;
	float2 Uv: TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};

float4 SampleTextureColor(float2 Uv)
{
	if (gFilterMode == 0)
	{
		return gDiffuseMap.Sample(gSamPoint, Uv);
	}
	
	if (gFilterMode == 1)
	{
		return gDiffuseMap.Sample(gSamLinear, Uv);
	}

	return gDiffuseMap.Sample(gSamAnisotropic, Uv);
}

//-----------------------------
// Vertex Shader
//-----------------------------

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0; // C-style cast of zero to zero output
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.WorldPosition = mul(float4(input.Position, 1.f), gWorldMatrix);
	output.Uv = input.Uv;
	output.Normal = mul(normalize(input.Normal), (float3x3)gWorldMatrix);
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorldMatrix);
	return output;
}

//-----------------------------
// Shading Helpers
//-----------------------------

float3 CalculateNormalFromMap(VS_OUTPUT input)
{
	float3 binormal = normalize(cross(input.Normal, input.Tangent));
	float3 sampledNormal = gNormalMap.Sample(gSamPoint, input.Uv);
	float3x3 tangentSpaceAxis = { input.Tangent, binormal, input.Normal };
	input.Normal = float3(2 * sampledNormal.x - 1.f, 2 * sampledNormal.y - 1.f, 2 * sampledNormal.z - 1.f);
	return /*input.Normal =*/ mul(input.Normal, tangentSpaceAxis);
}

float4 CalculateLambert(VS_OUTPUT input)
{
	return SampleTextureColor(input.Uv) * gLightIntensity / gPi;
}

float4 CalculatePhong(VS_OUTPUT input)
{
	float3 reflect = gLightDirection - mul(mul(2.f, dot(gLightDirection, input.Normal)), input.Normal);
	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInvMatrix[3].xyz);
	float cosAlpha = dot(reflect, -viewDirection);
	cosAlpha = saturate(cosAlpha);

	float3 ks = gSpecularMap.Sample(gSamPoint, input.Uv);
	float exp = gGlossinessMap.Sample(gSamPoint, input.Uv);
	return float4(mul(ks, pow(cosAlpha, exp * gShininess)), 1.f);
}

//-----------------------------
// Pixel Shader
//-----------------------------

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	if (gUsingNormalMap)
	{
		input.Normal = CalculateNormalFromMap(input);
	}
	
	// ObservedArea
	float observedArea = saturate(dot(input.Normal, -gLightDirection));
	if (gShading == 0)	return float4(observedArea, observedArea, observedArea, 1.f);

	// Lambert
	float4 lambert = CalculateLambert(input);
	if (gShading == 1) return lambert * observedArea;

	// Phong
	float4 phong = CalculatePhong(input);
	if (gShading == 2) return phong * observedArea;
	
	// Combined
	return (phong + lambert) * observedArea + float4( .025f, .025f, .025f, .0f);
}

//-----------------------------
// Technique
//-----------------------------

technique11 TheGrandTechnique
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

technique11 TheGrandTechniqueFRONTCULL
{
	pass P0
	{
		SetRasterizerState(gRasterizerStateFRONTCULL);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
};

technique11 TheGrandTechniqueNOCULL
{
	pass P0
	{
		SetRasterizerState(gRasterizerStateNOCULL);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
};