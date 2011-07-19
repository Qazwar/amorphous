#ifndef __GenericShaderDesc_HPP__
#define __GenericShaderDesc_HPP__


class CLightingTechnique
{
public:
	enum Name
	{
		NORMAL,
		HEMISPHERIC,
		NUM_TYPES
	};
};


class CShaderLightingType
{
public:
	enum Name
	{
		PER_VERTEX,
		PER_PIXEL,
		NUM_TYPES
	};
};


class CAlphaBlendType
{
public:
	enum Name
	{
		NONE,
//		UNIFORM,
		DIFFUSE_ALPHA,                         ///< use alpha of diffuse color
		DECAL_TEX_ALPHA,
//		NORMAL_MAP_ALPHA,
		MOD_DIFFUSE_ALPHA_AND_DECAL_TEX_ALPHA, ///< modulate diffuse and decal texture alpha
//		TEX0_ALPHA,
//		TEX1_ALPHA,
//		TEX2_ALPHA,
//		TEX3_ALPHA,
//		MOD_DIFFUSE_ALPHA_AND_TEX0_ALPHA,
//		MOD_DIFFUSE_ALPHA_AND_TEX1_ALPHA,
//		MOD_DIFFUSE_ALPHA_AND_TEX0_ALPHA_AND_TEX1_ALPHA,
		NUM_ALPHA_BLEND_TYPES
	};
};


class CVertexBlendType
{
public:
	enum Name
	{
		NONE,
		QUATERNION_AND_VECTOR3, ///< 3DMath/Transform class
		MATRIX,                 ///< 3DMath/Matrix34 class
		NUM_TYPES
	};
};

class CSpecularSource
{
public:
	enum Name
	{
		NONE,
		UNIFORM,                       ///< uniform specularity
		TEX0_ALPHA,
		DECAL_TEX_ALPHA = TEX0_ALPHA,  ///< alpha channel of decal texture (stage 0)
		TEX1_RED,
		TEX1_GREEN,
		TEX1_BLUE,
		TEX1_ALPHA,
		NORMAL_MAP_ALPHA = TEX1_ALPHA, ///< alpha channel of normal map texture (stage 1)
		NUM_TYPES
	};
};

class CEnvMapOption
{
public:
	enum Name
	{
		NONE,
		ENABLED,
		NUM_OPTIONS
	};
};

class CPlanerReflectionOption
{
public:
	enum Name
	{
		NONE,
		FLAT,
		PERTURBED,
		NUM_OPTIONS
	};
};


class CGenericShaderDesc
{
public:

	CLightingTechnique::Name LightingTechnique;
	CShaderLightingType::Name ShaderLightingType; // per-vertex / per-pixel
	CSpecularSource::Name Specular;
	CVertexBlendType::Name VertexBlendType;
	CAlphaBlendType::Name AlphaBlend;
	CEnvMapOption::Name EnvMap;
	CPlanerReflectionOption::Name PlanerReflection;
	int NumPointLights;
	int NumDirectionalLights;
	int NumSpotLights;

public:

	CGenericShaderDesc()
		:
	LightingTechnique(CLightingTechnique::HEMISPHERIC),
	ShaderLightingType(CShaderLightingType::PER_PIXEL),
	Specular(CSpecularSource::NONE),
	VertexBlendType(CVertexBlendType::NONE),
	AlphaBlend(CAlphaBlendType::NONE),
	EnvMap(CEnvMapOption::NONE),
	PlanerReflection(CPlanerReflectionOption::NONE),
	NumPointLights(-1),
	NumDirectionalLights(-1),
	NumSpotLights(-1)
	{}

	bool operator==( const CGenericShaderDesc& rhs ) const
	{
		if( LightingTechnique    == rhs.LightingTechnique
		 && ShaderLightingType   == rhs.ShaderLightingType
		 && Specular             == rhs.Specular
		 && VertexBlendType      == rhs.VertexBlendType
		 && AlphaBlend           == rhs.AlphaBlend
		 && EnvMap               == rhs.EnvMap
		 && PlanerReflection     == rhs.PlanerReflection
		 && NumPointLights       == rhs.NumPointLights
		 && NumDirectionalLights == rhs.NumDirectionalLights
		 && NumSpotLights        == rhs.NumSpotLights )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};


#endif  /* __GenericShaderDesc_HPP__ */
