#ifndef __GLCgEffect_HPP__
#define __GLCgEffect_HPP__


#include "../../Shader/CgEffectBase.hpp"


namespace amorphous
{


#pragma comment( lib, "cgGL.lib" )


class CGLCgEffect : public CCgEffectBase
{
	void SetCGTextureParameter( CGparameter& param, TextureHandle& texture );

	void InitCgContext();

public:
	CGLCgEffect();

	~CGLCgEffect(){}

//	bool LoadShaderFromFile( const std::string& filename );
//	bool LoadShaderFromText( const stream_buffer& buffer );

//	void Release();
//	void Reload();

//	void SetHandlesToNULL();

//	inline void SetViewerPosition( const Vector3& vEyePosition );

//	inline void SetVertexBlendMatrix( int i, const Matrix34& mat );
//	inline void SetVertexBlendMatrix( int i, const Matrix44& mat );

//	inline Result::Name SetTexture( const int iStage, const TextureHandle& texture );
//	inline Result::Name SetCubeTexture( const int index, const TextureHandle& cube_texture );

//	inline void Begin();

//	inline void End();

//	inline Result::Name SetTechnique( const unsigned int id );
//	inline Result::Name SetTechnique( ShaderTechniqueHandle& tech_handle );
//	bool RegisterTechnique( const unsigned int id, const char *pcTechnique );
//	GLXHANDLE GetTechniqueHandle( int id ) { return m_aTechniqueHandle[id]; }

//	void SetTextureParam()

//	std::shared_ptr<ShaderLightManager> GetShaderLightManager();

//	void InitBlendTransformVariables( const std::string& variable_name );
//	void InitBlendTransformVariables();
//	void SetVertexBlendTransforms( const std::vector<Transform>& src_transforms );
};


} // namespace amorphous



#endif /* __GLCgEffect_HPP__ */
