#ifndef  __ShadowMapManager_H__
#define  __ShadowMapManager_H__

#include <map>
#include "../3DMath/3DGameMath.hpp"
#include "GraphicsComponentCollector.hpp"
#include "TextureRenderTarget.hpp"
#include "Camera.hpp"
#include "GraphicsDevice.hpp"
#include "ShadowMaps.hpp"


namespace amorphous
{


/**
 * Still in an early stage of the development
 * - requires "Shader/ShadowMap.fx" to run
 *
 * usage notes:
 * steps
 * //1. for 
 * 1. Call CShadowMapManager::BeginSceneShadowMap().
 * 2. render objects that cast shadow to others using the shader technique "ShadowMap".
 *    Set the technique through the effect obtained by
 *    CShader::Get()->GetShaderManager()->GetEffect()
 *    e.g.) CShader::Get()->GetShaderManager()->GetEffect()->SetTechnique( "..." ).
 * 3. Call CShadowMapManager::EndSceneShadowMap().
 *
 * 4. Call CShadowMapManager::BeginSceneDepthMap().
 * 5. Render objects that receive shadow from the objects rendered in step 2
 *    using the shader technique "SceneShadowMap".
 * 6. Call CShadowMapManager::EndSceneDepthMap()
 *
 * 7. Call CShadowMapManager::BeginScene()
 * 8. Render objects using original shaders
 * 9. Call CShadowMapManager::EndScene()
 *
 */
class CShadowMapManager : public CGraphicsComponent
{
protected:

	enum Params
	{
		NUM_MAX_SCENE_SHADOW_TEXTURES = 2,
	};

	typedef std::map< int, boost::shared_ptr<CShadowMap> > IDtoShadowMap;

	IDtoShadowMap m_mapIDtoShadowMap;

	boost::shared_ptr<CShadowMapSceneRenderer> m_pSceneRenderer;

	// Temporarily stores the shadow map to return shadowmap/depth test technique
	boost::shared_ptr<CShadowMap> m_pCurrentShadowMap;

	int m_IDCounter;

	/// used to temporarily hold original surfaces
//	LPDIRECT3DSURFACE9 m_pOriginalSurface;
//	LPDIRECT3DSURFACE9 m_pOriginalDepthSurface;

	CViewport m_OriginalViewport;

	int m_iTextureWidth;
	int m_iTextureHeight;

	int m_ShadowMapSize;

	/// Holds textures(s) with the original screen size.
	/// Shadows of the scene are rendered to this texture, then overlayed
	/// to the original, non-shadowed scene.
	boost::shared_ptr<CTextureRenderTarget> m_apShadowTexture[NUM_MAX_SCENE_SHADOW_TEXTURES];

	CCamera m_SceneCamera;

	/// Stores the shader necessasry for shadowmap.
	/// The shader supposed to contain 2 techniques
	/// - "ShadowMap": for shadow map rendering. renders the shadow casters to shadow map texture. m_LightCamera is used to calculate  
	ShaderHandle m_Shader;

	boost::shared_ptr<CTextureRenderTarget> m_pSceneRenderTarget;

//	static std::string ms_strDefaultShaderFilename;

	std::string m_ShadowMapShaderFilename;

	bool m_DisplayShadowMapTexturesForDebugging;

protected:

	void SetDefault();

	bool CreateShadowMapTextures();

	bool CreateSceneShadowMapTextures();

	virtual TextureFormat::Format GetShadowMapTextureFormat() { return TextureFormat::R32F; }

public:

	CShadowMapManager();

	~CShadowMapManager();

	/// returns true on success
	virtual bool Init();

	/// The system sets the size to the screen width and height by default.
	void SetSceneShadowTextureSize( int texture_width, int texture_height );

	void SetSceneRenderer( boost::shared_ptr<CShadowMapSceneRenderer> pSceneRenderer );

	std::map< int, boost::shared_ptr<CShadowMap> >::iterator CreateShadowMap( U32 id, const Light& light );

	Result::Name UpdateLightForShadow( U32 id, const Light& light );

	void RemoveShadowForLight( int shadowmap_id );

	void UpdateLight( int shadowmap_id, const Light& light );

	void RenderShadowCasters( CCamera& camera );

	void RenderShadowReceivers( CCamera& camera );

	ShaderHandle& GetShader() { return m_Shader; }

	CShaderTechniqueHandle ShaderTechniqueForShadowCaster( CVertexBlendType::Name vertex_blend_type = CVertexBlendType::NONE );

	CShaderTechniqueHandle ShaderTechniqueForShadowReceiver( CVertexBlendType::Name vertex_blend_type = CVertexBlendType::NONE );

	CShaderTechniqueHandle ShaderTechniqueForNonShadowedCasters( CVertexBlendType::Name vertex_blend_type = CVertexBlendType::NONE );

//	void Init( int texture_width, int texture_height );

//	void SetTextureWidth( const int width, const int height );

//	void BeginSceneForShadowCaster();
//	void EndSceneForShadowCaster();
//	void BeginSceneForShadowReceiver();
//	void EndSceneForShadowReceiver();

	/// sets m_ShaderManager to CShader (singleton)
	void BeginSceneShadowMap();

	/// why virtual?
	/// - See CVarianceShadowMapManager
	virtual void EndSceneShadowMap();

	virtual void PostProcessShadowMap( CShadowMap& shadow_map ) {}

//	virtual void UpdateLightPositionAndDirection();


	virtual void BeginSceneDepthMap();

	void EndSceneDepthMap();

	/// sets the render target texture for the scene
	void BeginScene();
	void EndScene();

	bool HasShadowMap() const { return !(m_mapIDtoShadowMap.empty()); }

	void RenderSceneWithShadow();

	void UpdateScreenSize();

	void ReleaseTextures();

	virtual void ReleaseGraphicsResources() {}

	virtual void LoadGraphicsResources( const CGraphicsParameters& rParam ) {}

	void SetCameraDirection( const Vector3& vCamDir ) { m_SceneCamera.SetOrientation( CreateOrientFromFwdDir( vCamDir ) ); }
	void SetCameraPosition( const Vector3& vCamPos ) { m_SceneCamera.SetPosition( vCamPos ); }
	void SetSceneCamera( const CCamera& camera ) { m_SceneCamera = camera; }

	CCamera& SceneCamera() { return m_SceneCamera; }

	/// for visual debugging
//	void RenderShadowMapTexture( int sx, int sy, int ex, int ey );
	void RenderSceneShadowMapTexture( int sx, int sy, int ex, int ey );
	void RenderSceneWithoutShadow( int sx, int sy, int ex, int ey );
	void RenderSceneWithShadow( int sx, int sy, int ex, int ey );

//	boost::shared_ptr<CTextureRenderTarget> GetSceneShadowTexture() { return m_apShadowTexture[0]; }

	TextureHandle GetSceneShadowTexture();

//	static void SetDefaultShaderFilename( const std::string& filename ) { ms_strDefaultShaderFilename = filename; }
	void SetShadowMapShaderFilename( const std::string& filename ) { m_ShadowMapShaderFilename = filename; }

	/// For debugging
	void SaveSceneTextureToFile( const std::string& filename );
};

} // namespace amorphous



#endif		/*  __ShadowMapManager_H__  */
