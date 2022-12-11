//---------------------------------
//Input/Output structs
//----------------------------------
struct VS_INPUT
{
	Vector3 Position : POSITION;
	Vector3 Color : COLOR;
};

struct VS_OUTPUT
{
	Vector4 Position : SV_POSITION;
	Vector3 Color : COLOR;
};

//Vertex Shader
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = Vector4{ input.Position, 1.f };
	output.Color = input.Color;
	return output;
}

//Pixel shader
Vector4 PS(VS_OUTPUT input) : SV_TARGET
{
	return Vector4(input.Color, 1.f);
}