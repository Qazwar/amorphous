
#ifndef __KGL_FIXEDVECTOR_SERIALIZATION_H__
#define __KGL_FIXEDVECTOR_SERIALIZATION_H__

#include "../FixedVector.h"

#include "Archive.h"


namespace GameLib1
{

namespace Serialization
{


template <class T, int iArraySize>
inline IArchive& operator & ( IArchive& ar, TCFixedVector<T, iArraySize>& vec )
{
	int i, iSize;
	if( ar.GetMode() == IArchive::MODE_OUTPUT )
	{
		// record array size
		iSize = vec.size();
		ar & iSize;

		// record each element
		for( i=0; i<vec.size(); i++ )
			ar & vec[i];
	}
	else // i.e. input mode
	{
		// load array size
		ar & iSize;
		vec.resize(iSize);
		for( i=0; i<iSize; i++ )
			ar & vec[i];
	}

	return ar;
}


}	  /*  Serialization  */

}	  /*  GameLib1  */


#endif  /*  __KGL_FIXEDVECTOR_SERIALIZATION_H__  */
