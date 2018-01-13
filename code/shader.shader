struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
	float2 texcoord : TEXCOORD; 
};

Texture2D Texture;
SamplerState ss; 

VOut VShader(float4 position : POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD)
{
    VOut output;

    output.position = position;
    output.color = color;

	output.texcoord = texcoord;    // set the texture coordinates, unmodified

    return output;
}


float4 PShader(float4 position : SV_POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD) : SV_TARGET
{
    return color * Texture.Sample(ss, texcoord);
}
