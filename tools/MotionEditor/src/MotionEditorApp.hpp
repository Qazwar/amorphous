#ifndef  __MotionEditorApp_HPP__
#define  __MotionEditorApp_HPP__


#include <memory>
#include "gds/App/GraphicsApplicationBase.hpp"
#include "gds/Graphics/fwd.hpp"
#include "gds/Graphics/Camera.hpp"
#include "gds/Input/fwd.hpp"

using namespace amorphous;


class CMotionPrimitiveViewer;


class CMotionEditorApp : public CGraphicsApplicationBase
{
	std::shared_ptr<CMotionPrimitiveViewer> m_pMotionPrimitiveViewer;

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
