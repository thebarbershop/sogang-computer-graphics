Shader "HLSL/Simple_cg"
{
    Properties
    {

    }
    SubShader
    {
        Tags { "RenderType"="Opaque" }
        LOD 100

        Pass
        {
            CGPROGRAM
            #pragma vertex vert	//vert 라는 함수를 vertex shader로 사용
            #pragma fragment frag //frag 라는 함수를 framgment shader 로 사용

            #include "UnityCG.cginc"

			uniform float tester[4];
            struct v2f
            {
                float4 vertex : SV_POSITION;	//SV_POSITION : 실제로 파일이 그려지는 위치
				float4 v_color : PRICOLOR;	//primitive color를 표시하기 위한 임의의 field.
            };

            fixed4 _Primitive_color;

            v2f vert (float4 vertex : POSITION)	//POSITION : 이것이 붙은 변수에 position 데이터를 넣을 것을 명시
            {
                v2f o;
                o.vertex = UnityObjectToClipPos(vertex);
				o.v_color = _Primitive_color;
                return o;
            }

            fixed4 frag (v2f i) : SV_Target	//SV_Target : 출력되는 색상(리턴값)이 실제 색임을 나타낸다.
            {
                // sample the texture
                fixed4 color = i.v_color;

                return color;
            }
            ENDCG
        }
    }
}
