
float4 g_vAmbientColor = float4(0.1f, 0.1f, 0.1f, 1.f);

bool g_bSpecular : register(b0) = false;

float4 vSpecColor = {1.0f, 1.0f, 1.0f, 1.0f};

float g_fMaterialPower = 1.0f;

float3 g_vEyePos;	// the position of the eye(camera) in world space

float3 g_vLightDir = { 0.56568f, 0.70711f, 0.42426f };

// transformation matrices
float4x4 WorldViewProj	: WORLDVIEWPROJ;
float4x4 World			: WORLD;
float4x4 View			: VIEW;
float4x4 Proj			: PROJ;

float4x4 WorldView		: WORLDVIEW;


// initial and range of directional, point and spot lights within the light array
int iLightDirIni   = 0;
int iLightDirNum   = 1;
int iLightPointIni = 0;
int iLightPointNum = 0;


//--------------------------------------------------------------------------------
// textures
//--------------------------------------------------------------------------------

#include "../../../Shaders/TexDef.fxh"



//--------------------------------------------------------------------------------
// vertex & pixel shaders for vertex blending
//--------------------------------------------------------------------------------


#define NUM_MAX_BLEND_MATRICES_PER_VERTEX	2


//#include "VS_Default.fxh"
//#include "PS_Default.fxh"

//#include "QuickTest.fxh"

#include "../../../Shaders/SkyBox.fxh"


technique NullShader
{
	pass P0
	{
		VertexShader = NULL;
		PixelShader  = NULL;
	}
}


//--------------------------------------------------------------------------------
// Name: VS_NoLighting()
// Desc: vertex shader
//--------------------------------------------------------------------------------

void VS_NoLighting(
	float3 Pos  : POSITION, 
	float3 Normal : NORMAL,
	float4 Diffuse : COLOR0,
	float2 Tex0 : TEXCOORD0,
	out float4 oPos : POSITION,
	out float4 oDiffuse : COLOR0,
	out float2 oTex : TEXCOORD0,
	out float oDist : TEXCOORD1 )
{
	oPos = mul( float4(Pos,1), WorldViewProj );

	oTex = Tex0;

	float3 ViewPos = mul( float4(Pos,1), WorldView );
	oDist = ViewPos.z;

	// adjust transparency
	oDiffuse = float4( 1,1,1,1 );//Diffuse.r, Diffuse.g, Diffuse.b, 0.5f + fCamDir );
}


//--------------------------------------------------------------------------------
// Name: PS_NoLighting()
// Desc: pixel shader
//--------------------------------------------------------------------------------

float4 PS_NoLighting(
       float4 Diffuse : COLOR0,
       float2 Tex0 : TEXCOORD0,
	   float Dist : TEXCOORD1 ) : COLOR
{
	// just output texture color
	float fog_blend = ( Dist - 100 )/ 300;

	fog_blend = clamp( fog_blend, 0, 1 );

//	return tex2D(Sampler0, Tex0);
	return tex2D(Sampler0, Tex0) * (1.0 - fog_blend) + float4( 0.7, 0.7, 0.7, 1.0 ) * fog_blend;
}


technique NoLighting
{
	pass P0
	{
		VertexShader = compile vs_1_1 VS_NoLighting();
		PixelShader  = compile ps_2_0 PS_NoLighting();
//		CullMode = Ccw;
	}
}