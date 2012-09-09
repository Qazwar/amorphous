#include "CartridgeMaker.hpp"
#include "../3DMath/PrimitivePolygonModelMaker.hpp"
#include "../3DMath/TCBSpline.hpp"
#include "../Graphics/MeshModel/General3DMesh.hpp"

using namespace std;


namespace firearm
{


static float CalculateNormalAngle( const vector< pair<float,float> >& diameter_and_height, int i )
{
	if( diameter_and_height.size() == 2
		&& fabs(diameter_and_height[0].first - diameter_and_height[1].first) < 0.000001 )
	{
		// a vertical wall
		return 0;
	}

	const float radius = diameter_and_height[i].first * 0.5f;
	const float height = diameter_and_height[i].second;

	if( i==0 )
	{
		float next_radius = diameter_and_height[1].first * 0.5f;
		float next_height = diameter_and_height[1].second;
		return atan( (next_radius - radius) / (next_height-height) );
	}
	else if( i==diameter_and_height.size()-1 )
	{
		float prev_radius = diameter_and_height[i-1].first * 0.5f;
		float prev_height = diameter_and_height[i-1].second;
		return atan( (radius - prev_radius) / (height-prev_height) );
	}
	else
	{
		// Has 3 or more segments
		float prev_radius = diameter_and_height[i-1].first * 0.5f;
		float prev_height = diameter_and_height[i-1].second;
		float next_radius = diameter_and_height[i+1].first * 0.5f;
		float next_height = diameter_and_height[i+1].second;
		float lower_normal_angle = atan( (radius - prev_radius) / (height-prev_height) );
		float upper_normal_angle = atan( (next_radius - radius) / (next_height-height) );
		return (lower_normal_angle + upper_normal_angle) * 0.5f;
	}
}


// The code create a cylinderical segments are somewhat similar to the one in CreateCylinder()
// The rationale behind writing a sepaate code is that
// - The cartridge maker needs multiple segments with different radii.
//   - Note that when radii are scaled later, the normals need to be recalculated as well.
// - In order to create UV texture coords, the cartridge maker needs cylinder segments whose vertices are duplicated at one vertex.
//   - CreateCylinder() can support this with some modifications, but the recalculations at duplicated vertices would be tricky.
// - The vertices are addd from bottom to top
void CartridgeMaker::AddSegments(
                     const vector< pair<float,float> >& diameter_and_height,
                     int num_sides,
                     bool create_top_polygons,
                     //PrimitiveModelStyle::Name top_style,
                     bool create_bottom_polygons,
                     //PrimitiveModelStyle::Name bottom_style,
                     vector<Vector3>& vecDestPos,
                     vector<Vector3>& vecDestNormal,
					 vector< vector<int> >& vecDestPoly )
{
	Vector3 vUp = Vector3(0,1,0);//vDirFromBottomToTop = Vec3GetNormalized( vCore );
//	vector<Vector3> vecNormal;

	if( diameter_and_height.size() < 2 )
		return;

	const int num_divisions = diameter_and_height.size() - 1;

	if( vecDestPos.size() != vecDestNormal.size() )
		return;

	const int vertex_index_offset = (int)vecDestPos.size();

	bool is_normal_const = false;
	Vector3 const_normal( Vector3(0,0,0) );
	if( diameter_and_height.size() == 2
	 && fabs(diameter_and_height[1].second - diameter_and_height[0].second) < 0.000001 )
	{
		is_normal_const = true;
		float radius_diff = diameter_and_height[1].first - diameter_and_height[0].first;
		if( radius_diff <= 0 )
			const_normal = Vector3(0,1,0);
		else
			const_normal = Vector3(0,-1,0);
	}

	// create vertices
	for( int i=0; i<num_divisions+1; i++ )
	{
		const float radius = diameter_and_height[i].first * 0.5f;
		const float height = diameter_and_height[i].second;
//		const Matrix34 pose = Matrix34( -vUp * height * ( (float)i / (float)num_divisions - 0.5f ), Matrix33Identity() );

		const float normal_angle = is_normal_const ? 0 : CalculateNormalAngle( diameter_and_height, i );

		for( int j=0; j<num_sides+1; j++ )
		{
			const float angle = (float)(j%num_sides) / (float)num_sides * 2.0f * (float)PI;
			const Vector2 vDir = Matrix22Rotation( angle ) * Vector2(1,0);
			Vector2 vLocalPos = vDir * radius;
//			Vector3 vPos = pose * Vector3( vLocalPos.x, 0, vLocalPos.y );
			Vector3 vPos = Vector3( vLocalPos.x, height, vLocalPos.y );

//			LOG_PRINT( "cylinder vertex: " + to_string( vPos ) );

			vecDestPos.push_back( vPos );

			Vector3 normal
				= is_normal_const ?
				const_normal // vertical up/down
				: Matrix33RotationY(angle) * Matrix33RotationZ(normal_angle) * Vector3(1,0,0);

//			vecNormal.push_back( Vec3GetNormalized( vPos - pose.vPosition ) );
//			vecNormal.push_back( Vec3GetNormalized( Vector3( vDir.x, 0, vDir.y ) ) );
			vecDestNormal.push_back( normal );
		}
	}

	int top_center_vertex_index    = 0;
	int bottom_center_vertex_index = 0;
//	Vector3 top_center    = vUp * diameter_and_height.back().second;
	Vector3 bottom_center = Vector3(0,0,0);//vUp * diameter_and_height.front().second;

	if( create_bottom_polygons )
	{
		// Bottom polygons are added before all the rest of the wall polygons are created,
		// so they are inserted to the front.
		bottom_center_vertex_index = 0;
//		vecDestPos.push_back( bottom_center );
//		if( bottom_style == PrimitiveModelStyle::EDGE_VERTICES_UNWELDED )
		bool weld_bottom_rim_vertices = false;
		if( weld_bottom_rim_vertices )
		{
		}
		else
		{
			// Duplicate the points on the rims (bottom)
			vector<Vector3> bottom_vertices;
			bottom_vertices.reserve( num_sides + 1 ); // rims + center
			bottom_vertices.push_back( bottom_center ); // center vertex
			bottom_vertices.insert( bottom_vertices.end(), vecDestPos.begin(), vecDestPos.begin() + num_sides ); // rim vertices
//			vector<Vector3>::iterator start = vecDestPos.begin() + num_sides * num_divisions;
//			vecDestPos.insert( vecDestPos.end(), start, start + num_sides );
			vecDestPos.insert( vecDestPos.begin(), bottom_vertices.begin(), bottom_vertices.end() ); // insert to the head of the destination vector
			// Copy normals as well (all the normals points down)
			Vector3 bottom_normal( Vector3(0,-1,0) );
			vecDestNormal.insert( vecDestNormal.begin(), num_sides + 1, bottom_normal );
		}
	}

	if( create_top_polygons )
	{
		top_center_vertex_index = (int)vecDestPos.size();

		bool weld_top_rim_vertices = false;
		if( weld_top_rim_vertices )
		{
		}
		else
		{
			Vector3 top_center = Vector3( 0, vecDestPos.back().y, 0 );
			// Duplicate the points on the rims (top)
			vector<Vector3> top_vertices;
			top_vertices.reserve( num_sides + 1 ); // rims + center
			top_vertices.push_back( top_center ); // center vertex
			top_vertices.insert( top_vertices.end(), vecDestPos.end() - num_sides - 1, vecDestPos.end() - 1 ); // rim vertices
			vecDestPos.insert( vecDestPos.end(), top_vertices.begin(), top_vertices.end() ); // append to the destination vector
			Vector3 top_normal( Vector3(0,1,0) );
			vecDestNormal.insert( vecDestNormal.end(), num_sides + 1, top_normal );
		}
//		top_center_vertex_index    = vecDestPos.size();
//		vecDestPos.push_back( top_center );
//		if( top_style == PrimitiveModelStyle::EDGE_VERTICES_UNWELDED )
//		{
//			// Duplicate the points on the rims (top)
//			// Note that the duplicated vertex position is copied
//			vecDestPos.insert( vecDestPos.end(), vecDestPos.begin(), vecDestPos.begin() + num_sides );
//		}
////		else // i.e. style == PrimitiveModelStyle::EDGE_VERTICES_WELDED
//			// No need to duplicate points on the top and bottom rims
	}

	// Add normals

	// side vertices
//	vector<Vector3> rim_normals;
//	rim_normals.resize( num_sides );
//	for( int i=0; i<num_sides; i++ )
//	{
//		Vector3 pos = vecDestPos[i];
//		rim_normals[i] = Vec3GetNormalized( Vector3( pos.x, 0, pos.z ) );
//	}
//
//	vecDestNormal.resize( 0 );
//	for( int i=0; i<num_divisions+1; i++ )
//		vecDestNormal.insert( vecDestNormal.end(), rim_normals.begin(), rim_normals.end() );

//	if( create_top_polygons )
//	{
//		if( top_style == PrimitiveModelStyle::EDGE_VERTICES_UNWELDED )
//			vecDestNormal.insert( vecDestNormal.end(), num_sides+1,  vUp ); // top
//		else
//			vecDestNormal.push_back(  vUp );
//	}
//
//	if( create_bottom_polygons )
//	{
//		if( bottom_style == PrimitiveModelStyle::EDGE_VERTICES_UNWELDED )
//			vecDestNormal.insert( vecDestNormal.end(), num_sides+1, -vUp ); // bottom
//		else
//			vecDestNormal.push_back( -vUp );
//	}

	// create polygon indices

	const int num_polygons
		= num_divisions * num_sides
		+ (create_top_polygons    ? num_sides : 0)
		+ (create_bottom_polygons ? num_sides : 0);

	vecDestPoly.reserve( vecDestPoly.size() + num_polygons );

	// side (quads)
	int side_vertex_index_offset = vertex_index_offset;
	if( create_bottom_polygons )
		side_vertex_index_offset += (num_sides + 1);

	for( int i=0; i<num_divisions; i++ )
	{
		// polygons on the enclosing side of the cylinder
		int lower_start = (num_sides+1) * i;
		int upper_start = (num_sides+1) * (i+1);
		for( int j=0; j<num_sides; j++ )
		{
			vecDestPoly.push_back( vector<int>() );
			vecDestPoly.back().resize( 4 );
			vecDestPoly.back()[0] = side_vertex_index_offset + upper_start  + j;
			vecDestPoly.back()[1] = side_vertex_index_offset + upper_start  + j + 1;
			vecDestPoly.back()[2] = side_vertex_index_offset + lower_start  + j + 1;
			vecDestPoly.back()[3] = side_vertex_index_offset + lower_start  + j;
		}
	}

	// top (triangles)
	if( create_top_polygons )
	{
		int global_rim_vertex_offset  = (int)vecDestPos.size() - num_sides;//(top_style == PrimitiveModelStyle::EDGE_VERTICES_UNWELDED) ? top_center_vertex_index + 1 : 0;
		for( int i=0; i<num_sides; i++ )
		{
			vecDestPoly.push_back( vector<int>() );
			vecDestPoly.back().resize( 3 );
			vecDestPoly.back()[0] = top_center_vertex_index;
			vecDestPoly.back()[1] = global_rim_vertex_offset + (i+1) % num_sides;
			vecDestPoly.back()[2] = global_rim_vertex_offset + i;
		}
	}

	// bottom (triangles)
	if( create_bottom_polygons )
	{
		int rim_vertex_offset  = 1;//(bottom_style == PrimitiveModelStyle::EDGE_VERTICES_UNWELDED) ? bottom_center_vertex_index + 1 : num_segments * num_divisions;
		for( int i=0; i<num_sides; i++ )
		{
			vecDestPoly.push_back( vector<int>() );
			vecDestPoly.back().resize( 3 );
			vecDestPoly.back()[0] = vertex_index_offset + bottom_center_vertex_index;
			vecDestPoly.back()[1] = vertex_index_offset + rim_vertex_offset + i;
			vecDestPoly.back()[2] = vertex_index_offset + rim_vertex_offset + (i+1) % num_sides;
		}
	}
}


Result::Name CartridgeMaker::MakeBullet(
	const BulletDesc& bullet_desc,
	unsigned int num_sides,
	float case_top_height,
	vector<Vector3>& points,
	vector<Vector3>& normals,
	vector< vector<int> >& polygons
	)
{
//	if( bullet_desc.length < 0.001f
//	 || bullet_desc.diameter < 0.001f )
//	{
//		return Result::INVALID_ARGS;
//	}

	int num_segments = 0;
	int i =0;
	while( i<BulletDesc::NUM_MAX_BULLET_CURVE_PIONTS )
	{
		if( bullet_desc.bullet_curve_points[i].diameter < 0.001f )
			break;
		else
			i++;
	}

//	float case_top_height = 0.05f;

	vector< pair<float,float> > d_and_h_pairs;

	const int num_control_points = (bullet_desc.num_control_points < BulletDesc::NUM_MAX_BULLET_SLICES) ?
		bullet_desc.num_control_points : BulletDesc::NUM_MAX_BULLET_SLICES;

	const float bullet_length
		= bullet_desc.bullet_slice_control_points[num_control_points].position.y
		- bullet_desc.bullet_slice_control_points[0].position.y;
	float height_offset = case_top_height - (bullet_length - bullet_desc.exposed_length);
	for( int i=0; i<num_control_points-1; i++ )
	{
//		if( bullet_desc.create_model_only_for_exposed_part
//		 &&  )
//		{
//		}

		const BulletSliceControlPoint& prev_cp = (i==0) ? bullet_desc.bullet_slice_control_points[0] : bullet_desc.bullet_slice_control_points[i-1];
		const BulletSliceControlPoint& cp      = bullet_desc.bullet_slice_control_points[i];
		const BulletSliceControlPoint& next_cp = bullet_desc.bullet_slice_control_points[i+1];
		const BulletSliceControlPoint& cp_after_next = (i<num_control_points-2) ? bullet_desc.bullet_slice_control_points[i+2] : bullet_desc.bullet_slice_control_points[num_control_points-1];
//		if( cp.position == Vector2(0,0) )
//			break;
		int num_sub_segs_per_segment = 4;
		for( int j=0; j<num_sub_segs_per_segment; j++ )
		{
			const Vector2& p0 = prev_cp.position;
			const Vector2& p1 = cp.position;
			const Vector2& p2 = next_cp.position;
			const Vector2& p3 = cp_after_next.position;
			float frac = (float)j / (float)num_sub_segs_per_segment;
			Vector2 position = InterpolateWithTCBSpline(
				frac, p0, p1, p2, p3, cp.tension, cp.continuity, cp.bias );

			d_and_h_pairs.push_back( pair<float,float>( position.x * 2.0f, height_offset + position.y ) );
		}
	}

	AddSegments( d_and_h_pairs,
		num_sides,
		true,
		false,
		points,
		normals,
		polygons
		);

	return Result::SUCCESS;
}


pair<float,float> CalculateInetrpolatedDiameterAndHeight( const pair<float,float> *src, int i0, int i1, float dist_from_i1 )
{
	float diameter_diff = (src[i1].first  - src[i0].first);
	float radius_diff = diameter_diff * 0.5f;
	float length = (src[i1].second - src[i0].second);
	float long_edge = sqrt( radius_diff*radius_diff + length*length );

	float f = dist_from_i1 / long_edge;
	float radius = src[i1].first + radius_diff * 0.5f * f;
	float height = src[i1].second - length * f;

	return pair<float,float>( radius * 2.0f, height );
}


Result::Name CartridgeMaker::MakeCase( const CaseDesc& src_desc, unsigned int num_sides, std::vector<Vector3>& points, std::vector<Vector3>& normals, vector< vector<int> >& polygons )
{
//	vector<Vector3> points, normals;
//	vector< vector<int> > polygons;

	float radii[2] = { 1.0f, 1.0f };
//	int num_sides = 12;

//	src_desc.case_segments;
	int current_segment_index = 0;
	vector< pair<float,float> > diameter_and_height_pairs;
	diameter_and_height_pairs.reserve( 6 );

//	float height = 0;
	while(1)
	{
		int num_segments = 1;
		int start_segment_index = current_segment_index;
		int end_segment_index = current_segment_index;

		if( start_segment_index == CaseDesc::MAX_NUM_CASE_SLICES - 1 )
			break;

		if( fabs(src_desc.case_slices[start_segment_index+1].diameter) < 0.000001
		 && fabs(src_desc.case_slices[start_segment_index+1].height)   < 0.000001 )
		{
			break;
		}

		diameter_and_height_pairs.resize( 0 );

		const CaseSlice& start_slice = src_desc.case_slices[start_segment_index];
		diameter_and_height_pairs.push_back( pair<float,float>( start_slice.diameter,    start_slice.height ) );

		for( current_segment_index = start_segment_index + 1;
			 current_segment_index < CaseDesc::MAX_NUM_CASE_SLICES - 1;
			 current_segment_index++ )
		{
			const CaseSlice& current_slice = src_desc.case_slices[current_segment_index];
			if( current_slice.curvature_radius == 0 )
			{
				num_segments += 1;
				end_segment_index = current_segment_index;
				diameter_and_height_pairs.push_back( pair<float,float>( current_slice.diameter, current_slice.height ) );
				break; // Exit the loop without incrementing current_segment_index 
			}
			else
			{
				// Create rounded as a continuous cylinder

				if( current_segment_index == 0 || current_segment_index == CaseDesc::MAX_NUM_CASE_SLICES - 1 )
					break;

				num_segments += 5;
//				const CaseSlice& prev_slice = src_desc.case_slices[current_segment_index-1];
//				const CaseSlice& next_slice = src_desc.case_slices[current_segment_index+1];
//				diameter_and_height_pairs.push_back( CalculateInetrpolatedDiameterAndHeight( src_desc.case_slices, current_segment_index-1, current_segment_index, 0 ) );
				diameter_and_height_pairs.push_back( pair<float,float>( current_slice.diameter, current_slice.height ) );
//				diameter_and_height_pairs.push_back( CalculateInetrpolatedDiameterAndHeight( src_desc.case_slices, current_segment_index-1, current_segment_index, 1 ) );
			}
		}
		
		int point_offset = (int)points.size();

		// Close the top if
		// - reached the bottom of the inside,
		// - or reached the top and the client code has specified to close the top.
		bool create_top_polygons
			= (end_segment_index == src_desc.num_case_slices-1)
			|| (end_segment_index == src_desc.top_outer_slice_index && src_desc.create_internal_polygons == false);

		bool create_bottom_polygons = (start_segment_index == 0) ? true : false;

		AddSegments(
			diameter_and_height_pairs,
			num_sides,
			create_top_polygons,
//			top_style,
			create_bottom_polygons,
//			bottom_style,
			points,
			normals,
			polygons
			);

		start_segment_index = end_segment_index;
	}

	return Result::SUCCESS;
}


Result::Name CartridgeMaker::Make(
		const CartridgeDesc& src_desc,
		unsigned int num_sides,
		std::vector<Vector3>& points,
		std::vector<Vector3>& normals,
		std::vector< std::vector<int> >& polygons
		)
{
	Result::Name case_res   = MakeCase( src_desc.case_desc, num_sides, points, normals, polygons );

	if( case_res != Result::SUCCESS )
		return case_res;

	Result::Name bullet_res = MakeBullet(
		src_desc.bullet_desc, num_sides, src_desc.case_desc.GetTopHeight(),
		points, normals, polygons
		);

	return bullet_res;
}


} // firearm