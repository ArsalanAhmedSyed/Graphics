//--------------------------------------------------------------------------------------
// File: DX11 Framework.hlsl
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//Shader Variables
Texture2D texDiffuse : register(t0);
TextureCube texCube : register(t1);
SamplerState sampLinear : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
    
    //Diffuse varibles
    float4 DiffuseLight;
    float4 DiffuseMaterial;
    float3 DirectionToLight;
    
    //Ambinat varibles
    float pading;
    float4 AmbLight;
    float4 AmbMaterial;
    
    //Specular varibles
    float4 SpecMaterial;
    float4 SpecLight;
    float  SpecPower;
    float3 EyeWorldPos;
    
    //Fog varibles
    float FogStart;
    float FogEnd;
    
    //Boolean Check
    uint EnableFog;
    uint EnableSkybox;
}

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float3 PosW : POSITION0;
    float3 NormalW : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(float3 Pos : POSITION, float3 Normal : NORMAL, float2 TexCoord : TEXCOORD0)
{
    float4 pos4 = float4(Pos, 1.0f);
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    output.Pos = mul( pos4, World);                 //Pass world matrix
    output.PosW = output.Pos;                       //Store default world position
    output.Pos = mul( output.Pos, View );           //Pass view matrix
    output.Pos = mul( output.Pos, Projection );     //Pass Projection matrix
    
    //Setup the Normal Value
    Normal = normalize(Normal);
    output.NormalW = mul(float4(Normal, 0), World); //Pass world normal
    
    output.TexCoord = TexCoord;                     //Pass texture
    
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input) : SV_Target
{   
    input.NormalW = normalize(input.NormalW);                               //Normalize the normal

    //
    // Texture Sampling
    //
    float4 textureColour = texDiffuse.Sample(sampLinear, input.TexCoord);   //Sample Crate texture 
    float4 skyboxtextureColor = texCube.Sample(sampLinear, input.PosW);     //Sample Skybox texture
    
    //
    // Ambient Calculation
    //
    float4 Ambient = (AmbLight + pading) * AmbMaterial;                         //Final Ambient Value
    
    //
    // Diffuse Calculation
    //
    float4 TotalPotentialDif = DiffuseMaterial * DiffuseLight;                //Calculate Potential Diffuse
    float4 DifPercent = dot((DirectionToLight + pading), input.NormalW);     //Calculate Diffuse Percent
    float4 Diffuse = DifPercent * TotalPotentialDif;                        //Final Diffuse Value

    //
    // Specular Calculation
    //
    float3 lighDir = normalize(DirectionToLight);
    float3 toEye = EyeWorldPos - input.PosW;
    toEye = normalize(toEye);                                               //Get camera normal direction
    
    float3 ReflectDir = normalize(reflect(-lighDir, input.NormalW));
    float SpecularMax = max(dot(ReflectDir, toEye), 0);                     
    float SpecularIntensity = pow(SpecularMax, SpecPower);                  //Calculate specular Intensity
    float4 SpecPotential = SpecLight * SpecMaterial;                        //Calculate Specular Potential
    float4 Specular = SpecularIntensity * SpecPotential;                    //Get Specular

    //
    // Linear Fog Calculation
    //
    float fogDensity = 2.0f;
    float distValue = distance(EyeWorldPos, input.PosW);                    //get distance between world and camera
    float linearFog = (FogEnd - distValue) / (FogEnd - FogStart);           //Get linear or fog factor 
    float4 fogColour = float4(0.5, 0.5, 0.5, 1.0);                          //Set fog colour (gray)
    float4 finalFogColour = linearFog * fogColour;                          //Final Fog value
    
    
    //
    // return value Check
    //
    if (EnableSkybox == 1)
    {
        return skyboxtextureColor;
    }
    else if (EnableFog == 1)
    {
        return (Ambient + Diffuse + Specular + textureColour) * finalFogColour;
    }
    else
    {
        return (Ambient + Diffuse + Specular + textureColour);
    }
}
