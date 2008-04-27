#include "D3DXMeshObject.h"

#include "3DCommon/Shader/Shader.h"
#include "3DCommon/Shader/ShaderManager.h"

#include "3DCommon/FVF_BumpVertex.h"
#include "3DCommon/FVF_BumpWeightVertex.h"
#include "3DCommon/FVF_ColorVertex.h"
#include "3DCommon/FVF_TextureVertex.h"
#include "3DCommon/FVF_NormalVertex.h"
#include "3DCommon/FVF_WeightVertex.h"
#include "3DCommon/FVF_ShadowVertex.h"

#include "3DCommon/MeshModel/MeshBone.h"
using namespace MeshModel;

#include "Support/Log/DefaultLog.h"


CD3DXMeshObject::CD3DXMeshObject()
: m_pMesh(NULL)
{
}


CD3DXMeshObject::CD3DXMeshObject( const std::string& filename )
: m_pMesh(NULL)
{
	HRESULT hr = LoadFromFile(filename);

	if( FAILED(hr) )
	{
		LOG_PRINT_ERROR( " - Failed to load a mesh file" + filename );
	}
}


CD3DXMeshObject::~CD3DXMeshObject()
{
	Release();
}


void CD3DXMeshObject::Release()
{
	// release materials
	CD3DXMeshObjectBase::Release();

    if( m_pMesh != NULL )
	{
        m_pMesh->Release();
		m_pMesh = NULL;
	}
}


HRESULT CD3DXMeshObject::LoadFromXFile( const std::string& filename )
{
	LPD3DXBUFFER pAdjacencyBuffer = NULL;
	HRESULT hr = LoadD3DXMeshAndMaterialsFromXFile( filename, m_pMesh, pAdjacencyBuffer );

	SAFE_RELEASE( pAdjacencyBuffer );

	return hr;
}


//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the object
//-----------------------------------------------------------------------------
void CD3DXMeshObject::Render()
{
	LPDIRECT3DDEVICE9 pd3dDevice = DIRECT3D9.GetDevice();
	LPD3DXMESH pMesh = m_pMesh;

	//We use only the first texture stage (stage 0)
	pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

	// color arguments on texture stage 0
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );

	// alpha arguments on texture stage 0
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );

	// alpha-blending settings 
	pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

    // Meshes are divided into subsets by materials. Render each subset in a loop
//	HRESULT hr;
	LPDIRECT3DTEXTURE9 pTex = NULL;
    for( int i=0; i<m_NumMaterials; i++ )
    {
        // Set the material and texture for this subset
		pd3dDevice->SetMaterial( &m_pMeshMaterials[i] );

//		if( FAILED(hr) ) MessageBox(NULL, "SetMaterial() Failed", "Error", MB_OK|MB_ICONWARNING);

		if( pTex = GetTexture(i,0).GetTexture() )
		{
			// blend color & alpha of vertex & texture
			pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
		}
		else
		{
			// no texture for this material - use only the vertx color & alpha
			pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		}

		pd3dDevice->SetTexture( 0, pTex );

        // Draw the mesh subset
        m_pMesh->DrawSubset( i );
    }

}


bool CD3DXMeshObject::LoadFromArchive( C3DMeshModelArchive& archive, const std::string& filename )
{
	// copy the contents from the archive

	m_strFilename = filename;

	LPDIRECT3DDEVICE9 pd3dDev = DIRECT3D9.GetDevice();

	HRESULT hr;

	LPD3DXMESH pMesh = LoadD3DXMeshFromArchive( archive );

	if( !pMesh )
		return false;

	// load surface materials & textures
	LoadMaterialsFromArchive( archive );

	hr = SetAttributeTable( pMesh, archive.GetTriangleSet() );

	if( FAILED(hr) )
		return false;


	// set attribute IDs for each face
	DWORD *pdwBuffer = NULL;
	pMesh->LockAttributeBuffer( 0, &pdwBuffer );
	DWORD face = 0;
	for( int i=0; i<m_NumMaterials; i++ )
	{
		CMMA_TriangleSet& triangle_set = archive.GetTriangleSet()[i];

		DWORD face_start = triangle_set.m_iStartIndex / 3;
		DWORD num_faces = triangle_set.m_iNumTriangles;
		for( face=face_start; face<face_start + num_faces; face++ )
		{
			pdwBuffer[face] = i;
		}
	}

	pMesh->UnlockAttributeBuffer();

	m_pMesh = pMesh;
	pMesh = NULL;

	PeekAttribTables( m_pMesh );

	return true;
}
