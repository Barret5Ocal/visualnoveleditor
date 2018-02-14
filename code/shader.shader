cbuffer ConstantBuffer
{
    float4x4 final;
	float4x4 rotation;    // the rotation matrix
    float4 lightvec;      // the light's vector
    float4 lightcol;      // the light's color
    float4 ambientcol;    // the ambient light's color
}

struct VOut
{
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;    // texture coordinates
    float4 position : SV_POSITION;
};

Texture2D Texture;
SamplerState ss; 

VOut VShader(float4 position : POSITION, float4 normal : NORMAL, float2 texcoord : TEXCOORD)
{
    VOut output;

    output.position = mul(final, position);
    output.color = ambientcol;

	float4 norm = normalize(mul(rotation, normal));
    float diffusebrightness = saturate(dot(norm, lightvec));
    output.color += lightcol * diffusebrightness;

	output.texcoord = texcoord;    // set the texture coordinates, unmodified

    return output;
}

float4 PShader(float4 color : COLOR, float2 texcoord : TEXCOORD) : SV_TARGET
{
	float4 newcolor = color * Texture.Sample(ss, texcoord);
    newcolor.a = 1.0f;
    return newcolor;
}