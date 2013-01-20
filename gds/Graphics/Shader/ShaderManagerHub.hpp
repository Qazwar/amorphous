#ifndef  __ShaderManagerHub_H__
#define  __ShaderManagerHub_H__


#include "FixedFunctionPipelineManager.hpp"
#include "ShaderManager.hpp"
#include "../Camera.hpp"
#include "../../Support/Singleton.hpp"


namespace amorphous
{


#define ShaderManagerHub ( (*CShaderManagerHub::Get()) )



/**
 * set view and projection transforms for all the shader managers
 *
 */
class CShaderManagerHub
{
	std::vector<CShaderManager *> m_vecpShaderManager;

	std::vector<Matrix44> m_vecViewMatrix;
	std::vector<Matrix44> m_vecProjMatrix;

private:

	void RegisterShaderManager( CShaderManager* pShaderMgr );

	bool ReleaseShaderManager( CShaderManager* pShaderMgr );

protected:

	static CSingleton<CShaderManagerHub> m_obj;

public:

	static CShaderManagerHub* Get() { return m_obj.get(); }

	CShaderManagerHub();

	inline void PushViewAndProjectionMatrices( const Matrix44& view, const Matrix44& proj );

	/// must be called in pairs with PopViewAndProjectionMatrices()
	inline void PushViewAndProjectionMatrices( const Camera& camera );

	/// must be called in pairs with PushViewAndProjectionMatrices()
	inline void PopViewAndProjectionMatrices();

	inline void PopViewAndProjectionMatrices_NoRestore();

	friend class CShaderManager;
};


inline void CShaderManagerHub::PushViewAndProjectionMatrices( const Matrix44& view, const Matrix44& proj )
{
	// push to the stack
	m_vecViewMatrix.push_back( view );
	m_vecProjMatrix.push_back( proj );

	size_t i, num_shader_mgrs = m_vecpShaderManager.size();
	for( i=0; i<num_shader_mgrs; i++ )
	{
		m_vecpShaderManager[i]->SetViewTransform( view );
		m_vecpShaderManager[i]->SetProjectionTransform( proj );
	}

	FixedFunctionPipelineManager().SetViewTransform( view );
	FixedFunctionPipelineManager().SetProjectionTransform( proj );
}


inline void CShaderManagerHub::PushViewAndProjectionMatrices( const Camera& camera )
{
	PushViewAndProjectionMatrices( camera.GetCameraMatrix(), camera.GetProjectionMatrix() );
}


inline void CShaderManagerHub::PopViewAndProjectionMatrices()
{
	if( m_vecViewMatrix.size() == 0 )
		return; // stack is empty

	m_vecViewMatrix.pop_back();
	m_vecProjMatrix.pop_back();

	if( m_vecViewMatrix.size() == 0 )
		return; // stack is empty

	// set the previous transforms
	Matrix44 matView, matProj;

	matView = m_vecViewMatrix.back();
	matProj = m_vecProjMatrix.back();

	size_t i, num_shader_mgrs = m_vecpShaderManager.size();
	for( i=0; i<num_shader_mgrs; i++ )
	{
		m_vecpShaderManager[i]->SetViewTransform( matView );
		m_vecpShaderManager[i]->SetProjectionTransform( matProj );
	}

	// update the transforms of fixed function pipeline as well
	FixedFunctionPipelineManager().SetViewTransform( matView );
	FixedFunctionPipelineManager().SetProjectionTransform( matProj );
}


inline void CShaderManagerHub::PopViewAndProjectionMatrices_NoRestore()
{
	if( m_vecViewMatrix.size() == 0 )
		return; // stack is empty

	m_vecViewMatrix.pop_back();
	m_vecProjMatrix.pop_back();
}


} // namespace amorphous



#endif		/*  __ShaderManagerHub_H__  */
