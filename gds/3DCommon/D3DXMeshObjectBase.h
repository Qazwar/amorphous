#ifndef  __D3DXMESHOBJECTBASE_H__
#define  __D3DXMESHOBJECTBASE_H__


#include <direct.h>

#include <vector>
#include <string>

#include "3DMath/aabb3.h"
#include "3DMath/Sphere.h"
#include "3DCommon/fwd.h"
#include "3DCommon/Direct3D9.h"
#include "3DCommon/TextureHandle.h"

#include "3DCommon/MeshModel/3DMeshModelArchive.h"
using namespace MeshModel;


class CD3DXMeshObjectBase
{
private:

	std::vector<int> m_vecFullMaterialIndices;

protected:

	class CMeshMaterial
	{
	public:

		// array of textures for a material
		// - possible usages
		//   - surface color (most common)
		//   - normal map
		//   - specular map (often stored in alpha channel of normal map)
		// A texture handle will be empty if no texture is used for the stage
		std::vector<CTextureHandle> Texture;
	};

	std::string m_strFilename;

	/// bounding box of the mesh
	AABB3 m_AABB;

	/// Number of materials
	int m_NumMaterials;

	D3DMATERIAL9 *m_pMeshMaterials;

	std::vector<AABB3> m_vecAABB;	///< aabb for each triangle subset

	/// visibility flag for each triangle set
	/// 1 visible / 0: not visible
	/// all elements set to 1 by default
	std::vector<int> m_IsVisible;

	/// enable / disable visibility clipping based on view frustum test.
	/// If true, user is responsible for calling UpdateVisibility( const CCamera& camera )
	/// every time the mesh is rendered.
	bool m_bViewFrustumTest;

	std::vector<CMeshMaterial> m_vecMaterial;

	/// flexible vertex format flag
	DWORD m_dwFVF;

	/// size of vertex (in bytes)
	int m_iVertexSize;

	/// vertex decleration
	LPDIRECT3DVERTEXDECLARATION9 m_pVertexDecleration;

	/// bounding sphere in local space of the model
	/// - Each implementation must properly initialize the sphere
	Sphere m_LocalShpere;

private:

	/// allocate material buffers, etc.
	void InitMaterials( int num_materials );

protected:

	virtual void LoadVertices( void*& pVBData, C3DMeshModelArchive& archive );

	HRESULT LoadMaterials( D3DXMATERIAL* d3dxMaterials, int num_materials );

	HRESULT LoadMaterialsFromArchive( C3DMeshModelArchive& rArchive );

	virtual const D3DVERTEXELEMENT9 *GetVertexElemenets( CMMA_VertexSet& rVertexSet );

	HRESULT FillIndexBuffer( LPD3DXMESH pMesh, C3DMeshModelArchive& archive );

	/// materials must be loaded before calling this method
	HRESULT SetAttributeTable( LPD3DXMESH pMesh, const vector<CMMA_TriangleSet>& vecTriangleSet );

	LPD3DXMESH LoadD3DXMeshFromArchive( C3DMeshModelArchive& archive );

	/// \param xfilename [in] .x file
	/// \param rpMesh [out] reference to a pointer that receives the mesh loaded by this function
	/// \param rpAdjacencyBuffer [out] reference to a pointer that receives adjacency buffer
	/// materials are also loaded from the input .x file and stored in CD3DXMeshObjectBase::m_pMeshMaterials
	HRESULT LoadD3DXMeshAndMaterialsFromXFile( const std::string& xfilename,
		                                       LPD3DXMESH& rpMesh,
		                                       LPD3DXBUFFER& rpAdjacencyBuffer );

	/// check the attribute tables (for debugging)
	void PeekAttribTables( LPD3DXBASEMESH pMesh );

	/// procedures to create bounding sphere from D3DXMesh
	/// - Deprecated, and nobody's using this. See this as a sample code
	/// - The bounding shpere should be create from mesh archives
	HRESULT CreateLocalBoundingSphereFromD3DXMesh( LPD3DXMESH pMesh );

	/// load mesh from .x file
	/// - Declared protected since this is a platform dependent
	///   and sould not be directly called by outside module
	///   unlike LoadFromArchive()
	virtual HRESULT LoadFromXFile( const std::string& filename ) = 0;

public:

	inline CD3DXMeshObjectBase();

	virtual ~CD3DXMeshObjectBase() { Release(); }

	/// returns true on success
	bool LoadFromFile( const std::string& filename );

	/// load basic mesh properties from a mesh archive
	/// NOTE: filename is required to get the path for textures files
	virtual bool LoadFromArchive( C3DMeshModelArchive& archive, const std::string& filename ) = 0;

	virtual void Release();

	virtual LPD3DXBASEMESH GetBaseMesh() { return NULL; }

	inline int GetNumMaterials() const { return m_NumMaterials; }

	inline const D3DMATERIAL9& GetMaterial( int i ) const { return m_pMeshMaterials[i]; }

	inline LPDIRECT3DVERTEXDECLARATION9 GetVertexDeclaration() { return m_pVertexDecleration; }

	/// user is responsible for updating the visibility by calling UpdateVisibility( const CCamera& cam )
	void ViewFrustumTest( bool do_test ) { m_bViewFrustumTest = do_test; }

	void UpdateVisibility( const CCamera& cam );

	inline bool IsMeshVisible( int triset_index ) const { return m_IsVisible[triset_index]==1 ? true : false; }


	inline bool IsMeshVisible() const { return m_IsVisible[m_NumMaterials]==1 ? true : false; }

	inline CTextureHandle& GetTexture( int material_index, int tex_index );

	/// renderes mesh using a shader technique
	/// this method assumes that you have set a valid shader technique that renders
	/// meshes with a texture sampler accessed by 'pTexHandle'
//	virtual void Render( LPD3DXEFFECT pEffect, D3DXHANDLE pTexHandle, D3DXHANDLE pNormalMapTexHandle = NULL );

	virtual void RenderSubsets( CShaderManager& rShaderMgr, const std::vector<int>& vecMaterialIndex /* some option to specify handles for texture */);

	/// Renderes mesh using a shader technique
	/// - Assumes that you have already set a valid technique that can be obtained from 'rShaderMgr'
	/// - 
	inline void Render( CShaderManager& rShaderMgr );

	inline void RenderSubset( CShaderManager& rShaderMgr, int material_index );

	enum eMeshType { TYPE_MESH, TYPE_PMESH, TYPE_SMESH, TYPE_DMESH };

	virtual unsigned int GetMeshType() const = 0;

};


class CMeshObjectFactory
{
public:

	CMeshObjectFactory() {}
	virtual ~CMeshObjectFactory() {}

	CD3DXMeshObjectBase* LoadMeshObjectFromFile( const std::string& filename, int mesh_type );

	/// TODO: support PMesh and SMesh
	CD3DXMeshObjectBase*  LoadMeshObjectFromArchvie( C3DMeshModelArchive& mesh_archive, const std::string& filepath, int mesh_type );

};



// ================================= inline implementations =================================


inline CD3DXMeshObjectBase::CD3DXMeshObjectBase()
:
m_NumMaterials(0L),
m_pMeshMaterials(NULL),
m_iVertexSize(0),
m_pVertexDecleration(NULL),
m_bViewFrustumTest(false)
{
}


inline CTextureHandle& CD3DXMeshObjectBase::GetTexture( int material_index, int tex_index )
{
	return m_vecMaterial[material_index].Texture[tex_index];;
}


/// \param vecMaterialIndex indices of materials(subsets) to render 
inline void CD3DXMeshObjectBase::Render( CShaderManager& rShaderMgr )
{
	RenderSubsets( rShaderMgr, m_vecFullMaterialIndices );
}


inline void CD3DXMeshObjectBase::RenderSubset( CShaderManager& rShaderMgr, int material_index )
{
	vector<int> single_index;
	single_index.push_back( material_index );

	RenderSubsets( rShaderMgr, single_index );
}


#endif		/*  __D3DXMESHOBJECTBASE_H__  */
