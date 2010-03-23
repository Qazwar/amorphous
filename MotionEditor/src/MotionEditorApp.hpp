#ifndef  __MotionEditorApp_HPP__
#define  __MotionEditorApp_HPP__


#include <boost/shared_ptr.hpp>
#include <gds/App/GraphicsApplicationBase.hpp>
#include <gds/Graphics/fwd.hpp>
#include <gds/Graphics/Camera.hpp>
#include <gds/Graphics/MeshObjectHandle.hpp>
#include <gds/Graphics/TextureHandle.hpp>
#include <gds/Graphics/ShaderHandle.hpp>
#include <gds/Graphics/Shader/ShaderTechniqueHandle.hpp>
#include <gds/Graphics/Shader/ShaderManager.hpp>
#include <gds/Input/fwd.hpp>
#include <gds/Input.hpp>


class CMotionPrimitiveViewer;


class CMotionEditorApp : public CGraphicsApplicationBase
{
	boost::shared_ptr<CMotionPrimitiveViewer> m_pMotionPrimitiveViewer;

private:

	const std::string GetApplicationTitle() { return "Motion Editor (just a viewer for now)"; }

public:

	CMotionEditorApp();

	~CMotionEditorApp();

	int Init();

	void Release() {};

	void Update( float dt );

	void Render();

//	void HandleInput( const SInputData& input );
};


#endif /* __MotionEditorApp_HPP__ */
