#ifndef  __EnvMapTest_HPP__
#define  __EnvMapTest_HPP__


#include "amorphous/Graphics/TextureHandle.hpp"
#include "amorphous/Graphics/MeshObjectHandle.hpp"
#include "amorphous/Graphics/CubeMapManager.hpp"
#include "amorphous/Graphics/ShaderHandle.hpp"
#include "amorphous/Graphics/Shader/ShaderTechniqueHandle.hpp"
#include "amorphous/Input.hpp"
//#include "Graphics/UnitSphere.hpp"

#include "../../_Common/GraphicsTestBase.hpp"


class CEnvMapTest;


class CubeMapTestRenderer : public CCubeMapSceneRenderer
{
	CEnvMapTest *m_pEnvMapTest;

public:

	CubeMapTestRenderer(CEnvMapTest *pEnvMapTest) : m_pEnvMapTest(pEnvMapTest) {}

	virtual ~CubeMapTestRenderer() {}

	virtual Vector3 GetCameraPosition() { return Vector3(0,1,0); }

	/// Called by CubeMapManager in CubeMapManager::RenderToCubeMap()
	/// to render the scene to the cube map surface.
	/// - called 6 times in total to render the scene to all the cube map surfaces
	/// - IDirect3DDevice9::BeginScene() & EndScene() pair is not called
	///   in CubeMapManager::RenderToCubeMap() before and after the 
	///   RenderSceneToCubeMap() calls.
	///   User must call them in this functions?
	///   - changed: BeginScene() and EndScene() are called in CubeMapManager::RenderToCubeMap()
	virtual void RenderSceneToCubeMap( Camera& camera );
};


class CEnvMapTest : public CGraphicsTestBase
{
	boost::shared_ptr<CubeMapManager> m_pCubeMapManager;

	boost::shared_ptr<CCubeMapSceneRenderer> m_pCubeMapSceneRenderer;

	std::vector<MeshHandle> m_vecEnvMappedMesh;

	std::vector<MeshHandle> m_vecNonEnvMappedMesh;

	int m_CurrentEnvMapTarget;

	ShaderTechniqueHandle m_NonEnvMappedMesh;
	ShaderTechniqueHandle m_EnvMappedMesh;
//	ShaderTechniqueHandle m_SkyboxTechnique;
//	ShaderTechniqueHandle m_DefaultTechnique;

	ShaderHandle m_EnvMappedMeshShader;

	ShaderHandle m_NonEnvMappedMeshShader;

//	shared_ptr<ShaderLightManager> m_pShaderLightManager;

	bool m_UseEmbeddedShader;

	Vector3 m_vLightPosition;


public:

	CEnvMapTest();

	~CEnvMapTest();

	const char *GetAppTitle() const { return "EnvMapTest"; }

	int Init();

	void Release() {};

	void Update( float dt );

//	void Render();

	virtual void RenderBase();

	virtual void HandleInput( const InputData& input );

	void RenderNonEnvMappedMeshes();

	void UpdateViewTransform( const Matrix44& matView );

	void UpdateProjectionTransform( const Matrix44& matProj );
};


#endif /* __EnvMapTest_HPP__ */