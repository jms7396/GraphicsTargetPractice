struct AimVertToPixel
{
	float4 position	: SV_POSITION;
	float4 color	: COLOR;
};

float4 main(AimVertToPixel input) : SV_TARGET
{
	// Just return an arbitrary color
	return float4(0.8f, 0.8f, 1.0f, 1.0f);
}