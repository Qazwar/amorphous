#ifndef  __D3DXSMESHOBJECT_H__
#define  __D3DXSMESHOBJECT_H__


#include "D3DXPMeshObject.hpp"
#include "Graphics/fwd.hpp"
#include "Graphics/MeshModel/MeshBone.hpp"


namespace amorphous
{


/**
 mesh with skeleton hierarchy
 - Cannot be loaded from .x mesh
*/
class CD3DXSMeshObject : public CD3DXPMeshObject
{
	/// holds vertex blend matrices (world transforms)
	Transform *m_paWorldTransforms;

	int m_iNumBones;

	/// hold pointers to each bone in a single array
	/// used to set matrices with indices
	std::vector<MeshBone *> m_vecpBone;

	/// root node of the hierarchical structure
    MeshBone *m_pRootBone;

	std::vector<Matrix34> m_vecLocalTransformCache;

//	std::vector<Transform> m_vecLocalTransformCache;

private:

	bool LoadSkeletonFromArchive( C3DMeshModelArchive& archive );

public:

	CD3DXSMeshObject();

	CD3DXSMeshObject( const std::string& filename );

	~CD3DXSMeshObject();

	virtual void Release();

	virtual bool LoadFromArchive( C3DMeshModelArchive& archive, const std::string& filename, U32 option_flags );

	virtual bool LoadFromArchive( C3DMeshModelArchive& archive, const std::string& filename, U32 option_flags, int num_pmeshes );

	inline int GetNumBones() const { return m_iNumBones; }

	inline int GetBoneMatrixIndexByName( const std::string& bone_name ) const;

//	inline const MeshBone& GetBone( int index ) const;

	inline const MeshBone& GetBone( const std::string& bone_name ) const;

	inline const MeshBone& GetRootBone() const { return m_pRootBone ? *m_pRootBone : MeshBone::NullBone(); }

//	inline void SetLocalTransform( int matrix_index, const Matrix34 *local_transform );

	/// calculate hierarchical transforms by handing an array of matrices
	/// that represents local transformations at each bone
	inline void CalculateBlendTransforms( Matrix34 *paLocalTransform );

	/// Set local transformation for a bone to a cache.
	/// NOTE: this function does not update the vertex blend matrix of the bone.
	/// User is responsible for calling SetLocalTransformsFromCache() later to update the vertex blend matrix of the bone.
	inline void SetLocalTransformToCache( int index, const Matrix34& local_transform );

	inline void SetLocalTransformToCache( int index, const Transform& local_transform );

	inline void CalculateBlendTransformsFromCachedLocalTransforms();

	inline void ResetLocalTransformsCache();

	inline void CalculateBlendTransforms( const std::vector<Transform>& src_local_transforms, std::vector<Transform>& dest_blend_transforms );

	/// Returns the pointer to the array of vertex blend matrices (world transforms)
	inline Transform* GetBlendTransforms() { return m_paWorldTransforms; }

	inline void GetBlendTransforms( std::vector<Transform>& dest_transforms );

	void DumpSkeletonToTextFile( const std::string& output_filepath ) const;

	virtual MeshType::Name GetMeshType() const { return MeshType::SKELETAL; }
};


inline int CD3DXSMeshObject::GetBoneMatrixIndexByName( const std::string& bone_name ) const
{
	if( m_pRootBone )
		return m_pRootBone->GetBoneMatrixIndexByName_r( bone_name.c_str() );
	else
		return -1;
}

/*
inline const MeshBone& CD3DXSMeshObject::GetBone( int index ) const
{
}
*/

inline const MeshBone& CD3DXSMeshObject::GetBone( const std::string& bone_name ) const
{
	if( m_pRootBone )
		return m_pRootBone->GetBoneByName_r( bone_name.c_str() );
	else
		return MeshBone::NullBone();
}


inline void CD3DXSMeshObject::CalculateBlendTransforms( Matrix34 *paLocalTransform )
{
	if( !m_pRootBone )
		return;

	int index = 0;
	m_pRootBone->Transform_r( NULL, paLocalTransform, index );
}


inline void CD3DXSMeshObject::SetLocalTransformToCache( int index, const Matrix34& local_transform )
{
	if( 0 <= index && index < (int)m_vecLocalTransformCache.size() )
		m_vecLocalTransformCache[index] = local_transform;
}


inline void CD3DXSMeshObject::SetLocalTransformToCache( int index, const Transform& local_transform )
{
//	if( 0 <= index && index < (int)m_vecLocalTransformCache.size() )
//		m_vecLocalTransformCache[index] = local_transform;
}


inline void CD3DXSMeshObject::CalculateBlendTransformsFromCachedLocalTransforms()
{
	if( 0 < m_vecLocalTransformCache.size() )
		CalculateBlendTransforms( &m_vecLocalTransformCache[0] );
}


inline void CD3DXSMeshObject::ResetLocalTransformsCache()
{
	m_vecLocalTransformCache.resize( m_vecLocalTransformCache.size(), Matrix34Identity() );
}


inline void CD3DXSMeshObject::CalculateBlendTransforms( const std::vector<Transform>& src_local_transforms, std::vector<Transform>& dest_blend_transforms )
{
	if( !m_pRootBone )
		return;

	if( (int)src_local_transforms.size() != m_iNumBones )
		return;

	dest_blend_transforms.resize( m_iNumBones );

	int index = 0;
	m_pRootBone->CalculateBlendTransforms_r( NULL, &(src_local_transforms[0]), &(dest_blend_transforms[0]), index );
}


inline void CD3DXSMeshObject::GetBlendTransforms( std::vector<Transform>& dest_transforms )
{
	if( !m_pRootBone )
		return;

	if( m_vecLocalTransformCache.empty() )
		return;

	dest_transforms.resize( m_iNumBones );

	std::vector<Transform> src_local_transforms;
	src_local_transforms.resize( m_vecLocalTransformCache.size() );
	for( size_t i=0; i<m_vecLocalTransformCache.size(); i++ )
		src_local_transforms[i].FromMatrix34( m_vecLocalTransformCache[i] );

	int index = 0;
	m_pRootBone->CalculateBlendTransforms_r( NULL, &(src_local_transforms[0]), &(dest_transforms[0]), index );
}


/*
inline void CD3DXSMeshObject::SetLocalTransform( int matrix_index, const Matrix34 *local_transform )
{
	if( m_vecpBone.size() == 0 || m_vecpBone.size() <= matrix_index )
		return;

	m_vecpBone[matrix_index]->SetLocalTransform( local_transform );
}*/


} // namespace amorphous



#endif  /*  __D3DXSMESHOBJECT_H__  */
