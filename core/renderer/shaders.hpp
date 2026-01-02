#pragma once

namespace selaura::shaders {
    namespace geometry {
        inline constexpr char vertex[] = R"(
            cbuffer projection : register(b0) { matrix mat; };
            struct VS_INPUT { float3 pos : POSITION; float4 col : COLOR; float2 uv : TEXCOORD; float4 data : DATA; };
            struct VS_OUTPUT { float4 pos : SV_POSITION; float4 col : COLOR; float2 uv : TEXCOORD; float4 data : DATA; };
            VS_OUTPUT main(VS_INPUT input) {
                VS_OUTPUT output;
                output.pos = mul(float4(input.pos, 1.0f), mat);
                output.col = input.col;
                output.uv = input.uv;
                output.data = input.data;
                return output;
            }
        )";

        inline constexpr char pixel[] = R"(
            Texture2DArray tex : register(t0);
            SamplerState smp : register(s0);

            struct PS_INPUT { float4 pos : SV_POSITION; float4 col : COLOR; float2 uv : TEXCOORD; float4 data : DATA; };

            float sd_rounded_box(float2 p, float2 b, float r) {
                float2 q = abs(p) - b + r;
                return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;
            }

            float median(float r, float g, float b) {
                return max(min(r, g), min(max(r, g), b));
            }

            float4 main(PS_INPUT input) : SV_TARGET {
                float2 size = input.data.xy;
                float radius = input.data.z;
                float type = input.data.w;

                if (type >= 3.0) {
                    float px_range = 8.0;
                    float2 uv = input.uv;

                    float2 dx = ddx(uv);
                    float2 dy = ddy(uv);

                    float2 uv_offsets[4] = {
                        float2(-0.125, -0.375), float2(0.375, -0.125),
                        float2(0.125, 0.375),  float2(-0.375, 0.125)
                    };

                    float2 uv_grad = fwidth(uv);
                    float2 safe_uv_min = uv - uv_grad * 0.5;
                    float2 safe_uv_max = uv + uv_grad * 0.5;

                    float total_opacity = 0.0;

                    [unroll]
                    for (int i = 0; i < 4; i++) {
                        float2 sample_uv = uv + uv_offsets[i].x * dx + uv_offsets[i].y * dy;

                        float3 msd = tex.SampleLevel(smp, float3(sample_uv, 0), 0).rgb;
                        float sd = median(msd.r, msd.g, msd.b);

                        float2 sig_dist = float2(ddx(sd), ddy(sd));
                        float pixel_dist = (sd - 0.5) / (length(sig_dist) + 0.0001);

                        total_opacity += saturate(pixel_dist + 0.5);
                    }

                    float opacity = total_opacity / 4.0;

                    opacity = smoothstep(0.05, 0.95, opacity);

                    float weight_adjustment = 1.0 / lerp(1.25, 1.45, saturate(1.0 - opacity));
                    opacity = pow(opacity, weight_adjustment);

                    if (opacity < 0.01) discard;

                    return float4(input.col.rgb, input.col.a * opacity);
                }

                float2 p = (input.uv - 0.5) * size;
                float d = sd_rounded_box(p, size * 0.5, min(radius, min(size.x, size.y) * 0.5));
                float alpha = 1.0 - smoothstep(-fwidth(d), fwidth(d), d);
                if (alpha <= 0.001) discard;

                float4 color = input.col;
                if (type >= 2.0) {
                    float frame = floor(type - 2.0);
                    color *= tex.Sample(smp, float3(input.uv, frame));
                }

                return float4(color.rgb, color.a * alpha);
            }
        )";
    }
}