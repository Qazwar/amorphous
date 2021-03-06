#ifndef  __3DMath_AABTree_H__
#define  __3DMath_AABTree_H__


#include <vector>
#include <string>

#include "Vector3.hpp"
#include "AABB3.hpp"
#include "ray.hpp"

#include "amorphous/Support/Serialization/Serialization.hpp"
#include "amorphous/Support/Serialization/Serialization_3DMath.hpp"


namespace amorphous
{
using namespace serialization;


//===============================================================================
// AABTree
//   - creates a mesh object that holds geometries specified as template class
//   - the template class TGeometry needs to implement the following functions
//   - TGeometry must also be serializable. i.e.) must be derived from IArchiveObjectBase
//===============================================================================

/*
class TGeometry : public IArchiveObjectBase
{
public:

	/// returns the axis-aligned bounding box in world space
	const AABB3& GetAABB() const;

	/// return if the ray hits the polygon
	bool ClipTrace( SRay& ray ) const;

	/// returns true if the polygon should be checked for collision
	bool IsCollidable() const;

}
*/

/**
 STMBinNode - binary tree node for triangle mesh
 - each non-leaf node holds an axis-aligned plane, which is represented by 'iAxis' & 'fDist'.
 - 'iAxis' represents the plane normal along the corresponding axis
 - Holds indices to geometries
   - Instances of geometries are stored in AABTree instance that owns nodes
*/
class AABNode : public IArchiveObjectBase
{
public:

    /// axis of aa plane - 0:x, 1:y, 2:z
	int iAxis;

	/// distance of plane
	float fDist;

	/// aabb to hold subspace represented by this node
	AABB3 aabb;

	/// indices to child nodes
	int child[2];

	int iCellIndex;

	/// indices to geometries which belong to this node.
	/// - In a leafy binary tree,
	///   - Only the leaf nodes hold polygons, so this will be empty for non-leaf nodes
	///   - Geometries which cross subspaces belong to the both of the leaf nodes
	/// - In a non-leafy binary tree,
	///   - Geometries that cross a subspace belong to the internal nodes
	std::vector<int> veciGeometryIndex;

	int depth; ///< zero-origin depth

	static int ms_DefaultReserveSize;

	enum NodeType
	{
		LEAF = -1,
	};

public:

	AABNode() : iCellIndex(-1), iAxis(0), fDist(0), depth(0) { child[0] = child[1] = LEAF; }

	inline bool IsLeaf() const { return ( (child[0] == LEAF) && (child[1] == LEAF) ); }

	inline void AddIndex( int index ) { veciGeometryIndex.push_back( index ); }

	inline void Clear() { veciGeometryIndex.resize(0); }

	inline void Serialize( IArchive& ar, const unsigned int version );
};


inline void AABNode::Serialize( IArchive& ar, const unsigned int version )
{
	ar & iAxis;
	ar & fDist;
	ar & aabb;
	ar & child[0] & child[1];
	ar & iCellIndex;
	ar & veciGeometryIndex;
	ar & depth;
}


//===============================================================================
// AABTree
//===============================================================================

extern void  WriteNodeToFile_r( int node_index, std::vector<AABNode>& nodes, int depth, FILE *fp );

enum class AABTreeType
{
	LEAFY,
	NON_LEAFY,
	NUM_TREE_TYPES
};

template<class TGeometry>
class AABTree : public IArchiveObjectBase
{
protected:

	/**
	Usually a particular type of objects, such as spheres and boxes, managed by an aab tree.
	*/
	std::vector<TGeometry> m_vecGeometry;

	std::vector<AABNode> m_vecNode;

	int m_TreeDepth;

	/// world position of the tree
	Vector3 m_vWorldPos;

	/// controls recursion
	/// - RecursionStopCond::COND_AND or RecursionStopCond::COND_OR
	int m_RecursionStopCond;

	/// used to control recursion depth
	int m_MaxDepth;

	/// used to control recursion depth
	int m_iNumMaxTrianglesPerCell;

	/// used to control recursion depth
	float m_fMinimumCellVolume;

	/// used at runtime as a stack of node indices to check for collisions
	std::vector<int> m_vecNodeToCheck;

	/// used at runtime to store geometry indices that are possibly hitting the ray / linesegment
//	std::vector<int> m_vecGeometryToCheck;

private:

	inline void BuildLA_r( int index, int depth );

protected:

	void InitRootNode();

public:

	enum RecursionStopCond
	{
		COND_AND, ///< i.e. loose recursion stopper
		COND_OR,  ///< i.e. strict recursion stopper
	};

	inline AABTree();

	inline virtual ~AABTree();

	inline void Release();

	std::vector<TGeometry>& GetGeometryBuffer() { return m_vecGeometry; }

	virtual AABTreeType GetTreeType() const = 0;

	/// creates an empty tree
	/// - Declare as virtual and have derived class call this
	///   - Avoid name conflicts with Build( const std::vector<TGeometry>& vecGeometry ) below.
	inline virtual void Build( const AABB3& rBoundingBox, const int depth );

	/// Creates a tree from the given geometries
	/// - Creates a copy of the instances of geometry and store it to a member variable
	void Build( const std::vector<TGeometry>& vecGeometry )
	{
		m_vecGeometry = vecGeometry;

		Build();
	}

	// creates the tree from 'm_vecGeometry'
	virtual void Build() = 0;

	const AABNode& GetNode( int index ) const { return m_vecNode[index]; }

	int GetNumNodes() const { return (int)m_vecNode.size(); }

	std::vector<AABNode>& GetNodeBuffer() { return m_vecNode; }

	int GetTreeDepth() const { return m_TreeDepth; }

	/// set tree position in world space
	void SetWorldPosition( const Vector3& rvWorldPos ) { m_vWorldPos = rvWorldPos; }

	void SetGeometry( const std::vector<TGeometry>& geometry ) { m_vecGeometry = geometry; }

	void SetMaxDepth( int max_depth ) { m_MaxDepth = max_depth; }

	void SetMinimumCellVolume( float volume ) { m_fMinimumCellVolume = volume; }

	void SetNumMaxGeometriesPerCell( int num_max_triangles ) { m_iNumMaxTrianglesPerCell = num_max_triangles; }

	inline void SetRecursionStopCondition( const std::string& cond );

	inline void SetRecursionStopCondition( RecursionStopCond condition ) { m_RecursionStopCond = condition; }

	inline bool ShouldStopRecursion( int depth, float sub_space_volume, int num_triangles_in_cell );

	/// Get a list of indices of geometry whose aabb intersects with 'aabb'
	/// - Checks overlaps between abbb and geometries
	/// \param [in] aabb
	/// \param [out] rvecDestIndex dest buffer that holds indices of geometries
	///              intersecting with aabb
	inline void GetIntersectingGeometries( const AABB3& aabb, std::vector<int>& rvecDestIndex );

	/// Get a list of indices of geometry whose aabb intersects with 'aabb'
	/// - Checks overlaps between abbb and geometry aabbs
	/// - Does not mean geometry itself intersects with the aabb
	/// - Calls TGeometry::GetAABB() inside
	/// \param [in] aabb
	/// \param [out] dest buffer to store indices
	virtual void GetIntersectingAABBs( const AABB3& aabb, std::vector<int>& rvecDestIndex ) = 0;

	/// get AABBs which possibly include the specified position 'vPos'
	inline void GetPossiblyIntersectingAABBs( const Vector3& vPos, std::vector<int>& rvecDestIndex );

	/// Add a geometry to the list and link it to a tree node
	/// - Assumes that the tree has already been contructed
	inline void AddGeometry( const TGeometry& geom );

	/// Clear geometry indices from nodes
	/// - Does not remove the geometry instances. Only the indices in the nodes
	inline void ResetRegisteredGeometries();

	/// - Call ResetRegisteredGeometries()
	/// - Link geoms to the tree nodes
	inline void UpdateRegisteredGeometries();

	virtual void LinkGeometry( int geom_index ) = 0;

	/// split a leaf node in half
	/// - precondition: the argument node must be a leaf
	inline void Subdivide( AABNode& node, int axis )
	{
		node.iAxis = axis;
		float fMidDist = node.aabb.GetCenterPosition()[axis];
		node.fDist = fMidDist;

		int child0 = node.child[0] = (int)m_vecNode.size();
		int child1 = node.child[1] = (int)m_vecNode.size() + 1;
		m_vecNode.insert( m_vecNode.end(), 2, AABNode() );

		m_vecNode[child0].aabb = node.aabb;
		m_vecNode[child0].aabb.vMin[axis] = fMidDist;

		m_vecNode[child1].aabb = node.aabb;
		m_vecNode[child1].aabb.vMax[axis] = fMidDist;
	}

	inline void CreateRootNode( const AABB3& root_aabb )
	{
		m_vecNode.resize( 1 );
		m_vecNode[0].aabb = root_aabb;
	}

//	virtual void ClipTrace( const CLineSegment& line_segment, CLineSegmentHit& results ) = 0;

	inline virtual void Serialize( IArchive& ar, const unsigned int version );


	void WriteToFile( const std::string& filepath )
	{
		FILE* fp = fopen( filepath.c_str(), "w" );
		if( !fp )
			return;

		fprintf( fp, "Depth: %d\n", GetTreeDepth() );
		fprintf( fp, "Nodes: %d\n", GetNumNodes() );
		fprintf( fp, "\n" );

		if( 0 < m_vecNode.size() )
			WriteNodeToFile_r( 0, m_vecNode, 0, fp );

		fclose(fp);
	}
};


/**
 - leafy binary tree
 - Not thread safe
   - See m_TestCounter and how it is used
*/
template<class TGeometry>
class LeafyAABTree : public AABTree<TGeometry>
{
	int m_TestCounter;

	// test counters for each geometry
	std::vector<int> m_vecTestCounter;

public:

	LeafyAABTree() : m_TestCounter(0) {}

	virtual ~LeafyAABTree() {}

	AABTreeType GetTreeType() const { return AABTreeType::LEAFY; }

	void Build( const AABB3& rBoundingBox, const int depth ) { AABTree<TGeometry>::Build( rBoundingBox, depth ); }

	void Build( const std::vector<TGeometry>& vecGeometry ) { AABTree<TGeometry>::Build( vecGeometry ); }

	/// \param [in] vecGeometry copied and stored
	inline void Build();

	inline virtual void GetIntersectingAABBs( const AABB3& aabb, std::vector<int>& rvecDestIndex );

	inline virtual void LinkGeometry( int geom_index ) {}

//	virtual void ClipTrace( const CLineSegment& line_segment, CLineSegmentHit& results ) {}

	inline void Serialize( IArchive& ar, const unsigned int version );
};


template<class TGeometry>
class CNonLeafyAABTree : public AABTree<TGeometry>
{
public:

	CNonLeafyAABTree() {}

	virtual ~CNonLeafyAABTree() {}

	AABTreeType GetTreeType() const { return AABTreeType::NON_LEAFY; }

	/// update the link of the geometry[index]
	inline void UpdateGeometry( int index );

	void Build( const AABB3& rBoundingBox, const int depth ) { AABTree<TGeometry>::Build( rBoundingBox, depth ); }

	/// Cannot call AABTree::Build() from an instance of CNonLeafyAABTree. Why?
	void Build( const std::vector<TGeometry>& vecGeometry ) { AABTree<TGeometry>::Build( vecGeometry ); }

	/// \param [in] vecGeometry copied and stored
	inline void Build();

	inline virtual void GetIntersectingAABBs( const AABB3& aabb, std::vector<int>& rvecDestIndex );

	inline virtual void LinkGeometry( int geom_index );

//	void ClipTrace( const CLineSegment& line_segment, CLineSegmentHit& results );
};

/*
void CNonLeafyAABTree::ClipTrace( const CLineSegment& line_segment, CLineSegmentHit& results )
{
	line_segment
	m_vecGeometryToCheck.resize( 0 );

	AABB3 aabb = // create aabb from line_segment
	GetIntersecting( m_vecGeometryToCheck, aabb );

	size_t i, num = m_vecGeometryToCheck.size();

	if( num == 0 )
		return;	// no intersection

	for( i=0; i<num; i++ )
	{
	}
}
*/

} // namespace amorphous


#include "AABTree.inl"


#endif		/*  __3DMath_AABTree_H__  */
