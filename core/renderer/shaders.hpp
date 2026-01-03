#pragma once

namespace selaura::shaders {
    namespace geometry {
        inline constexpr char vertex[] = R"(
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

            struct VS_OUTPUT {
                float4 pos : SV_POSITION;
                float4 col : COLOR;
                float2 uv : TEXCOORD;
                float4 data : DATA;
                float4 radii : RADII;
            };

            VS_OUTPUT main(VS_INPUT input) {
                VS_OUTPUT output;
                output.pos = mul(float4(input.pos, 1.0f), mat);
                output.col = input.col;
                output.uv = input.uv;
                output.data = input.data;
                output.radii = input.radii;
                return output;
            }
        )";

        inline constexpr char pixel[] = R"(
            Texture2DArray tex : register(t0);
            SamplerState smp : register(s0);

            struct PS_INPUT {
                float4 pos : SV_POSITION;
                float4 col : COLOR;
                float2 uv : TEXCOORD;
                float4 data : DATA;
                float4 radii : RADII;
            };

            float median(float r, float g, float b) {
                return max(min(r, g), min(max(r, g), b));
            }

            float sd_rounded_box(float2 p, float2 b, float4 r) {
                float radius;
                if (p.x < 0.0) {
                    radius = (p.y < 0.0) ? r.x : r.w;
                } else {
                    radius = (p.y < 0.0) ? r.y : r.z;
                }

                float2 q = abs(p) - b + radius;
                return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - radius;
            }

            float4 main(PS_INPUT input) : SV_TARGET {
                float2 size = input.data.xy;
                float stroke_width = input.data.z;
                float type = input.data.w;

                if (type >= 3.0) {
                    float px_range = 8.0;
                    float2 uv = input.uv;
                    float2 dx = ddx(uv);
                    float2 dy = ddy(uv);
                    float2 uv_offsets[4] = { float2(-0.125, -0.375), float2(0.375, -0.125), float2(0.125, 0.375), float2(-0.375, 0.125) };
                    float total_opacity = 0.0;
                    [unroll]
                    for (int i = 0; i < 4; i++) {
                        float2 s_uv = uv + uv_offsets[i].x * dx + uv_offsets[i].y * dy;
                        float3 msd = tex.SampleLevel(smp, float3(s_uv, 0), 0).rgb;
                        float sd = median(msd.r, msd.g, msd.b);
                        float2 sig = float2(ddx(sd), ddy(sd));
                        total_opacity += saturate(((sd - 0.5) / (length(sig) + 0.0001)) + 0.5);
                    }
                    float opacity = pow(smoothstep(0.05, 0.95, total_opacity / 4.0), 1.0 / 1.35);
                    if (opacity < 0.01) discard;
                    return float4(input.col.rgb, input.col.a * opacity);
                }

                if (type >= 2.0) {
                    float4 color = input.col * tex.Sample(smp, float3(input.uv, floor(stroke_width + 0.05)));
                    if (color.a < 0.01) discard;
                    return color;
                }

                float2 p = (input.uv - 0.5) * size;
                float2 half_size = size * 0.5;

                float d_outer = sd_rounded_box(p, half_size, input.radii);
                float aa = fwidth(d_outer);
                float alpha = 0.0;

                if (stroke_width > 0.001) {
                    float4 inner_radii = max(input.radii - stroke_width, 0.0);
                    float2 inner_half_size = half_size - stroke_width;

                    float d_inner = sd_rounded_box(p, inner_half_size, inner_radii);

                    alpha = smoothstep(aa, -aa, d_outer) - smoothstep(aa, -aa, d_inner);
                } else {
                    alpha = smoothstep(aa, -aa, d_outer);
                }

                if (alpha <= 0.001) discard;
                return float4(input.col.rgb, input.col.a * alpha);
            }
        )";
    }
}