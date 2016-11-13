// Constant Buffer for data recieved from C++ side
cbuffer externalData : register(b0) 
{
	matrix world;
	matrix view;
	matrix projection;
};

// Only need to take in a Position
struct VertexShaderInput
{
	float3 position	: POSITION;
};

// Only need to output a Position
struct VertexToPixel
{
	float4 position	: SV_POSITION;
};

float4 main( VertexShaderInput input )
{
	VertexToPixel output;

	// Calculate output position based on matrix tranformations
	matrix worldViewProj = mul(mul(world, view), projection);
	output.position = mul(float4(input.position, 1.0f), worldViewProj);

	return output;
}