#ifndef __VEC3_STRINGAUX_H__
#define __VEC3_STRINGAUX_H__


#include "StringAux.hpp"

#include "3DMath/aabb3.hpp"
#include "3DMath/aabb2.hpp"


inline std::string to_string( const Vector3& v, int precision = 3, int num_zfills = 0 )
{
	std::string fmt_float = "%" + to_string(num_zfills) + "." + to_string(precision) + "f";
	std::string fmt_buffer = "( " + fmt_float + ", " + fmt_float + ", " + fmt_float + " )";

	char buffer[64];
	sprintf( buffer, fmt_buffer.c_str(), v.x, v.y, v.z );

	return std::string(buffer);
}


inline std::string to_string( const AABB3& aabb )
{
	return std::string( "aabb: min" + to_string(aabb.vMin) + " - max" + to_string(aabb.vMax) );
}


inline std::string to_string( const Vector2& v, int precision = 3, int num_zfills = 0 )
{
	std::string fmt_float = "%" + to_string(num_zfills) + "." + to_string(precision) + "f";
	std::string fmt_buffer = "( " + fmt_float + ", " + fmt_float + " )";

	char buffer[64];
	sprintf( buffer, fmt_buffer.c_str(), v.x, v.y );

	return std::string(buffer);
}


inline std::string to_string( const AABB2& aabb )
{
	return std::string( "aabb: min" + to_string(aabb.vMin) + " - max" + to_string(aabb.vMax) );
}


#endif  /*  __VEC3_STRINGAUX_H__  */
