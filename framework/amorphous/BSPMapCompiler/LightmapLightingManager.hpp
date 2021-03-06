#ifndef  __LightmapLightingManager_H__
#define  __LightmapLightingManager_H__


#include <vector>
#include <thread>
#include <memory>

#include "amorphous/3DMath/Vector3.hpp"
#include "amorphous/3DMath/Plane.hpp"
#include "amorphous/3DMath/ray.hpp"
#include "amorphous/3DMath/MathMisc.hpp"
#include "amorphous/3DMath/AABTree.hpp"
#include "amorphous/Graphics/FloatRGBColor.hpp"
#include "amorphous/Graphics/LightStructs.hpp"
#include "amorphous/Support/SafeDeleteVector.hpp"
//#include "amorphous/Support/StatusDisplay/StatusDisplay.hpp"
#include "Lightmap.hpp"

namespace amorphous
{



class CLightmapLightingManager;

enum eFlag
{
	LF_IGNORE_ANGLE_FACTOR				= (1 << 0),	// ignore angle attenuation
	LF_CREATE_LIGHT_DIRECTION_TEXTURE	= (1 << 1),	// create light direction maps (for bump mapping)
	LF_USE_HEMISPHERIC_LIGHT			= (1 << 2),	// use hemispheric lighting to create lightmaps
};


class CLightRaytrace
{
protected:

	std::shared_ptr<Light> m_pLight;

	/// polygon mesh for ray check
	/// - borrowed reference
	AABTree<IndexedPolygon> *m_pGeometry;

public:

	CLightRaytrace( std::shared_ptr<Light> pLight ) : m_pLight(pLight) {}

	virtual ~CLightRaytrace() {}

	virtual void CalcLightAmount( CLightmap& rLightmap, int x, int y ) = 0;

	void SetGeometry( AABTree<IndexedPolygon> *pGeometry ) { m_pGeometry = pGeometry; }
};


class CPointLightRaytrace : public CLightRaytrace
{
	Vector3 m_vLightPosition;

public:

	CPointLightRaytrace( std::shared_ptr<Light> pLight ) : CLightRaytrace(pLight) {}

	inline virtual void CalcLightAmount( CLightmap& rLightmap, int x, int y );

};


class CDirectionalLightRaytrace : public CLightRaytrace
{
	Vector3 m_vLightDirection;

public:

	CDirectionalLightRaytrace( std::shared_ptr<Light> pLight ) : CLightRaytrace(pLight) {}

	inline virtual void CalcLightAmount( CLightmap& rLightmap, int x, int y );

};


/// task executed by a thread
class CLightmapRaytraceTask
{
	CLightmap *m_pLightmap;

	std::vector<CLightRaytrace *> m_pLightRaytrace;

	bool m_bDone;

	CLightmapLightingManager *m_pMgr;

	std::shared_ptr<AABTree<IndexedPolygon>> m_pGeometry;

public:

	CLightmapRaytraceTask() : m_pMgr(NULL), m_pLightmap(NULL), m_bDone(false) {}

//	CLightmapRaytraceTask(CLightmapLightingManager *pMgr) : m_pMgr(pMgr), m_pLightmap(NULL), m_bDone(false) {}

	~CLightmapRaytraceTask()
	{
		SafeDeleteVector( m_pLightRaytrace );
	}

	void RunTask();

	void SetLightRaytraceTasks( std::vector<CLightRaytrace *>& light_raytrace )
	{
		m_pLightRaytrace = light_raytrace;

		const size_t num = m_pLightRaytrace.size();
		for( size_t i=0; i<num; i++ )
			m_pLightRaytrace[i]->SetGeometry( m_pGeometry.get() );
	}

	void SetLightmapLightingManager( CLightmapLightingManager *pMgr ) { m_pMgr = pMgr; }

	void ThreadMain();

    void operator()();

	friend class CLightmapLightingManager;
};


class CLightmapLightingManager
{
	/// pointer to the array of pointers to lights
	std::vector<std::shared_ptr<Light>> *m_pvecpLight;

	std::vector<CLightmap> *m_pvecLightmap;

	AABTree<IndexedPolygon> *m_pGeometry;

	/// ambient light: global & uniform lighting for the entire map
	AmbientLight m_AmbientLight;

	float m_fContrast;

	float m_fSurfaceErrorTolerance;

	float m_fDirectionalLightDistance;

	unsigned int m_OptionFlag;
	
	int m_NumTotalLightmapTexels;

	int m_NumProcessedLightmapTexels;

	int m_CurrentLightmapIndex;

	boost::mutex m_Mutex;


	/// point lights
//	vector<CPointLight *> m_vecpPointLight;
	/// directional lights
//	vector<CDirectionalLight *>m_vecpDirectionalLight;
	/// zone ambient lights (experimental)
//	vector<CZoneAmbientLight *>m_vecpZoneAmbientLight;
//	float m_fAmbientIntensity;
//	SFloatRGBColor m_AmbientColor;

private:

	std::vector<CLightRaytrace *> CreateLightRaytraceTasks( std::vector<std::shared_ptr<Light>> *pvecpLight );

public:

	CLightmapLightingManager();

	bool CreateLightmaps( std::vector<std::shared_ptr<Light>> *pvecpLight,
                          std::vector<CLightmap> *pvecLightmap,
						  AABTree<IndexedPolygon> *pGeometry );

//	void SetLights( vector<CLight *>& rvecpLight ) { m_pvecpLight = &rvecpLight; }

//	bool Calculate( vector<CLightmap>& rvecLightmap, CPolygonMesh<T>& rMesh );

	void ScaleIntensityAndAddAmbientLight();

	CLightmap *GetLightmapForRaytraceTask();

	inline void RaiseOptionFlag( const unsigned int flag ) { m_OptionFlag |= flag; }

	inline unsigned int GetOptionFlag() const { return m_OptionFlag; }

	inline void ClearOptionFlag( const unsigned int flag ) { m_OptionFlag &= (~flag); }
};


inline CLightmap *CLightmapLightingManager::GetLightmapForRaytraceTask()
{
	std::lock_guard<std::mutex> scoped_lock(m_Mutex);

	if( m_CurrentLightmapIndex < (int)m_pvecLightmap->size() )
	{
		int index = m_CurrentLightmapIndex;
		m_CurrentLightmapIndex += 1;

		return &(*m_pvecLightmap)[index];
	}
	else
		return NULL;
}


/**
 \param lightmap [in,out] lightmap to update
 \param x [in]
 \param y [in]

*/
inline void CPointLightRaytrace::CalcLightAmount( CLightmap& rLightmap, int x, int y )
{
	const SPlane& plane = rLightmap.GetPlane();

	float dist = plane.GetDistanceFromPoint( m_vLightPosition );

//	if( dist < 0.0 )
//		return;			//The light is behind the faces

	const Vector3 vLightmapPoint = rLightmap.GetPoint(x,y);

	CLineSegment line_segment;
	CLineSegmentHit results;
	// check the ray from the point on the lightmap to the position of the light
	line_segment.vStart = m_pLight->GetPosition();
///	line_segment.vGoal  = rLightmap.GetPoint(x,y);

	const Vector3 vToLight = m_pLight->GetPosition() - vLightmapPoint;
	const float fRealDist = Vec3Length(vToLight);
	Vector3 vDirToLight = vToLight / fRealDist;
//	line_segment.vGoal = vLightmapPoint + vToLight_n * m_fSurfaceErrorTolerance;
	line_segment.vGoal = vLightmapPoint + plane.normal * 0.01f;//m_fSurfaceErrorTolerance;

	AABB3 aabb;
	aabb.Nullify();
	aabb.AddPoint( line_segment.vStart );
	aabb.AddPoint( line_segment.vGoal );

	std::vector<int> vecPolygonsToCheck;

//	m_pGeometry->GetIntersectingAABBs( aabb, vecPolygonsToCheck );

//	IsRayBlocked()

	if( false /*line_segment.fFraction < 1.0f*/ )
	{
		// ray is blocked
		Vector3 vLightDir = rLightmap.GetLightDirection(x,y);
		vLightDir += vDirToLight * 0.1f;//pPointLight->fIntensity * 0.1f;
		rLightmap.SetLightDirection( x, y, vLightDir );

		return;
	}

		// the light is not obstructed and reaching the target point
///			vToLight = pPointLight->vPosition - rLightmap.GetPoint(x,y);
///			fRealDist = Vec3Length(&vToLight);

	SFloatRGBColor color = m_pLight->CalcLightAmount( vLightmapPoint, rLightmap.GetNormal(x,y) );

	// >>> check dist calc
//	float val = fRealDist * 0.1f;
//	Limit( val, 0.0f, 1.0f );
//	color = SFloatRGBColor( val, val, val );
	// <<<< check dist calc

	rLightmap.AddTexelColor( x, y, color );

//		fAttenuation = 1.0f / ( pPointLight->fAttenuation0
//					          + pPointLight->fAttenuation1 * fRealDist
//						      + pPointLight->fAttenuation2 * fRealDist * fRealDist );

//		if (fAttenuation>1) fAttenuation=1;

//		fAngle = Vec3Dot( vToLight_n, rLightmap.GetNormal(x,y) );	// use normal calculated for each lightmap texel (for smooth shading)

/*		if( m_OptionFlag & LF_IGNORE_ANGLE_FACTOR )
		{
			// exclude angle factor
			// angle attenuation is calculated by using the light direction map during runtime
			fPower = pPointLight->fIntensity * fAttenuation;
		}
		else if( m_OptionFlag & LF_USE_HEMISPHERIC_LIGHT )
		{
			float f = ( fAngle + 1.0f ) * 0.5f;
			fPower = pPointLight->fIntensity * fAttenuation * f;
		}
		else
		{
			fPower = pPointLight->fIntensity * fAttenuation * fAngle;
		}

		if( fPower < 0.0 )
			continue;
*/

		// ========================= normal pixel intensity calculation ===========================
//		color = rLightmap.GetTexelColor(x,y);
//		rLightmap.SetTexelColor(x,y, color + pPointLight->Color * fPower );

		// ========= replace lightmap colors with normal directions (visual debugging) ============
//				Vector3& rvNormal = rLightmap.avNormal[ LMTexelAt(x,y) ];
//				rLightmap.intensity[ LMTexelAt(x,y) ].fRed   = (float)fabs(rvNormal.x);
//				rLightmap.intensity[ LMTexelAt(x,y) ].fGreen = (float)fabs(rvNormal.y);
//				rLightmap.intensity[ LMTexelAt(x,y) ].fBlue  = (float)fabs(rvNormal.z);

		if( true /*m_iLightmapCreationFlag & LMB_CREATE_LIGHT_DIRECTION_MAP_TEXTURE*/ )
		{
//					vNormal = rLightmap.GetNormal(x,y);
//					rLightmap.SetNormal(x,y, vNormal + vToLight_n * pPointLight->fIntensity );
			Vector3 vLightDir = rLightmap.GetLightDirection(x,y);
			vLightDir += vDirToLight * m_pLight->fIntensity * 100.0f;
			rLightmap.SetLightDirection( x, y, vLightDir );
		}

//		num_processed_texels++;
//				CStatusDisplay::Get()->UpdateProgress( "Lightmap Raytrace",
//				                                       (float)(num_processed_texels) / (float)num_total_texels );

//		Sleep( 5 );
}


inline void CDirectionalLightRaytrace::CalcLightAmount( CLightmap& rLightmap, int x, int y )
{
//				float dist = rPlane.GetDistanceFromPoint( pDirLight->vPosition );
//				if( dist < 0.0 )

	const Vector3 vDirToLight = - m_pLight->GetDirection();
	Vector3 vLightDir = Vector3(0,0,0);

	const SPlane& plane = rLightmap.GetPlane();

	float d = Vec3Dot( vDirToLight, plane.normal );
	if( d < 0 )
	{
		// The light is behind the faces
		// add small light direction
		vLightDir = rLightmap.GetLightDirection(x,y);
		vLightDir += ( plane.normal + Vector3( 0, vDirToLight.y, 0 ) ) * 0.01f;
		rLightmap.SetLightDirection( x, y, vLightDir );

		return;
	}

	const Vector3 vLightmapPoint = rLightmap.GetPoint(x,y);
/*
	CLineSegment line_segment;
	line_segment.vStart = vLightmapPoint + vDirToLight * 100.0f;//m_fDirectionalLightDistance;
//	line_segment.vGoal  = vLightmapPoint + vDirToLight * m_fSurfaceErrorTolerance;
	line_segment.vGoal  = vLightmapPoint + plane.normal * 0.01f;
	line_segment.fFraction = 1.0f;

	m_pGeometry->RayTrace( ray );
*/
	if( false /*line_segment.fFraction < 1.0f*/ )
	{
		// ray is blocked
		vLightDir = rLightmap.GetLightDirection(x,y);
		vLightDir += vDirToLight * 0.01f;
		rLightmap.SetLightDirection( x, y, vLightDir );

		return;
	}

	SFloatRGBColor color = m_pLight->CalcLightAmount( vLightmapPoint, rLightmap.GetNormal(x,y) );

	rLightmap.AddTexelColor( x, y, color );

	// the light is not obstructed and reaching the target point
/*
//				fAngle = Vec3Dot( vToLight_n, rPlane.normal );	// use face normal (for flat shading)
	fAngle = Vec3Dot( vDirToLight, rLightmap.GetNormal(x,y) );	// use normal calculated for each lightmap texel (for smooth shading)

	if( true ) // m_iLightmapCreationFlag & LMB_CREATE_LIGHT_DIRECTION_MAP_TEXTURE )
	{	// exclude angle factor
		// angle attenuation is calculated by using the light direction map during runtime
		fPower = pDirLight->fIntensity;
	}
	else
	{
		fPower = pDirLight->fIntensity * fAngle;
	}

	if( fPower < 0.0 )
		continue;

	// ========================= normal pixel intensity calculation ===========================
	color = rLightmap.GetTexelColor(x,y);
	rLightmap.SetTexelColor(x,y, color + pDirLight->Color * fPower );

	// ========= replace lightmap colors with normal directions (visual debugging) ============
//				Vector3& rvNormal = rLightmap.avNormal[ LMTexelAt(x,y) ];
//				rLightmap.intensity[ LMTexelAt(x,y) ].fRed   = (float)fabs(rvNormal.x);
//				rLightmap.intensity[ LMTexelAt(x,y) ].fGreen = (float)fabs(rvNormal.y);
//				rLightmap.intensity[ LMTexelAt(x,y) ].fBlue  = (float)fabs(rvNormal.z);
*/
	if( true )//m_iLightmapCreationFlag & LMB_CREATE_LIGHT_DIRECTION_MAP_TEXTURE )
	{
		vLightDir = rLightmap.GetLightDirection(x,y);
		vLightDir += vDirToLight; // * pDirLight->fIntensity;
		rLightmap.SetLightDirection( x, y, vLightDir );
	}

}



/*
template <class T>
void CLightingForLightmap_SimpleRaytrace<T>::CalculateLightmapTexelIntensity( CLightmap& rLightmap,
																			  CPolygonMesh<T>& rMesh )
{
	int i,x,y;
	Vector3 vToLight, vToLight_n;
//	int iNumLights = m_pvecpLight->size();
	int iNumLights = m_vecpPointLight.size();
	CPointLight* pPointLight = NULL;
	float fRealDist, fAttenuation, fPower, fAngle;

	int iLightmapWidth  = rLightmap.GetNumPoints_X();
	int iLightmapHeight = rLightmap.GetNumPoints_Y();

	Vector3 vLightmapPoint;
	Vector3 vNormal;
	Vector3 vLightDir;
	SFloatRGBColor color;

	int num_processed_texels = m_NumProcessedLightmapTexels;
	const int num_total_texels = m_NumTotalLightmapTexels;

	SPlane& rPlane = rLightmap.GetPlane();
	CLineSegment line_segment;
	for(i=0; i<iNumLights; i++)
	{
		pPointLight = (CPointLight *)(m_vecpPointLight[i]);

		for(y=0; y<iLightmapHeight; y++)
		{
			for(x=0; x<iLightmapWidth; x++)
			{

			}
		}
	}
	
	m_NumProcessedLightmapTexels = num_processed_texels;

}
*/

} // amorphous



#endif  /*  __LightmapLightingManager_H__  */
