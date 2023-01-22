
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : WorldMatrix;
float4x4 gInvViewMatrix : InvViewMatrix;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossMap : GlossMap;

SamplerState gSampler;

RasterizerState gRasterizerState
{
    CullMode = none;
    FrontCounterClockwise = true;
};

//--------------------------------------------------
//BlendState
//--------------------------------------------------

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

//--------------------------------------------------
//DepthStencil
//--------------------------------------------------


DepthStencilState gDepthStencilState
{
    DepthEnable = true;
    DepthWriteMask = zero;
    DepthFunc = less;
    StencilEnable = false;
};


const float1 PI = 3.1415927f;
    
    //Light
const float3 g_LightDirection = { 0.577f, -0.577f, 0.577f };
const float1 g_LightIntensity = 7.0f;
const float3 g_AmbientColor = { .025f, .025f, .025f };

//---------------------------------
//Input/Output structs
//----------------------------------

struct VS_INPUT
{
	float3 Position : POSITION;
    float2 UV : TEXTCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
    float2 UV : TEXTCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float4 WorldPosition : WORLDPOSITION;
};


//Vertex Shader
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4( input.Position, 1.f ), gWorldViewProj);
    output.UV = input.UV;
    output.Normal = mul(normalize(input.Normal), (float3x3) gWorldMatrix);
    output.Tangent = mul(normalize(input.Tangent), (float3x3) gWorldMatrix);
    output.WorldPosition = mul(float4(input.Position, 1.f), gWorldMatrix);
	return output;
}

float4 MaxToOne(float4 v)
{
    float maxValue = max(v.r, max(v.g, v.b));
    if (maxValue > 1.f)
    {
        return float4(v / maxValue);
    }
    return v;    
}

float4 PixelShading(VS_OUTPUT input)
{       
	//sample diffuse color
    float4 diffuseSample = gDiffuseMap.Sample(gSampler, input.UV); //Diffuse

    float4 returnColor = diffuseSample;
    returnColor = MaxToOne(returnColor);
    
    return returnColor;
}

//Pixel shader
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    float4 color = PixelShading(input);
    
    //float3 testColor = (0, 0, 0);
    return float4(color);
}

//--------------------------------------------------
//Technique
//--------------------------------------------------

technique11 PointTechnique
{
	pass P0
	{
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

