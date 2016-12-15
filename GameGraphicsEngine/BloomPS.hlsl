
cbuffer Data : register(b0)
{
	float bloomIntensity;
	float originalIntensity;
	float bloomSaturation;
	float originalSaturation;
}


// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

float4 AdjustSaturation(float4 color, float saturation)
{
	float gray = dot(color, float3(0.3, 0.3, 0.3));
	return lerp(gray, color, saturation);
}

// Textures and such
Texture2D BlurThreshold	: register(t0);
Texture2D Original      : register(t1);
SamplerState Sampler	: register(s0);


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	float4 bloomColor = BlurThreshold.Sample(Sampler, input.uv);
	float4 originalColor = Original.Sample(Sampler, input.uv);
	
	//bloomColor = AdjustSaturation(bloomColor, bloomSaturation) * bloomIntensity;
	//originalColor = AdjustSaturation(originalColor, originalSaturation) * originalIntensity;
	originalColor *= (1 - saturate(bloomColor));

	return originalColor + bloomColor;
}