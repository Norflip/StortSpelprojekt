#include "CommonBuffers.hlsl"

Texture2D rgbTexture : register(t0);
Texture2D glowTexture : register(t1);

SamplerState glowSampler;
#include "IO.hlsl"

struct PS_INPUT_GLOW
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT_GLOW input) : SV_TARGET
{
    float4 tempColor = 0;
    float4 tempGlowColor = 0;
    float3 blurColor = 0;
    bool brightPixelFound = false;
    
    //brughtness 1 = max of one color
    float brightness = 1;
    
    //pixel range
    int range = 2;
    
    for (int x = -range; x <= range; x++)
    {
        for (int y = -range; y <= range; y++)
        {
            tempColor = rgbTexture.Sample(glowSampler, input.uv, int2(x, y));
            tempGlowColor = glowTexture.Sample(glowSampler, input.uv, int2(x, y));
            
            //length(tempColor.rgb) becomes vector for checking of brightness
            
            //if (length(tempColor.rgb) >= brightness && tempGlowColor.a != 0.f)
            if (length(tempColor.rgb) >= brightness && tempColor.a != 0.f)
            {
                brightPixelFound = true;
            }
            blurColor += tempColor.rgb;
        }
    }
    
    blurColor /= ((range * 2 + 1) * (range * 2 + 1));
    
    if (brightPixelFound == true)
    {
        return float4(blurColor, 1.0f);
    }
    
    return rgbTexture.Sample(glowSampler, input.uv);
}