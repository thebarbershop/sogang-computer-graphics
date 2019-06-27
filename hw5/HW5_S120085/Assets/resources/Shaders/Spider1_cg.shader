Shader "HLSL/Spider1_cg"
{
	Properties
	{
		u_material_ambient_color("Ambient", Color) = (0,0,0,1)
		u_material_diffuse_color("Diffuse", Color) = (0,0,0,1)
		u_material_specular_color("Specular", Color) = (0,0,0,1)
		u_material_emissive_color("Emissive", Color) = (0,0,0,1)
		u_material_specular_exponent("Specular", float) = 30
		u_base_texture("Texture", 2D) = "white" {}
	}
	SubShader
	{
		Tags { "RenderType" = "Opaque" }
		LOD 100

		Pass
		{
			CGPROGRAM
			#pragma vertex vert
			#pragma fragment frag
			// make fog work
			#pragma multi_compile_fog

			#include "UnityCG.cginc"

			#define NUMBER_OF_LIGHTS_SUPPORTED 4
		
			float4 u_light_position[NUMBER_OF_LIGHTS_SUPPORTED]; // assume point or direction in EC in this example shader
			fixed4 u_light_ambient_color[NUMBER_OF_LIGHTS_SUPPORTED], u_light_diffuse_color[NUMBER_OF_LIGHTS_SUPPORTED], u_light_specular_color[NUMBER_OF_LIGHTS_SUPPORTED];
			float4 u_light_light_attenuation_factors[NUMBER_OF_LIGHTS_SUPPORTED]; // compute this effect only if .w != 0.0
			float3 u_light_spot_direction[NUMBER_OF_LIGHTS_SUPPORTED];//빛의 방향
			float u_light_spot_exponent[NUMBER_OF_LIGHTS_SUPPORTED];
			float u_light_spot_cutoff_angle[NUMBER_OF_LIGHTS_SUPPORTED];
			float u_light_light_on[NUMBER_OF_LIGHTS_SUPPORTED];
			float u_light_slit_count[NUMBER_OF_LIGHTS_SUPPORTED];

			float screen_frequency;
			float screen_width;
			int screen_draw, screen_effect;

			fixed4 u_material_ambient_color;
			fixed4 u_material_diffuse_color;
			fixed4 u_material_specular_color;
			fixed4 u_material_emissive_color;
			float u_material_specular_exponent;

			struct appdata
			{
				float3 v_position : POSITION;
				float3 v_normal : NORMAL;
				float2 v_tex_coord : TEXCOORD0;
			};

			struct v2f
			{
				float4 vertex : SV_POSITION;
				float4 v_position_EC : COLORPOS;	//미리 정의되지 않은 field. 임의의 field 사용 가능
				float2 v_position_sc : SCREENEFFECT;	//미리 정의되지 않은 field. 임의의 field 사용 가능
				float3 v_normal_EC : NORMAL;
				float2 v_tex_coord : TEXCOORD0;
			};

            struct twoColor
            {
                fixed4 primary : COLOR;
                fixed4 secondary: COLOR;
            };


			uniform fixed4 u_global_ambient_color;	//0.2f, 0.2f, 0.2f, 1.0
			

			uniform sampler2D u_base_texture;

			uniform int u_flag_texture_mapping;
			uniform int u_flag_fog;


			#define FOG_COLOR float4(0.7, 0.7, 0.7, 1.0)
			#define FOG_NEAR_DISTANCE 350.0
			#define FOG_FAR_DISTANCE 700.0
			
			fixed4 mix(fixed4 color1, fixed4 color2, float factor) {//두 색을 섞는 함수. r = x * (1-a) + y*a
				fixed4 result;
				result.x = color1.x * (1 - factor) + color2.x * factor;
				result.y = color1.y * (1 - factor) + color2.y * factor;
				result.z = color1.z * (1 - factor) + color2.z * factor;
				result.w = 1.0f;
				return result;
			}

			twoColor lighting_equation_textured(float3 P_EC, float3 N_EC, fixed4 base_color) {

				const float zero_f = 0.0;
				const float one_f = 1.0;
				
                /* 정반사 색깔 분리모드 */
				twoColor color;
				float local_scale_factor, tmp_float;
				float3 L_EC;

				color.primary = u_material_emissive_color + u_global_ambient_color * base_color;
                color.secondary = 0;

				for (int i = 0; i < NUMBER_OF_LIGHTS_SUPPORTED; i++) {
					if (u_light_light_on[i] == 0) { continue; }

					local_scale_factor = one_f;
					if (u_light_position[i].w != zero_f) { // point light source
						L_EC = u_light_position[i].xyz - P_EC.xyz;

						if (u_light_light_attenuation_factors[i].w != zero_f) {
							float4 tmp_vec4;

							tmp_vec4.x = one_f;
							tmp_vec4.z = dot(L_EC, L_EC);
							tmp_vec4.y = sqrt(tmp_vec4.z);
							tmp_vec4.w = zero_f;
							local_scale_factor = one_f / dot(tmp_vec4, u_light_light_attenuation_factors[i]);
						}

						L_EC = normalize(L_EC);


						if (u_light_spot_cutoff_angle[i] < 180.0) { // [0.0, 90.0] or 180.0
							float spot_cutoff_angle = clamp(u_light_spot_cutoff_angle[i], zero_f, 90.0);
							float3 spot_dir = normalize(u_light_spot_direction[i]);

							tmp_float = dot(-L_EC, spot_dir);
							if (tmp_float >= cos(radians(spot_cutoff_angle))) {
								tmp_float = pow(tmp_float, u_light_spot_exponent[i]);
								tmp_float *= cos(u_light_slit_count[i] * 4 * acos(tmp_float));
							}
							else 
								tmp_float = zero_f;
							
							local_scale_factor *= tmp_float;
						}
					}
					else {  // directional light source
						L_EC = normalize(u_light_position[i].xyz);

					}


					if (local_scale_factor > zero_f) {
						twoColor local_color;
                        local_color.primary = u_light_ambient_color[i] * u_material_ambient_color;
                        local_color.secondary = 0;

						tmp_float = dot(N_EC, L_EC);
						if (tmp_float > zero_f) {
							local_color.primary += u_light_diffuse_color[i] * base_color*tmp_float;

							float3 H_EC = normalize(L_EC - normalize(P_EC));
							tmp_float = dot(N_EC, H_EC);
							if (tmp_float > zero_f) {
								local_color.secondary += u_light_specular_color[i]
									* u_material_specular_color*pow(tmp_float, u_material_specular_exponent);
						}
						color.primary += local_scale_factor * local_color.primary;
                        color.secondary += local_scale_factor * local_color.secondary;
						}
					}
				}
                

				return color;
			}


			v2f vert(appdata i)
			{
				v2f o;

				o.vertex = UnityObjectToClipPos(i.v_position); // mul (UNITY_MATRIX_MV, float4 (pos, 1.0)).xyz 와 동일.
				o.v_position_EC = float4(UnityObjectToViewPos(i.v_position), 1.0);
				o.v_normal_EC = normalize(mul(UNITY_MATRIX_IT_MV, float4(i.v_normal, 0)).xyz);
				o.v_tex_coord = i.v_tex_coord;
				o.v_position_sc.x = i.v_position.x;
				o.v_position_sc.y = i.v_position.y;
				return o;
			}

			fixed4 frag(v2f i) : SV_Target
			{
				// sample the texture
				fixed4 color;

				fixed4 base_color;
				twoColor shaded_color;
				float fog_factor;

				if (u_flag_texture_mapping == 1)
					base_color = tex2D(u_base_texture, i.v_tex_coord);
				else
					base_color = u_material_diffuse_color;

				shaded_color = lighting_equation_textured(i.v_position_EC, normalize(i.v_normal_EC), base_color);

				if (u_flag_fog == 1) {
					fog_factor = (FOG_FAR_DISTANCE - length(i.v_position_EC.xyz)) / (FOG_FAR_DISTANCE - FOG_NEAR_DISTANCE);
					fog_factor = clamp(fog_factor, 0.0, 1.0);
					color = mix(FOG_COLOR, shaded_color.primary, fog_factor) + shaded_color.secondary;
				}
				else
					color = shaded_color.primary + shaded_color.secondary;


				return color;
			}
			ENDCG
		}
	}
}