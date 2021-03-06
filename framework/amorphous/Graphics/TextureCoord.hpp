#ifndef __TEXTURECOORD_H__
#define __TEXTURECOORD_H__


namespace amorphous
{

class TEXCOORD2
{
public:

	float u,v;

	TEXCOORD2() : u(0), v(0) {}

	TEXCOORD2( float _u, float _v ) : u(_u), v(_v) {}

	inline TEXCOORD2 operator+( TEXCOORD2 t ) const;
	inline TEXCOORD2 operator-( TEXCOORD2 t ) const;

	inline TEXCOORD2 operator*( float f ) const;
	inline TEXCOORD2 operator/( float f ) const;
};


inline TEXCOORD2 TEXCOORD2::operator+( TEXCOORD2 t ) const
{
	TEXCOORD2 out;
	out.u = u + t.u;
	out.v = v + t.v;
	return out;
}


inline TEXCOORD2 TEXCOORD2::operator-( TEXCOORD2 t ) const
{
	TEXCOORD2 out;
	out.u = u - t.u;
	out.v = v - t.v;
	return out;
}


inline TEXCOORD2 TEXCOORD2::operator*( float f ) const
{
	TEXCOORD2 out;
	out.u = u * f;
	out.v = v * f;
	return out;
}


inline TEXCOORD2 TEXCOORD2::operator/( float f ) const
{
	TEXCOORD2 out;
	out.u = u / f;
	out.v = v / f;
	return out;
}

} // namespace amorphous


#endif  /*  __TEXTURECOORD_H__  */
