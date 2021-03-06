#ifndef __BrickWallPolygonModelMaker_HPP__
#define __BrickWallPolygonModelMaker_HPP__


#include <vector>
#include "Vector3.hpp"
#include "range.hpp"


namespace amorphous
{


template<typename T>
class brick_wall_panel_desc
{
public:

	unsigned int num_horizontal_bricks;
	unsigned int num_vertical_bricks;

	/// 0: shift the bricks on the odd rows, 1: shift the bricks on the even rows, -1: do not shift bricks
	int alternate_shift;

	T inset;
	T extrusion;
	T interval;

//	std::pair<float,float> size_variations;
//	std::pair<float,float> inset_variations;
//	std::pair<float,float> extrusion_variations;
	range<T> size_variations;
	range<T> position_variations;

	/// If true, adds a rectangle polygon as a base layer
	/// of the brick wall panel. More specifically,
	/// - 4 vertices are added to the end of 'positions'.
	/// - One 4-point polygon is added to the end of 'polygons'
	/// in CreateBrickWallPanel().
	bool make_base_layer;

public:

	brick_wall_panel_desc()
	:
	num_horizontal_bricks(8),
	num_vertical_bricks(16),
	alternate_shift(1),
	inset(0.008f),
	extrusion(0.008f),
	interval(0.005f),
	make_base_layer(true)
	{}
};



template<typename T>
void CreateBrickWallPanel(
	const brick_wall_panel_desc<T>& desc,
	std::vector< tVector3<T> >& positions,
	std::vector< std::vector<unsigned int> >& polygons
	);


} // namespace amorphous



#endif /* __BrickWallPolygonModelMaker_HPP__ */
