#ifndef _SBL_GEOMETRY_H_
#define _SBL_GEOMETRY_H_
#include <sbl/core/File.h>
#include <sbl/math/Vector.h>
namespace sbl {


//-------------------------------------------
// POINT 2 CLASS 
//-------------------------------------------


/// The Point2 class represents a 2D Euclidean point.
template<typename T> class _Point2 {
public:

	// note: this class uses default copy constructor and assignment operator

    /// create a point
    _Point2(T xIn, T yIn) : x( xIn ), y( yIn ) {}
	_Point2() : x(0), y(0) {}  // need default constructor to make vectors of points

    /// coordinates
    T x;
    T y;

	// operators

	_Point2 operator+=(_Point2 const& p) {
		x += p.x;
		y += p.y;
		return *this;
	}

	_Point2 operator-=(_Point2 const& p) {
		x -= p.x;
		y -= p.y;
		return *this;
	}

	template <typename S> _Point2 operator*=(S const& scalar) {
		x *= scalar;
		y *= scalar;
		return *this;
	}

	// returns true if all elements are zero
	bool isZero() const {
		return (x == 0) && (y == 0);
	}
};

// rest of operators implemented as non-member functions
// ref: https://stackoverflow.com/questions/14482380/multiplying-an-object-with-a-constant-from-left-side

template <typename T> inline _Point2<T> operator+(_Point2<T> p1, _Point2<T> const& p2) { return (p1 += p2); };
template <typename T> inline _Point2<T> operator-(_Point2<T> p1, _Point2<T> const& p2) { return (p1 -= p2); };
template <typename T, typename S> inline _Point2<T> operator*(S const& scalar, _Point2<T> p) { return (p *= scalar); };
template <typename T, typename S> inline _Point2<T> operator*(_Point2<T> p, S const& scalar) { return (p *= scalar); };


typedef _Point2<unsigned char> Point2U;
typedef _Point2<int> Point2I;
typedef _Point2<float> Point2F;
typedef _Point2<double> Point2D;
typedef _Point2<double> Point2;  // TODO: remove this and rename _Point2 to Point2 after other code is migrated


//-------------------------------------------
// POINT 3 CLASS 
//-------------------------------------------


/// The Point3 class represents a 3D Euclidean point.
template <typename T> class _Point3 {
public:

	// note: this class uses default copy constructor and assignment operator

	/// create a point
    _Point3(T xIn, T yIn, T zIn) : x(xIn), y(yIn), z(zIn) {}
    _Point3() : x(0), y(0), z(0) {}

    /// coordinates
    T x;
    T y;
    T z;

	// operators

	_Point3 operator+=(_Point3 const& p) {
		x += p.x;
		y += p.y;
		z += p.z;
		return *this;
	}

	_Point3 operator-=(_Point3 const& p) {
		x -= p.x;
		y -= p.y;
		z -= p.z;
		return *this;
	}

	template <typename S> _Point3 operator*=(S const& scalar) {
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	// conversion function
	_Point2<T> toXY() const {
		return _Point2<T>(x, y);
	}

	// returns true if all elements are zero
	bool isZero() const {
		return (x == 0) && (y == 0) && (z == 0);
	}
};

// rest of operators implemented as non-member functions
// ref: https://stackoverflow.com/questions/14482380/multiplying-an-object-with-a-constant-from-left-side

template <typename T> inline _Point3<T> operator+(_Point3<T> p1, _Point3<T> const& p2) { return (p1 += p2); };
template <typename T> inline _Point3<T> operator-(_Point3<T> p1, _Point3<T> const& p2) { return (p1 -= p2); };
template <typename T, typename S> inline _Point3<T> operator*(S const& scalar, _Point3<T> p) { return (p *= scalar); };
template <typename T, typename S> inline _Point3<T> operator*(_Point3<T> p, S const& scalar) { return (p *= scalar); };

// converting 2D to 3D

template <typename T> inline _Point3<T> xy2xyz(_Point2<T> const& p, T z=0) { return _Point3<T>(p.x, p.y, z); }


typedef _Point3<unsigned char> Point3U;
typedef _Point3<int> Point3I;
typedef _Point3<float> Point3F;
typedef _Point3<double> Point3D;
typedef _Point3<double> Point3;  // TODO: remove this and rename _Point2 to Point2 after other code is migrated


//-------------------------------------------
// SEGMENT 2 CLASS 
//-------------------------------------------


/// The Segment2 class represents a 2D line between two 2D points
class Segment2 {
public:

	// note: this class uses default copy constructor and assignment operator

	/// create a segment between two points
	Segment2( Point2 &start, Point2 &end ) : m_start( start ), m_end( end ) {}

	/// the segment start point
	const Point2 &start() const { return m_start; }

	/// the segment end point
	const Point2 &end() const { return m_end; }

private:

	// the points
    Point2 m_start;
    Point2 m_end;
};


//-------------------------------------------
// SEGMENT 3 CLASS 
//-------------------------------------------


/// The Segment3 class represents a 3D line between two 3D points
class Segment3 {

	// note: this class uses default copy constructor and assignment operator

	/// create a segment between two points
	Segment3( Point3 &start, Point3 &end ) : m_start( start ), m_end( end ) {}

	/// the segment start point
	const Point3 &start() const { return m_start; }

	/// the segment end point
	const Point3 &end() const { return m_end; }

private:

	// the points
    Point3 m_start;
    Point3 m_end;
};


//-------------------------------------------
// MATRIX 3 CLASS 
//-------------------------------------------


/// The Matrix3 class represents a 3x3 matrix for geometric transformations.
class Matrix3 {
public:

	// note: this class uses default copy constructor and assignment operator

	/// create a zero-initialized matrix
	Matrix3();

	/// the matrix data
	double data[ 3 ][ 3 ];

	/// set diagonal elements
	inline void setDiag( double x, double y, double z ) { data[ 0 ][ 0 ] = x; data[ 1 ][ 1 ] = y; data[ 2 ][ 2 ] = z; }

	/// set off-diagonal elements to a given value
	void setOffDiag( double v );

	/// transform a point using the matrix
	Point3 operator*( const Point3 &p ) const;
};


//-------------------------------------------
// AFFINE TRANSFORM 3 CLASS
//-------------------------------------------


/// The AffineTransform3 class represents a 3D linear transformation.
class AffineTransform3 {
public:

	// note: this class uses default copy constructor and assignment operator

	/// create identity transform
	AffineTransform3() { a.data[ 0 ][ 0 ] = 1; a.data[ 1 ][ 1 ] = 1; a.data[ 2 ][ 2 ] = 1; }

	/// create transform from vector of parameters
	AffineTransform3( const VectorD &params ); 

	/// the translation offset
	Point3 b;

	/// the transformation matrix (rotating, scaling, etc.)
	Matrix3 a;

	/// set the translation offset
	inline void setTranslation( double x, double y, double z ) { b.x = x; b.y = y; b.z = z; }

	/// set the diagonal matrix elements
	inline void setDiag( double x, double y, double z ) { a.data[ 0 ][ 0 ] = x; a.data[ 1 ][ 1 ] = y; a.data[ 2 ][ 2 ] = z; }

	/// set the off-diagonal matrix elements
	inline void setOffDiag( double v ) { a.setOffDiag( v ); }

	/// set the matrix elements
	void setMatrix( double a00, double a01, double a02, double a10, double a11, double a12, double a20, double a21, double a22 );

	/// set rotation matrix
	void setRotation( double x, double y, double z );

	/// convert the transformation to a vector of parameter values
	VectorD toVector() const;

	/// apply the linear transformation to a point
	inline Point3 transform( const Point3 &x ) const { return b + a * x; }

	/// compute the inverse transformation
	AffineTransform3 inverse() const;
};


/// save 3D affine transformation parameters to text file
void saveTransform( File &file, AffineTransform3 &transform );


/// load 3D affine transformation parameters from text file
AffineTransform3 loadTransform( File &file );


} // end namespace sbl
#endif // _SBL_GEOMETRY_H_
