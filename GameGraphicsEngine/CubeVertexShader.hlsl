
// Constant Buffer
cbuffer externalData : register(b0)
{
	matrix view;
	matrix projection;
};

// Struct representing a single vertex worth of data
struct VertexShaderInput
{ 
	float3 position		: POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
};

// Struct representing the data we're sending down the pipeline
struct VertexToPixel
{
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float2 uvw			: TEXCOORD;
};

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;

	// Copy the view matrix and remove translations
	matrix viewNoMove = view;
	viewNoMove._41 = 0;
	viewNoMove._42 = 0;
	viewNoMove._43 = 0;

	// Calculate output pos
	matrix viewProj = mul(viewNoMove, projection);
	output.position = mul(float4(input.position, 1.0f), viewProj);

	// ensure the vertex is at max depth
	output.position.z = output.position.w;

	// Pass through the UVs
	output.uvw = input.position;

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}