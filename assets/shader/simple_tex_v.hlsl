struct VSInput
{
	float3 Position : POSITION;
	float4 Color : COLOR; 
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
};

cbuffer Transform : register(b0)
{
	float4x4 _world : packoffset(c0); // ワールド行列
	float4x4 _view : packoffset(c4); // view行列
	float4x4 _proj : packoffset(c8); // プロジェクション(射影)行列
}

VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput) 0;
	
	float4 localPos = float4(input.Position, 1.0f);
	float4 worldPos = mul(_world, localPos);
	float4 viewPos = mul(_view, worldPos);
	float4 projPos = mul(_proj, viewPos);
	
	output.Position = projPos;
	output.Color = input.Color;
	
	return output;
}