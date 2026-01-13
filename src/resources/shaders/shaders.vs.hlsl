cbuffer projection : register(b0) {
    matrix mat;
};

struct VS_INPUT {
    float3 pos : POSITION;
    float4 col : COLOR;
    float2 uv : TEXCOORD;
    float4 data : DATA;
    float4 radii : RADII;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 uv : TEXCOORD;
    float4 data : DATA;
    float4 radii : RADII;
};

PS_INPUT main(VS_INPUT input) {
    PS_INPUT output;
    output.pos = mul(float4(input.pos, 1.0f), mat);
    output.col = input.col;
    output.uv = input.uv;
    output.data = input.data;
    output.radii = input.radii;
    return output;
}