#include "EmbeddedGenericHLSL.hpp"
#include "EmbeddedHLSLShader.hpp"
#include "Graphics/Shader/GenericShaderDesc.hpp"
#include "Support/Log/DefaultLog.hpp"
#include <boost/algorithm/string/replace.hpp>

using namespace std;
using namespace boost;


class CEmbeddedHLSLEffectDesc
{
public:
	CEmbeddedHLSLShader vs;
	CEmbeddedHLSLShader ps;
//	const char *pTechniqueName;

//	const char *pVSName;
//	const char *pPSName;
//	const char *pVS;
//	const char *pPS;

public:

	CEmbeddedHLSLEffectDesc()
//		:
//	pTechniqueName("")
/*	pVSName(""),
	pPSName(""),
	pVS(""),
	pPS("")*/
	{}
/*
	bool IsValid() const
	{
		if( pVSName && 0 < strlen(pVSName)
		 && pPSName && 0 < strlen(pPSName)
		 && pVS && 0 < strlen(pVS)
		 && pPS && 0 < strlen(pPS) )
		{
			return true;
		}
		else
			return false;
	}*/

	bool IsValid() const
	{
		if( vs.IsValid()
		 && ps.IsValid() )
//		 && pTechniqueName && 0 < strlen(pTechniqueName) )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};


const char *CEmbeddedGenericHLSL::ms_pMatrix =
"#ifndef __MATRIX_FXH__\n"\
"#define __MATRIX_FXH__\n"\

// transformation matrices
"float4x4 WorldViewProj : WORLDVIEWPROJ;"\
"float4x4 World : WORLD;"\
"float4x4 View : VIEW;"\
"float4x4 Proj : PROJ;"\

"float4x4 WorldView : WORLDVIEW;\n"\

"#endif\n"; /* __MATRIX_FXH__ */


const char *CEmbeddedGenericHLSL::ms_pColorPair =
"#ifndef __COLORPAIR_H__\n"\
"#define __COLORPAIR_H__\n"\

//-----------------------------------------------------------------------------
// color pair (diffuse & specular)
//-----------------------------------------------------------------------------
"struct COLOR_PAIR"\
"{"\
	"float4 Color     : COLOR0;"\
	"float4 ColorSpec : COLOR1;"\
"};\n"\

"#endif\n"; /* __COLORPAIR_H__ */


const char *CEmbeddedGenericHLSL::ms_pTexDef =
"#ifndef __TEXDEF_FXH__\n"\
"#define __TEXDEF_FXH__\n"\

//--------------------------------------------------------------------------------
// textures
//--------------------------------------------------------------------------------

"texture Texture0;"\
"sampler Sampler0 = sampler_state"\
"{"\
	"Texture   = <Texture0>;"\
	"MipFilter = LINEAR;"\
	"MinFilter = LINEAR;"\
	"MagFilter = LINEAR;"\
"};"\

"texture Texture1;"\
"sampler Sampler1 = sampler_state"\
"{"\
	"Texture   = <Texture1>;"\
	"MipFilter = LINEAR;"\
	"MinFilter = LINEAR;"\
	"MagFilter = LINEAR;"\
"};"\

"texture Texture2;"\
"sampler Sampler2 = sampler_state"\
"{"\
	"Texture   = <Texture2>;"\
	"MipFilter = LINEAR;"\
	"MinFilter = LINEAR;"\
	"MagFilter = LINEAR;"\
"};"\

"texture Texture3;"\
"sampler Sampler3 = sampler_state"\
"{"\
	"Texture   = <Texture3>;"\
	"MipFilter = LINEAR;"\
	"MinFilter = LINEAR;"\
	"MagFilter = LINEAR;"\
"};"\

"texture Texture4;"\
"sampler Sampler4 = sampler_state"\
"{"\
	"Texture   = <Texture4>;"\
	"MipFilter = LINEAR;"\
	"MinFilter = LINEAR;"\
	"MagFilter = LINEAR;"\
"};"\

"texture Texture5;"\
"sampler Sampler5 = sampler_state"\
"{"\
	"Texture   = <Texture5>;"\
	"MipFilter = LINEAR;"\
	"MinFilter = LINEAR;"\
	"MagFilter = LINEAR;"\
"};\n"\
"#endif\n"; /* __TEXDEF_FXH__ */



const char *CEmbeddedGenericHLSL::ms_pLightDef =
"#ifndef __LIGHTDEF_FXH__\n"\
"#define __LIGHTDEF_FXH__\n"\


//-----------------------------------------------------------------------------
// light types
//-----------------------------------------------------------------------------

"#define LIGHT_TYPE_NONE        0\n"\
"#define LIGHT_TYPE_POINT       1\n"\
"#define LIGHT_TYPE_SPOT        2\n"\
"#define LIGHT_TYPE_DIRECTIONAL 3\n"\
"#define LIGHT_NUM_TYPES        4\n"\


//-----------------------------------------------------------------------------
// variables
//-----------------------------------------------------------------------------

//initial and range of directional, point and spot lights within the light array
"int iLightDirIni   = 0;"\
"int iLightDirNum   = 0;"\
"int iLightPointIni = 1;"\
"int iLightPointNum = 0;"\
//int iLightSpotIni;
//int iLightSpotNum;


//-----------------------------------------------------------------------------
// light structure
//-----------------------------------------------------------------------------

"struct CLight"\
"{"\
	"int iType;"\

	// position in world space
	"float3 vPos;"\

	// position in view space
	"float3 vPosVS;"\

	"float3 vDir;"\
	"float4 vAmbient;"\
	"float4 vDiffuseUpper;"\
	"float4 vDiffuseLower;"\
	"float4 vSpecular;"\
	"float  fRange;"\

	//1, D, D^2;
	"float3 vAttenuation;"\

	//cos(theta/2), cos(phi/2), falloff
	"float3 vSpot;"\
"};\n"\

"#endif"; /* __LIGHTDEF_FXH__ */



const char *CEmbeddedGenericHLSL::ms_pQuaternion =
"#ifndef __Quaternion_FXH__\n"\
"#define __Quaternion_FXH__\n"\

// Use float4 as quaternion

"float length_sq( float4 src )"\
"{"\
	"return src.x*src.x + src.y*src.y + src.z*src.z + src.w*src.w;"\
"}"\


"float4 quat_inv( float4 src )"\
"{"\
//	Scalar fNorm = (Scalar)0.0;

	"float fNormSq = length_sq(src);"\

	"float4 inv;"\
//	if ( fNormSq > (Scalar)0.0 )
//	{
		"float fInvNormSq = 1.0 / fNormSq;"\
		"inv.x = -src.x*fInvNormSq;"\
		"inv.y = -src.y*fInvNormSq;"\
		"inv.z = -src.z*fInvNormSq;"\
		"inv.w =  src.w*fInvNormSq;"\
/*	}
	else
	{
		// return an invalid result to flag the error
		inv.x = (Scalar)0.0;
		inv.y = (Scalar)0.0;
		inv.z = (Scalar)0.0;
		inv.w = (Scalar)0.0;
	}*/

	// Replace to this after cofirming the above code works
//	return float4( -src.x, -src.y, -src.z, src.w ) / fNormSq;

	"return inv;"\
"}"\


"float4 quat_mul( float4 lhs, float4 rhs )"\
"{"\
    "float4 res;"\

    "res.w ="\
        "lhs.w * rhs.w -"\
        "lhs.x * rhs.x -"\
        "lhs.y * rhs.y -"\
        "lhs.z * rhs.z;"\

    "res.x ="\
        "lhs.w * rhs.x +"\
        "lhs.x * rhs.w +"\
        "lhs.y * rhs.z -"\
        "lhs.z * rhs.y;"\

    "res.y ="\
        "lhs.w * rhs.y +"\
        "lhs.y * rhs.w +"\
        "lhs.z * rhs.x -"\
        "lhs.x * rhs.z;"\

    "res.z ="\
        "lhs.w * rhs.z +"\
        "lhs.z * rhs.w +"\
        "lhs.x * rhs.y -"\
        "lhs.y * rhs.x;"\

    "return res;"\
"}\n"\

"#endif\n"; /* __Quaternion_FXH__ */


const char *CEmbeddedGenericHLSL::ms_pTransform =
"#ifndef __Transform_FXH__\n"\
"#define __Transform_FXH__\n"\

//#include "Quaternion.fxh"

"struct Transform"\
"{"\
	"float4 translation;"\

	/// quaternion
	"float4 rotation;"\
"};"\


"float3 mul( Transform transform, float3 rhs )"\
"{"\
	"float4 res = quat_mul( quat_mul( transform.rotation, float4( rhs.x, rhs.y, rhs.z, 0 ) ), quat_inv( transform.rotation ) );"\
	"return res.xyz + transform.translation.xyz / transform.translation.w;"\
"}"\


"float4 mul( Transform transform, float4 rhs )"\
"{"\
	"return float4( mul( transform, rhs.xyz / rhs.w ), 1 );"\
"}\n"\


"#endif\n"; /* __Transform_FXH__ */


const char *CEmbeddedGenericHLSL::ms_pQVertexBlendFunctions =
"#ifndef __QVertexBlendFunctions_FXH__\n"\
"#define __QVertexBlendFunctions_FXH__\n"\

//===========================================================================================
// SkinPoint
// Applies 4 matrix skinning to a single point.  The point passed in is changed as well as returned.
//
"float4 SkinPoint( inout float4 io_value, Transform blends[NUM_MAX_BLEND_TRANSFORMS],  int4 indices, float4 weights)"\
"{"\
	"int i;"\
	"float4 incoming_io_value = io_value;"\

	"if(weights[0] != -1 )"\
	"{"\
		"io_value = 0;"\
		// skin
//		for(i=0; i <4 ; i++)
		"for(i=0; i <NUM_MAX_BLEND_TRANSFORMS_PER_VERTEX ; i++)"\
		"{"\
			"io_value  += mul( blends[indices[3-i]], incoming_io_value ) * weights[i];"\
//			io_value  += mul( blends[indices[i]], incoming_io_value ) * weights[i];
		"}"\
	"}"\
	"return io_value;"\
"}\n"\

//===========================================================================================
// SkinVector
// Applies 4 matrix skinning to a vector.  The vector passed in is changed as well as returned.
//
"float3 SkinVector( inout float3 io_value, Transform blends[NUM_MAX_BLEND_TRANSFORMS], int4 indices, float4 weights)"\
"{"\
	"int i;"\
	"float3 incoming_io_value = io_value;"\

	"if(weights[0] != -1 )"\
	"{"\
		"io_value = 0;"\
    
		// skin
//		for(i=0; i <4 ; i++)
		"for(i=0; i <NUM_MAX_BLEND_TRANSFORMS_PER_VERTEX ; i++)"\
		"{"\
			"io_value  += mul( blends[indices[3-i]], incoming_io_value ) * weights[i];"\
//			io_value  += mul( blends[indices[i]], incoming_io_value ) * weights[i];
		"}"\

		"io_value = normalize(io_value);"\
	"}"\

	"return io_value;"\
"}\n"\

"#endif";  /* __QVertexBlendFunctions_FXH__ */



const char *CEmbeddedGenericHLSL::ms_pTechniqueTemplate =
"technique $TECH"\
"{"\
	"pass P0"\
	"{"\
		"VertexShader = compile vs_$VS_VER $(VS)();"\
		"PixelShader  = compile ps_$PS_VER $(PS)();"\

		"CullMode = Ccw;"\
		"ZEnable = True;"\
		"AlphaBlendEnable = True;"\
		"SrcBlend = SrcAlpha;"\
		"DestBlend = InvSrcAlpha;"\
	"}"\
"}";



// ??? VS_HSPerVertexLights_QVertexBlend


void LoadShader_HSPerVeretxLighting( CGenericShaderDesc& desc, CEmbeddedHLSLEffectDesc& dest )
{
	// per-vetex without specular reflection
	// Just select the shader for multi lights
//	dest.pVS     = "";//ms_pVS_PerVertexHSLights;
//	dest.pPS     = "";//ms_pPS_PerVertexHSLights;
//	dest.pVSName = "";
//	dest.pPSName = "";
}


void LoadShader_HSPerVeretxLightingWithSpecular( CGenericShaderDesc& desc, CEmbeddedHLSLEffectDesc& dest )
{
/*	switch( desc.num_directional_lights )
	{
	case -1:
		switch( desc.num_point_lights )
		{
			dest.vs = CEmbeddedHLSLShaders::ms_VS_PVL_HSLs_Spacular;
			dest.ps = CEmbeddedHLSLShaders::ms_PS_PVL_HSLs_Spacular;
			dest.pTechniqueName = "PVL_HSLs_Spacular";
//			CreateShader( vs, ps,  );

		case -1:

			break;
		}
		// select a technique that supports multiple number of lights
		break;
	case 1:
		break;
	default:
		break;
	}*/
}


void LoadShader_HSPerVeretxLighting_QVertexBlend( CGenericShaderDesc& desc,
												  CEmbeddedHLSLEffectDesc& dest )
{
	dest.vs = CEmbeddedHLSLShaders::ms_VS_PVL_HSLs_QVertexBlend;
	dest.ps = CEmbeddedHLSLShaders::ms_PS_PVL_HSLs_QVertexBlend;
//	dest.pTechniqueName = "";

	switch( desc.NumDirectionalLights )
	{
	case -1:
	default:
		break;
	}
}


void LoadShader_HSPerVeretxLighting_QVertexBlend_Specular( CGenericShaderDesc& desc, CEmbeddedHLSLEffectDesc& dest )
{
}


void LoadShader_HSPerPixelLighting( CGenericShaderDesc& desc, CEmbeddedHLSLEffectDesc& dest )
{
	dest.vs = CEmbeddedHLSLShaders::ms_VS_PPL_HSLs;
	dest.ps = CEmbeddedHLSLShaders::ms_PS_PPL_HSLs;
//	dest.pTechniqueName = "PPL_HSLs";
}


void LoadShader_HSPerPixelLighting_Specular( CGenericShaderDesc& desc, CEmbeddedHLSLEffectDesc& dest )
{
	dest.vs = CEmbeddedHLSLShaders::ms_VS_PPL_HSLs_Specular;
	dest.ps = CEmbeddedHLSLShaders::ms_PS_PPL_HSLs_Specular;
//	dest.pTechniqueName = "PPL_HSLs_Specular";
}


void LoadShader_HSPerPixelLighting_QVertexBlend( CGenericShaderDesc& desc, CEmbeddedHLSLEffectDesc& dest )
{
}

/*
void LoadPerVeretxLightingShader( CGenericShaderDesc& desc, CEmbeddedHLSLEffectDesc& dest )
{
	switch( desc. )
	{
	default:
		break;
	}
}
*/

void LoadHSLightingShader( CGenericShaderDesc& desc, CEmbeddedHLSLEffectDesc& dest )
{
	switch( desc.ShaderLightingType )
	{
	case CShaderLightingType::PER_VERTEX:
		if( desc.VertexBlendType == CVertexBlendType::NONE )
		{
			if( desc.Specular )
				LoadShader_HSPerVeretxLightingWithSpecular(desc,dest);
			else
				LoadShader_HSPerVeretxLighting(desc,dest);
		}
		else if( desc.VertexBlendType == CVertexBlendType::QUATERNION_AND_VECTOR3
		      || desc.VertexBlendType == CVertexBlendType::MATRIX )
		{
			if( desc.Specular )
				LoadShader_HSPerVeretxLighting_QVertexBlend_Specular(desc,dest);
			else
				LoadShader_HSPerVeretxLighting_QVertexBlend(desc,dest);
		}
		break;
	case CShaderLightingType::PER_PIXEL:
		if( desc.Specular )
			LoadShader_HSPerPixelLighting_Specular(desc,dest);
		else
			LoadShader_HSPerPixelLighting(desc,dest);
		break;
	default:
		break;
	}
}


Result::Name CEmbeddedGenericHLSL::GenerateShader( CGenericShaderDesc& desc, std::string& hlsl_effect )
{
	LOG_FUNCTION_SCOPE();

	CEmbeddedHLSLEffectDesc hlsl_desc;

//	LoadShader( desc, hlsl_desc );
	LoadHSLightingShader( desc, hlsl_desc );

	if( !hlsl_desc.IsValid() )
		return Result::UNKNOWN_ERROR;

	hlsl_effect = "";
	hlsl_effect += hlsl_desc.vs.pContent; // vertex shader
	hlsl_effect += hlsl_desc.ps.pContent; // pixel shader

	replace_all( hlsl_effect, "#include \"Matrix.fxh\"",                ms_pMatrix );
	replace_all( hlsl_effect, "#include \"ColorPair.fxh\"",             ms_pColorPair );
	replace_all( hlsl_effect, "#include \"TexDef.fxh\"",                ms_pTexDef );
	replace_all( hlsl_effect, "#include \"LightDef.fxh\"",              ms_pLightDef );
	replace_all( hlsl_effect, "#include \"Quaternion.fxh\"",            ms_pQuaternion );
	replace_all( hlsl_effect, "#include \"Transform.fxh\"",             ms_pTransform );
	replace_all( hlsl_effect, "#include \"QVertexBlendFunctions.fxh\"", ms_pQVertexBlendFunctions);

	string tech( ms_pTechniqueTemplate );

	// All the effect has the same technique name, "Default"
	// rationale: the implementation of the effect is hidden from the client,
	// so technique names that represents the content of the shader, such as "PerPixel_HemisphericLighting",
	// cannot be not used.
	replace_first( tech, "$TECH", "Default" );
	replace_first( tech, "$(VS)", hlsl_desc.vs.pName );
	replace_first( tech, "$(PS)", hlsl_desc.ps.pName );

	replace_first( tech, "$VS_VER",   "2_0" );
	replace_first( tech, "$PS_VER",   "2_0" );

	hlsl_effect += tech;

	return Result::SUCCESS;
}

/*
CShaderHandle GetGenericShader( CGenericShaderDesc& desc )
{
	CShaderHandle handle
	bool res = Get( desc, handle );
	if( res )
		return handle;

	handle = Load( desc );

	return handle;
}
*/

#include "Graphics/Direct3D9.hpp"

void ConfigureShader()
{
	D3DCAPS9 caps;
	DIRECT3D9.GetDevice()->GetDeviceCaps( &caps );

	DWORD ps_ver = caps.PixelShaderVersion;

/*	if( 3.0 <= ps_ver )
	{
	}
	else
	{
	}*/
}