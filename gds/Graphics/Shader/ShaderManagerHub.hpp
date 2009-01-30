#ifndef  __ShaderManagerHub_H__
#define  __ShaderManagerHub_H__

#include "../Camera.hpp"
#include "../Direct3D9.hpp"
#include "ShaderManager.hpp"

#include <vector>


#include "Support/Singleton.hpp"
using namespace NS_KGL;


#define ShaderManagerHub ( (*CShaderManagerHub::Get()) )



/**
 * set view and projection transforms for all the shader managers
 *
 */
class CShaderManagerHub
{
	std::vector<CShaderManager *> m_vecpShaderManager;

	std::vector<D3DXMATRIX> m_vecViewMatrix;
	std::vector<D3DXMATRIX> m_vecProjMatrix;

private:

	void RegisterShaderManager( CShaderManager* pShaderMgr );

	bool ReleaseShaderManager( CShaderManager* pShaderMgr );

protected:

	static CSingleton<CShaderManagerHub> m_obj;

public:

	static CShaderManagerHub* Get() { return m_obj.get(); }

	CShaderManagerHub();

	/// must be called in pairs with PopViewAndProjectionMatrices()
	inline void PushViewAndProjectionMatrices( const CCamera& camera );

	/// must be called in pairs with PushViewAndProjectionMatrices()
	inline void PopViewAndProjectionMatrices();

	inline void PopViewAndProjectionMatrices_NoRestore();


	inline void SetCubeTexture( int index, LPDIRECT3DCUBETEXTURE9 pCubeTexture );

	friend class CShaderManager;
};


inline void CShaderManagerHub::PushViewAndProjectionMatrices( const CCamera& camera )
{
	D3DXMATRIX matView, matProj;

	camera.GetCameraMatrix( matView );
	camera.GetProjectionMatrix( matProj );

	// push to the stack
	m_vecViewMatrix.push_back( matView );
	m_vecProjMatrix.push_back( matProj );

	size_t i, num_shader_mgrs = m_vecpShaderManager.size();
	for( i=0; i<num_shader_mgrs; i++ )
	{
		m_vecpShaderManager[i]->SetViewTransform( matView );
		m_vecpShaderManager[i]->SetProjectionTransform( matProj );
	}

	// update the transforms of fixed function pipeline as well
	LPDIRECT3DDEVICE9 pd3dDev = DIRECT3D9.GetDevice();
	pd3dDev->SetTransform(D3DTS_VIEW, &matView);
	pd3dDev->SetTransform(D3DTS_PROJECTION, &matProj);
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
	D3DXMATRIX matView, matProj;

	matView = m_vecViewMatrix.back();
	matProj = m_vecProjMatrix.back();

	size_t i, num_shader_mgrs = m_vecpShaderManager.size();
	for( i=0; i<num_shader_mgrs; i++ )
	{
		m_vecpShaderManager[i]->SetViewTransform( matView );
		m_vecpShaderManager[i]->SetProjectionTransform( matProj );
	}

	// update the transforms of fixed function pipeline as well
	LPDIRECT3DDEVICE9 pd3dDev = DIRECT3D9.GetDevice();
	pd3dDev->SetTransform(D3DTS_VIEW, &matView);
	pd3dDev->SetTransform(D3DTS_PROJECTION, &matProj);
}


inline void CShaderManagerHub::PopViewAndProjectionMatrices_NoRestore()
{
	if( m_vecViewMatrix.size() == 0 )
		return; // stack is empty

	m_vecViewMatrix.pop_back();
	m_vecProjMatrix.pop_back();
}


inline void CShaderManagerHub::SetCubeTexture( int index, LPDIRECT3DCUBETEXTURE9 pCubeTexture )
{
	size_t i, num_shader_mgrs = m_vecpShaderManager.size();
	for( i=0; i<num_shader_mgrs; i++ )
	{
		m_vecpShaderManager[i]->SetCubeTexture( index, pCubeTexture );
	}
}


#endif		/*  __ShaderManagerHub_H__  */
