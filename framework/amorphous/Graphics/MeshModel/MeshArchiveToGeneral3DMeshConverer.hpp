#ifndef __MeshArchiveToGeneral3DMeshConverer_HPP__
#define __MeshArchiveToGeneral3DMeshConverer_HPP__


#include "amorphous/base.hpp"
#include "amorphous/Graphics/fwd.hpp"


namespace amorphous
{


class CMeshArchiveToGeneral3DMeshConverer
{
public:
	CMeshArchiveToGeneral3DMeshConverer(){}
	~CMeshArchiveToGeneral3DMeshConverer(){}

	/**
	Limitations
	- Only copies positions of vertices

	*/
	Result::Name Convert( const C3DMeshModelArchive& src, General3DMesh& dest );
};


} // namespace amorphous



#endif /* __MeshArchiveToGeneral3DMeshConverer_HPP__ */
