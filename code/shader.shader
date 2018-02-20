struct dir_light
{
    float4 Direction;
    float4 Diffuse;
    float4 Ambient;
};

cbuffer ConstantBuffer
{
        float4x4 Model;
	float4x4 View;
        float4x4 Projection;
        dir_light DirLight[1];
}

struct VOut
{
        // float4 color : COLOR;
        float2 texcoord : TEXCOORD;    // texture coordinates
        float4 position : SV_POSITION;
		float4 fragpostion : POSITION;
		float4 normal : NORMAL; 
};
    
Texture2D Texture;
SamplerState ss; 
    

VOut VShader(float4 position : POSITION, float4 normal : NORMAL, float2 texcoord : TEXCOORD)
{
	VOut output;
    
	output.fragpostion = mul(Model, position);
 
	output.normal = normal; 
	output.position = mul(Projection * View, output.fragpostion);

	//output.color = DirLight[0].Ambient;
        
	//output.color += DirLightCalc(rotation, normal, DirLight[0].Direction, DirLight[0].Diffuse);
        
	output.texcoord = texcoord;    
        
	return output;
}
    
float4 PShader(float2 texcoord : TEXCOORD, float4 position : SV_POSITION, float4 fragposition : POSITION, float4 normal : NORMAL) : SV_TARGET
{
	float ambientStrength = 0.1;
	float4 ambient = ambientStrength * DirLight[0].Ambient;

	float4 norm = normalize(normal);
	float4 lightDir = normalize(DirLight[0].Direction - fragposition);
	float diff = max(dot(norm, lightDir), 0.0);
	float4 diffuse = diff * DirLight[0].Diffuse;

	float4 result = (ambient + diffuse); 
	return result; 	

}