cbuffer vertShaderData : register(b0) 
{
	matrix world;
	matrix view;
	matrix projection;
}

struct AimVertShaderInput
{
	float3 position	: POSITION;
	float4 color	: COLOR;
};

struct AimVertToPixel 
{
	float4 position	: SV_POSITION;
	float4 color	: COLOR;
};

AimVertToPixel main( AimVertShaderInput input)
{
	AimVertToPixel output;
	
	// Do some matrix math to get the transformation matrix
	matrix worldViewProj = mul(mul(world, view), projection);

	// Set output position and colors
	output.position = mul(float4(input.position, 1.0f), worldViewProj);
	output.color = input.color;

	// Send output struct on to pixel shader
	return output;
}