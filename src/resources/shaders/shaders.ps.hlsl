struct PS_INPUT {
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 uv : TEXCOORD;
    float4 data : DATA;
    float4 radii : RADII;
};

Texture2D tex : register(t0);
SamplerState smp : register(s0);

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float sd_rounded_box(float2 p, float2 b, float4 r) {
    float radius = (p.x < 0.0) ? ((p.y < 0.0) ? r.x : r.w) : ((p.y < 0.0) ? r.y : r.z);
    float2 q = abs(p) - b + radius;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - radius;
}

float4 main(PS_INPUT input) : SV_TARGET {
    float2 size = input.data.xy;
    float type = input.data.w;

    if (type >= 4.0) {
        float2 tex_dim;
        tex.GetDimensions(tex_dim.x, tex_dim.y);
        float2 texel = 1.0 / tex_dim;
        float offset = input.data.z + 0.5;

        float4 sum = 0;
        sum += tex.Sample(smp, input.uv + float2(offset, offset) * texel);
        sum += tex.Sample(smp, input.uv + float2(-offset, offset) * texel);
        sum += tex.Sample(smp, input.uv + float2(-offset, -offset) * texel);
        sum += tex.Sample(smp, input.uv + float2(offset, -offset) * texel);
        float4 blurred = sum * 0.25;

        float2 p = (input.uv - 0.5) * size;
        float d = sd_rounded_box(p, size * 0.5, input.radii);

        float soft_edge = fwidth(d) + (input.data.z * 0.5);
        float alpha = smoothstep(soft_edge, -soft_edge, d);

        return float4(blurred.rgb, blurred.a * alpha * input.col.a);
    }

    if (type >= 3.0) {
        float3 msd = tex.Sample(smp, input.uv).rgb;
        float sd = median(msd.r, msd.g, msd.b);
        float px_range = input.data.z;
        float2 screen_uv_range = float2(length(ddx(input.uv)), length(ddy(input.uv)));
        float screen_px_distance = (sd - 0.5) * (px_range / (screen_uv_range.x * 512.0));
        float opacity = saturate(screen_px_distance + 0.5);
        if (input.data.y > 0.5) opacity = sd > 0.5 ? 1.0 : 0.0;
        if (opacity < 0.001) discard;
        return float4(input.col.rgb, input.col.a * opacity);
    }

    if (type >= 2.0) return tex.Sample(smp, input.uv) * input.col;

    float2 p = (input.uv - 0.5) * size;
    float2 half_size = size * 0.5;
    float d_outer = sd_rounded_box(p, half_size, input.radii);
    float aa = fwidth(d_outer);
    float stroke_width = input.data.z;
    float alpha = (stroke_width > 0.001) ?
        (smoothstep(aa, -aa, d_outer) - smoothstep(aa, -aa, sd_rounded_box(p, half_size - stroke_width, max(input.radii - stroke_width, 0.0)))) :
        smoothstep(aa, -aa, d_outer);

    if (alpha <= 0.001) discard;
    return float4(input.col.rgb, input.col.a * alpha);
}

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

PS_INPUT vs_main(VS_INPUT input) {
    PS_INPUT output;
    output.pos = mul(float4(input.pos, 1.0f), mat);
    output.col = input.col;
    output.uv = input.uv;
    output.data = input.data;
    output.radii = input.radii;
    return output;
}