//Shader with Color only
///*With constant buffer*/
cbuffer ConstantBufferMatrix : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
    float offset[208];
};

struct VSInput
{
    float4 position : POSITION;
    float4 color : COLOR;
};
struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

PSInput VSMain(VSInput input)
{
    PSInput result;
    /*Change position vector to be 4 units to matrix calculations*/
    input.position.w = 1.0f;
    /*Calculate position of vertex agains the world view projection matrices*/
    result.position = mul(input.position, world);
    result.position = mul(result.position, view);
    result.position = mul(result.position, projection);
 
    result.color = input.color;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}

//
//Texture2D shaderTexture;
//SamplerState SamplerType;
//cbuffer ConstantBufferMatrix : register(b0)
//{
//    float4x4 world;
//    float4x4 view;
//    float4x4 projection;
//    float offset[208];
//};
//
//struct VSInput
//{
//    float4 position : POSITION;
//    float2 tex : TEXCOORD0;
//};
//struct PSInput
//{
//    float4 position : SV_POSITION;
//    float2 tex : TEXCOORD0;
//};
//
//
//
//PSInput VSMain(VSInput input)
//{
//    PSInput result;
//    /*Change position vector to be 4 units to matrix calculations*/
//    input.position.w = 1.0f;
//    /*Calculate position of vertex agains the world view projection matrices*/
//    result.position = mul(input.position, world);
//    result.position = mul(result.position, view);
//    result.position = mul(result.position, projection);
// 
//    result.tex = input.tex;
//
//    return result;
//}
//
//float4 PSMain(PSInput input) : SV_TARGET
//{
//     float4 textureColor;
//
//
//    // Sample the pixel color from the texture using the sampler at this texture coordinate location.
//    textureColor = shaderTexture.Sample(SampleType, input.tex);
//
//    return textureColor;
//}