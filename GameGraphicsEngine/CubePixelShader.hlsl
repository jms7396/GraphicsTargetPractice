
// Struct representing the data we expect to receive from earlier pipeline stages
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uvw			: TEXCOORD;
};

// Texture Related Data
Texture2D Sky			: register(t0);
SamplerState Sampler	: register(s0);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	return Sky.Sample(Sampler, input.uvw);
}