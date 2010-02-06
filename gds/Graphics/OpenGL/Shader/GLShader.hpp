#ifndef __GLShader_HPP__
#define __GLShader_HPP__


#include <gds/Graphics/GLGraphicsDevice.hpp>
#include <gds/Graphics/OpenGL/GLExtensions.hpp>
#include <gds/Graphics/OpenGL/Shader/GLFixedFunctionPipelineManager.hpp>
#include <gds/Graphics/Shader/ShaderManager.hpp>
#include <gds/Graphics/ShaderHandle.hpp>
#include <gds/3DMath/Transform.hpp>


//==============================================================================
// Use glCreateShaderObjectARB, glShaderSourceARB, glCompileShaderARB, etc.
//==============================================================================

class CGLShader : public CShaderManager
{
protected:

	GLhandleARB m_Shader;

	virtual GLenum GetShaderType() const = 0;

public:

	CGLShader()
		:
	m_Shader(0)
	{}

	GLhandleARB GetGLHandle() { return m_Shader; }

	virtual ~CGLShader() {}

	bool LoadFromFile( const std::string& filepath );

	bool LoadShaderFromText( const stream_buffer& buffer );

	void Release();

	void Reload();
};


class CGLVertexShader : public CGLShader
{
	GLenum GetShaderType() const { return GL_VERTEX_SHADER_ARB; }

public:
};


class CGLFragmentShader : public CGLShader
{
	GLenum GetShaderType() const { return GL_FRAGMENT_SHADER_ARB; }

public:

};


class CGLProgram : public CShaderManager
{
	GLhandleARB m_Program;

//	boost::shared_ptr<CGLVertexShader> m_pVertexShader;
//	boost::shared_ptr<CGLFragmentShader> m_pFragmentShader;

	// Why raw ptr?
	// - graphis resources manager currenly stores shader managers with raw pointers
	CGLVertexShader *m_pVertexShader;
	CGLFragmentShader *m_pFragmentShader;

	CShaderHandle m_VertexShader;
	CShaderHandle m_FragmentShader;

public:

	CGLProgram();

	~CGLProgram();

	bool LoadShaderFromFile( const std::string& filename );

	bool LoadShaderFromText( const stream_buffer& buffer );

	void Release();

	void Reload();

//	void SetWorldTransform( const Matrix34& world_pose ) { GLFixedFunctionPipelineManager().SetWorldTransform(world_pose); }

	void SetWorldTransform( const Matrix44& matWorld ) { GLFixedFunctionPipelineManager().SetWorldTransform(matWorld); }

	void SetViewTransform( const Matrix44& matView ) { GLFixedFunctionPipelineManager().SetViewTransform(matView); }

	void SetProjectionTransform( const Matrix44& matProj ) { GLFixedFunctionPipelineManager().SetProjectionTransform(matProj); }

	void SetWorldViewTransform( const Matrix44& matWorld, const Matrix44& matView ) { GLFixedFunctionPipelineManager().SetWorldViewTransform(matWorld,matView); }

	void SetWorldViewProjectionTransform( const Matrix44& matWorld, const Matrix44& matView, const Matrix44& matProj ) { GLFixedFunctionPipelineManager().SetWorldViewProjectionTransform(matWorld,matView,matProj); }

	void SetBlendTransforms( const std::vector<Transform>& src_transforms );

/*
	virtual void SetWorldTransform( const Matrix34& world_pose ) {}

	virtual void SetWorldTransform( const Matrix44& matWorld ) {}

	virtual void SetViewTransform( const Matrix44& matView ) {}

	virtual void SetProjectionTransform( const Matrix44& matProj ) {}

	virtual void SetWorldViewTransform( const Matrix44& matWorld, const Matrix44& matView  ) {}

	virtual void SetWorldViewProjectionTransform( const Matrix44& matWorld, const Matrix44& matView, const Matrix44& matProj ) {}


	virtual void GetWorldTransform( Matrix44& matWorld ) const {}

	virtual void GetViewTransform( Matrix44& matView ) const {}


	virtual inline void SetViewerPosition( const D3DXVECTOR3& vEyePosition ) {}


	virtual HRESULT SetTexture( const int iStage, const LPDIRECT3DTEXTURE9 pTexture ) { return E_FAIL; }
*/
	Result::Name SetTexture( const int iStage, const CTextureHandle& texture );

//	HRESULT SetCubeTexture( const int index, const LPDIRECT3DCUBETEXTURE9 pCubeTexture ) { return E_FAIL; }

	void Begin();

	void End();
/*
//	virtual void SetTexture( const char *pName, const LPDIRECT3DTEXTURE9 pTexture ) {}

	virtual Result::Name SetTechnique( const unsigned int id ) { return Result::UNKNOWN_ERROR; }

	virtual Result::Name SetTechnique( CShaderTechniqueHandle& tech_handle ) { return Result::UNKNOWN_ERROR; }

//	bool RegisterTechnique( const unsigned int id, const char *pcTechnique );

//	D3DXHANDLE GetTechniqueHandle( int id ) { return m_aTechniqueHandle[id]; }

	virtual void SetParam( CShaderParameter< std::vector<float> >& float_param ) {}

//	void SetParam( CShaderParameter< std::vector<int> >& integer_param );

//	void SetTextureParam()

	virtual boost::shared_ptr<CShaderLightManager> GetShaderLightManager() { return boost::shared_ptr<CShaderLightManager>(); }
*/
	friend class CShaderManagerHub;
};



//==============================================================================
// Use glGenProgramsARB, glBindProgramARB, etc.
//==============================================================================
/*
class CGLProgramBase
{
	GLuint m_ProgramID;

private:

	virtual GLenum GetProgramType() const = 0;

public:

	CGLProgramBase()
		:
	m_ProgramID(0)
	{}

	virtual ~CGLProgramBase() {}

	void LoadShaderFromFile( const std::string& filepath );
};


class CGLVertexProgram : public CGLProgramBase
{
private:

	GLenum GetProgramType() const;

public:

	CGLVertexProgram() {}
};


class CGLFragmentProgram : public CGLProgramBase
{
private:

	GLenum GetProgramType() const;

public:

	CGLFragmentProgram() {}
};
*/


#endif /* __GLShader_HPP__ */
