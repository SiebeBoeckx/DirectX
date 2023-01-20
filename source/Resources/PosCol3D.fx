
float4x4 gWorldViewProj : WorldViewProjection;

float4x4 gWorldMatrix : WorldMatrix;

float4x4 gInvViewMatrix : InvViewMatrix;

Texture2D gDiffuseMap : DiffuseMap;

Texture2D gNormalMap : NormalMap;

Texture2D gSpecularMap : SpecularMap;

Texture2D gGlossMap : GlossMap;

SamplerState gSampler;

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
    output.Normal = mul(input.Normal, (float3x3)gWorldMatrix);
    output.Tangent = mul(input.Tangent, (float3x3)gWorldMatrix);
    output.WorldPosition = mul(float4(input.Position, 1.f), gWorldMatrix);
	return output;
}

float3 MaxToOne(float3 v)
{
    float maxValue = max(v.r, max(v.g, v.b));
    if (maxValue > 1.f)
    {
        return float3(v / maxValue);
    }
    return v;    
}

float3 Phong(float specular, float exp, float3 l, float3 v, float3 n)
{
	//todo: W3
	//assert(false && "Not Implemented Yet");
    float3 reflectL = reflect(l, n);
    float cosAlpha = max(0.f, dot(reflectL, v));
    float value = specular * pow(cosAlpha, exp);
    return float3(value, value, value);
}

float3 PixelShading(VS_OUTPUT input)
{    
    const float1 PI = 3.1415927f;
    
    //Light
    const float3 lightDirection = (0.577f, -0.577f, 0.577f);
    const float1 lightIntensity = 7.0f;
    const float3 ambientColor = (.025f, .025f, .025f);
    
	//normal map
    
    //if (true) //using normal map?
    //{
    float3 binormal = normalize(cross(input.Normal, input.Tangent));

        float3x3 tangentSpaceAxis =
        {
             normalize(input.Tangent),
	         binormal,
	         normalize(input.Normal),
        };

        float4 normalSample = gNormalMap.Sample(gSampler, input.UV); //normal
        float3 normalColor = normalSample.xyz; //normal
        
        normalColor = 2.f * normalColor - 1.f;

        float3 tangentSpaceNormal = normalize(mul(normalColor, tangentSpaceAxis));
    //}
    
	//observed area
    float1 cosineLaw = dot(tangentSpaceNormal, -lightDirection);
    
    if(cosineLaw < 0)
    {
        return ambientColor;
    }
    cosineLaw = saturate(cosineLaw);

	//sample diffuse color
    float4 diffuseSample = gDiffuseMap.Sample(gSampler, input.UV); //Diffuse
    float3 diffuseColor = diffuseSample.rgb / PI; //Diffuse

	//Phong
    const float1 shininess = 25.f;
    float3 viewDirection = normalize(input.WorldPosition.xyz - gInvViewMatrix[3].xyz);
    
    float1 specular = gSpecularMap.Sample(gSampler, input.UV).r; //Specular
    float1 phongExp = gGlossMap.Sample(gSampler, input.UV).r * shininess; //Phong exponent
    
    float3 phongColor = Phong(specular, phongExp, lightDirection, viewDirection, tangentSpaceNormal);
    
    float3 returnColor = lightIntensity * diffuseColor * cosineLaw + phongColor + ambientColor;
    //float3 returnColor = diffuseColor;
    returnColor = MaxToOne(returnColor);
    
    return returnColor;
}

//Pixel shader
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    float3 color = PixelShading(input);
    
    //float3 testColor = (0, 0, 0);
    return float4(color, 1.f);
}

//--------------------------------------------------
//Technique
//--------------------------------------------------

technique11 PointTechnique
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

