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
            struct PS_INPUT { float4 pos : SV_POSITION; float4 col : COLOR; float2 uv : TEXCOORD; float4 data : DATA; };

            float sd_rounded_box(float2 p, float2 b, float r) {
                float2 q = abs(p) - b + r;
                return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;
            }

            float4 main(PS_INPUT input) : SV_TARGET {
                float2 size = input.data.xy;
                float radius = input.data.z;
                float type = input.data.w;

                float2 p = (input.uv - 0.5) * size;

                float d;
                if (type > 0.5) {
                    d = length(p) - radius;
                } else {
                    d = sd_rounded_box(p, size * 0.5, radius);
                }

                float alpha = 1.0 - smoothstep(-1.0, 1.0, d);

                if (alpha <= 0.001) discard;

                return float4(input.col.rgb, input.col.a * alpha);
            }
        )";
    }
}