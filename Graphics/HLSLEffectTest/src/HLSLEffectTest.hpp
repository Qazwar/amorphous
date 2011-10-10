#ifndef  __HLSLEffectTest_H__
#define  __HLSLEffectTest_H__


#include "gds/Graphics/GraphicsComponentCollector.hpp"
#include "gds/Graphics/MeshObjectHandle.hpp"
#include "gds/Graphics/TextureHandle.hpp"
#include "gds/Graphics/ShaderHandle.hpp"
#include "gds/Graphics/Shader/ShaderTechniqueHandle.hpp"
#include "gds/Input/fwd.hpp"
#include "gds/GUI/fwd.hpp"

#include "../../../_Common/GraphicsTestBase.hpp"


class CTestMeshHolder
{
public:
	CMeshObjectHandle m_Handle;
	Matrix34 m_Pose;
	float m_fScale;

	CMeshResourceDesc m_MeshDesc;

	enum LoadingStyleName
	{
		LOAD_SYNCHRONOUSLY,
		LOAD_MESH_AND_TEX_TOGETHER,
		LOAD_MESH_AND_TEX_SEPARATELY,
		NUM_LOADING_STYLES
	};

	LoadingStyleName m_LoadingStyle;

	CTestMeshHolder( const std::string& filepath, LoadingStyleName loading_style, const Matrix34& pose );
};


class CHLSLEffectTest : public CGraphicsTestBase, public CGraphicsComponent
{
	enum Params
	{
		TEXT_BUFFER_SIZE = 4096
	};

//	std::vector<CMeshObjectHandle> m_vecMesh;
	std::vector<CTestMeshHolder> m_vecMesh;

	std::vector<CShaderHandle> m_Shaders;

	std::vector<CShaderTechniqueHandle> m_Techniques;

	int m_CurrentShaderIndex;

	CShaderTechniqueHandle m_MeshTechnique;

	bool m_EnableLight[2];

	boost::shared_ptr<CInputHandler_Dialog> m_pUIInputHandler;

	boost::shared_ptr<CFontBase> m_pFont;

	CGM_DialogManagerSharedPtr m_pSampleUI;

	char m_TextBuffer[TEXT_BUFFER_SIZE];

	bool m_DisplayDebugInfo;

	int m_CurrentMeshIndex;

	CTextureHandle m_LookupTextureForLighting;

private:

	void CreateSampleUI();

	bool SetShader( int index );

	bool InitShaders();

	void CreateCachedResources();

	void RenderMesh();

	void RenderDebugInfo();

public:

	CHLSLEffectTest();

	~CHLSLEffectTest();

	int Init();

	void Release() {};

	void Update( float dt );

	void Render();

//	virtual void RenderBase();

	virtual void HandleInput( const SInputData& input );

	void ReleaseGraphicsResources();
	void LoadGraphicsResources( const CGraphicsParameters& rParam );
};


#endif /* __HLSLEffectTest_H__ */
