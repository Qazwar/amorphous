#include "MeshBoneControllerBase.hpp"
#include "Graphics/Mesh/SkeletalMesh.hpp"
#include "XML/XMLNodeReader.hpp"

using namespace std;
using namespace boost;


//=================================================================================
// CBoneControlParam
//=================================================================================

CMeshBoneControllerBase::CBoneControlParam CMeshBoneControllerBase::CBoneControlParam::ms_NullObject;


void CMeshBoneControllerBase::CBoneControlParam::LoadFromXMLNode( CXMLNodeReader& reader )
{
	reader.GetChildElementTextContent( "Name", Name );
}


//=================================================================================
// CMeshBoneControllerBase
//=================================================================================

void CMeshBoneControllerBase::LoadFromXMLNode( CXMLNodeReader& reader )
{
	vector<CXMLNodeReader> param_reader = reader.GetImmediateChildren( "ControlParam" );

	for( size_t i=0; i<param_reader.size(); i++ )
	{
		CBoneControlParam param;
		param.LoadFromXMLNode( param_reader[i] );

		m_vecBoneControlParam.push_back( param );
	}
}

/*
void CMeshBoneControllerBase::UpdateTargetMeshTransforms()
{
	if( !m_pTargetMesh )
		return;

	size_t i, num = m_vecBoneControlParam.size();
	for( i=0; i<num; i++ )
	{
		CBoneControlParam& rParam = m_vecBoneControlParam[i];
		m_pTargetMesh->SetLocalTransformToCache( rParam.MatrixIndex, rParam.LocalTransform );
	}
}
*/

void CMeshBoneControllerBase::UpdateMeshBoneLocalTransforms( std::vector<Transform>& dest_mesh_bone_local_transforms )
{
	size_t i, num = m_vecBoneControlParam.size();
	for( i=0; i<num; i++ )
	{
		CBoneControlParam& rParam = m_vecBoneControlParam[i];

		if( 0 <= rParam.MatrixIndex && rParam.MatrixIndex < (int)dest_mesh_bone_local_transforms.size() )
			dest_mesh_bone_local_transforms[rParam.MatrixIndex].FromMatrix34( rParam.LocalTransform );
	}
}


/*
	m_vecTargetBoneName[ 0] = "FlapR";
	m_vecTargetBoneName[ 1] = "FlapL";
	m_vecTargetBoneName[ 2] = "WingR0";
	m_vecTargetBoneName[ 3] = "WingL0";
	m_vecTargetBoneName[ 4] = "WingR1";
	m_vecTargetBoneName[ 5] = "WingL1";
	m_vecTargetBoneName[ 6] = "WingR2";
	m_vecTargetBoneName[ 7] = "WingL2";
	m_vecTargetBoneName[ 8] = "FrontGear0";
	m_vecTargetBoneName[ 9] = "FrontGearBox0";
	m_vecTargetBoneName[10] = "FrontGear1";
	m_vecTargetBoneName[11] = "FrontGearBox1";
	m_vecTargetBoneName[12] = "RearGear0";
	m_vecTargetBoneName[13] = "RearGearBox0";
	m_vecTargetBoneName[14] = "RearGear1";
	m_vecTargetBoneName[15] = "RearGearBox1";
	m_vecTargetBoneName[] = "";
	m_vecTargetBoneName[] = "";
*/

/*
CMeshBoneControllerBase::CMeshBoneControllerBase()
{
}



void CMeshBoneController_Flap::SetTransform()
{
	size_t i, num = m_vecBoneControlParam.size();
	for( i=0; i<num; i++ )
	{
		CBoneControlParam& param = m_vecBoneControlParam[i];

		m_pTargetMesh->SetLocalTransformToCache( param.MatrixIndex, LocalTransform );
//		if( param.pBone )
//			param.pBone->m_LocalTransform = param.matOrient;
	}
}



void CMeshBoneControllerBase::Init()
{
	vector<CMeshBone>& vecBone;

	string tgt_bone_name;
	size_t i, num_bones = vecBone.size();
	for( i=0; i<num_bones; i++ )
	{
		CMeshBone& bone = vecBone[i];
		if( bone[i].name.find( "axis: " ) == 0 )
		{
			tgt_bone_name = bone[i].name.substr( strlen("axis: "), 1024 );
			CMeshBone* tgt_bone = mesh.GetBoneByName( tgt_bone_name );
			m_vecBoneControlParam[ GetIndex( tgt_bone_name ) ]
				= BoneControlParam( tgt_bone,  ??? axis ???  );
//			m_vecBoneControlParam.push_back( tgt_bone, bone[i].???vLocalOffset??? );
		}
	}
}
*/
