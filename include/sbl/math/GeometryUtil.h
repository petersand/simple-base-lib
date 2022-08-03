#ifndef _SBL_GEOMETRY_UTIL_H_
#define _SBL_GEOMETRY_UTIL_H_
#include <sbl/math/Geometry.h>
namespace sbl {


/*! \file GeometryUtil.h
	\brief The GeometryUtil module provides functions that operate on classes
	in the Geometry module.
*/


// converts a 3-demensional point's representation from doubles to floats
// use carefully as this will reduce precision
inline Point3F point3DtoF(Point3D point) { return Point3F((float)point.x, (float)point.y, (float)point.z); }


// converts a 3-demensional point's representation from floats to doubles
inline Point3D point3FtoD(Point3F point) { return Point3D((double)point.x, (double)point.y, (double)point.z); }


} // end namespace sbl
#endif // _SBL_GEOMETRY_UTIL_H_
