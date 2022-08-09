#ifndef _SBL_IMAGE_H_
#define _SBL_IMAGE_H_
#include <sbl/core/String.h>
#ifdef USE_OPENCV
	#include <opencv2/core.hpp>
#else
	#include <string.h> // for memcpy
#endif
#include <typeinfo>
namespace sbl {


// static assert; used here to make sure we don't use color methods on gray images or vice versa
#define assertStatic(condition) switch(0){case 0:case condition:;}


// debug mode image bound checking
#define IMGCHK assertDebug( inBounds( x, y ) )
#define IMGCHK_INTERP assertDebug( inBounds( x, y ) )


// note: we use BGR order for RGB images
#define R_CHANNEL 2
#define G_CHANNEL 1
#define B_CHANNEL 0


//-------------------------------------------
// IMAGE CLASS 
//-------------------------------------------


/// The Image class represents a single-channle (grayscale) or multi-channel (e.g. RGB) image.
template <typename T, int CHANNEL_COUNT > class Image {
public:

	/// basic constructor; does not initialize image
	Image( int width, int height ) { alloc( width, height ); }

	// basic copy constructor
	Image( const Image &img ) {	alloc( img.width(), img.height() );	memcpy( m_raw, img.rawConst(), m_rowBytes * m_height ); }

	// basic destructor
	~Image();

	/// get/set pixel values for gray images (assumes CHANNEL_COUNT == 1)
	inline T &data( int x, int y ) { assertStatic( CHANNEL_COUNT == 1 ); IMGCHK return m_ptr[ y ][ x ]; }
	inline const T &data( int x, int y ) const { assertStatic( CHANNEL_COUNT == 1 ); IMGCHK return m_ptr[ y ][ x ]; }
	float interp( float x, float y ) const;

	/// get/set pixel values for multi-channel images
	inline T &data( int x, int y, int c ) { IMGCHK return m_ptr[ y ][ x * CHANNEL_COUNT + c ]; }
	inline const T &data( int x, int y, int c ) const { IMGCHK return m_ptr[ y ][ x * CHANNEL_COUNT + c ]; }
	float interp( float x, float y, int c ) const;

	/// get/set pixel values for RGB (actually BGR) color images (assumes CHANNEL_COUNT >= 3)
	inline T b( int x, int y ) const { assertStatic( CHANNEL_COUNT >= 3 ); IMGCHK return m_ptr[ y ][ x * CHANNEL_COUNT + B_CHANNEL ]; }
	inline T g( int x, int y ) const { assertStatic( CHANNEL_COUNT >= 3 ); IMGCHK return m_ptr[ y ][ x * CHANNEL_COUNT + G_CHANNEL ]; }
	inline T r( int x, int y ) const { assertStatic( CHANNEL_COUNT >= 3 ); IMGCHK return m_ptr[ y ][ x * CHANNEL_COUNT + R_CHANNEL ]; }
	inline void setRGB( int x, int y, T r, T g, T b ) { assertStatic( CHANNEL_COUNT >= 3 ); IMGCHK m_ptr[ y ][ x * CHANNEL_COUNT + B_CHANNEL ] = b; m_ptr[ y ][ x * CHANNEL_COUNT + G_CHANNEL ] = g; m_ptr[ y ][ x * CHANNEL_COUNT + R_CHANNEL ] = r; }
	inline void setB( int x, int y, T v ) { assertStatic( CHANNEL_COUNT >= 3 ); IMGCHK m_ptr[ y ][ x * CHANNEL_COUNT + B_CHANNEL ] = v; }
	inline void setG( int x, int y, T v ) { assertStatic( CHANNEL_COUNT >= 3 ); IMGCHK m_ptr[ y ][ x * CHANNEL_COUNT + G_CHANNEL ] = v; }
	inline void setR( int x, int y, T v ) { assertStatic( CHANNEL_COUNT >= 3 ); IMGCHK m_ptr[ y ][ x * CHANNEL_COUNT + R_CHANNEL ] = v; }

	// get image info
	inline int width() const { return m_width; }
	inline int height() const { return m_height; }
	inline int rowBytes() const { return m_rowBytes; }
	inline int memUsed() const { return sizeof( Image<T,CHANNEL_COUNT> ) + m_rowBytes * m_height; }
	inline int depth() const { return sizeof(T) * 8; }
	inline int channelCount() const { return CHANNEL_COUNT; }
	inline bool isFloat() const { return typeid( T ) == typeid( float ); }

	/// check whether position is in image bounds
	inline bool inBounds( int x, int y ) const { return x >= 0 && x < m_width && y >= 0 && y < m_height; }
	inline bool inBounds( float x, float y ) const { return x >= 0 && x < m_width - 1 && y >= 0 && y < m_height - 1; }

	/// access image data via pointers
	inline T *raw() { return m_raw; }
	inline const T *rawConst() const { return m_raw; }
	inline T **pointers() { return m_ptr; }

	/// clear the image to the specified color
	void clear( T r, T g, T b );
	void clear( T v );

private:

	// common constructor code
	void alloc( int width, int height );

	// image data (top origin)
	T *m_raw;
	T **m_ptr;
	int m_width;
	int m_height;
	int m_rowBytes;

	// indicates whether to delete m_raw
	bool m_deleteRaw;

	// disable assignment operator
	Image &operator=( const Image &img );

// the rest of the class is only used for OpenCV interaction 
#ifdef USE_OPENCV
public:

	/// create an Image object wrapping a cv::Mat object
	Image(cv::Mat &mat);

	/// return cv::Mat object; create object if needed
	inline cv::Mat& cvMat() {
		if (m_cvMat.empty()) {
			createCvMat();
		}
		return m_cvMat;
	}
	inline const cv::Mat& cvMat() const {
		if (m_cvMat.empty()) {
			createCvMat();
		}
		return m_cvMat;
	}

private:

	/// create cv::Mat object if needed (m_cvMat is mutable so this func can be const)
	void createCvMat() const;

	/// the stored cv::Mat, if any
	mutable cv::Mat m_cvMat;

#endif // USE_OPENCV
};


//-------------------------------------------
// IMAGE CLASS IMPLEMENTATION 
//-------------------------------------------


// deallocate image data
template<typename T, int CHANNEL_COUNT> Image<T, CHANNEL_COUNT>::~Image() {
	if (m_deleteRaw)
		delete [] m_raw;
	delete [] m_ptr;
}


// common constructor code
template<typename T, int CHANNEL_COUNT> void Image<T, CHANNEL_COUNT>::alloc( int width, int height ) {
	m_width = width;
	m_height = height;
	m_rowBytes = m_width * sizeof(T) * CHANNEL_COUNT;
	int rowWidth = m_rowBytes / sizeof( T );
	assertDebug( rowWidth >= m_width );
	int size = rowWidth * m_height; // total number of elements

	// alloc pixel data
	m_raw = new T[ size ];
	if (m_raw == NULL) fatalError( "error allocating Image data" );
	m_deleteRaw = true;

	// check alignment
	if (((long long int) m_raw) & 7)
		fatalError("not quad-word aligned");

	// create row pointers
	m_ptr = new T*[ m_height ];
	if (m_ptr == NULL) fatalError( "error allocating Image pointers" );
	for (int i = 0; i < m_height; i++)
		m_ptr[ i ] = m_raw + i * rowWidth;
}


/// clear the image to the specified color
template<typename T, int CHANNEL_COUNT> void Image<T, CHANNEL_COUNT>::clear( T r, T g, T b ) {
	for (int y = 0; y < m_height; y++) {
		for (int x = 0; x < m_width; x++) {
			setRGB( x, y, r, g, b );
		}
	}
}


/// clear the image to the specified color
template<typename T, int CHANNEL_COUNT> void Image<T, CHANNEL_COUNT>::clear( T v ) {
	for (int y = 0; y < m_height; y++) {
		for (int x = 0; x < m_width; x++) {
			m_ptr[ y ][ x ] = v;
		}
	}
}


/// performs bilinear interpolation at specified point
template<typename T, int CHANNEL_COUNT> float Image<T, CHANNEL_COUNT>::interp( float x, float y ) const {
	IMGCHK_INTERP
    int xInt = (int) x; 
    int yInt = (int) y; 
	float val = 0;
	assertDebug( xInt >= 0 && yInt >= 0 && xInt < m_width && yInt < m_height );
	if (xInt == m_width - 1) { // fix(faster): make faster?
		if (yInt == m_height - 1) {
			val = (float) m_ptr[ yInt ][ xInt ];
		} else {
			float yFrac = y - yInt;
			val = (1.0f - yFrac) * m_ptr[ yInt     ][ xInt ] + 
						  yFrac  * m_ptr[ yInt + 1 ][ xInt ];
		}
	} else if (yInt == m_height - 1) {
		float xFrac = x - xInt;
		val = (1.0f - xFrac) * m_ptr[ yInt ][ xInt     ] + 
					  xFrac  * m_ptr[ yInt ][ xInt + 1 ];
	} else {
		float xFrac = x - xInt;
		float yFrac = y - yInt;
		val = (1.0f - xFrac) * (1.0f - yFrac) * m_ptr[ yInt     ][ xInt     ] + 
			  (1.0f - xFrac) *         yFrac  * m_ptr[ yInt + 1 ][ xInt     ] + 
					  xFrac  * (1.0f - yFrac) * m_ptr[ yInt     ][ xInt + 1 ] + 
					  xFrac  *         yFrac  * m_ptr[ yInt + 1 ][ xInt + 1 ];
	}
	return val;
}


/// performs bilinear interpolation at specified point
template<typename T, int CHANNEL_COUNT> float Image<T, CHANNEL_COUNT>::interp( float x, float y, int c ) const {
	IMGCHK_INTERP
    int xInt = (int) x; 
    int yInt = (int) y; 
    float xFrac = x - xInt;
    float yFrac = y - yInt;
    float val = (1.0f - xFrac) * (1.0f - yFrac) * m_ptr[ yInt     ][ (xInt    ) * CHANNEL_COUNT + c ] + 
  	            (1.0f - xFrac) *         yFrac  * m_ptr[ yInt + 1 ][ (xInt    ) * CHANNEL_COUNT + c ] + 
		                xFrac  * (1.0f - yFrac) * m_ptr[ yInt     ][ (xInt + 1) * CHANNEL_COUNT + c ] + 
	                    xFrac  *         yFrac  * m_ptr[ yInt + 1 ][ (xInt + 1) * CHANNEL_COUNT + c ];
	return val;
}


//-------------------------------------------
// IMAGE CLASS IMPLEMENTATION - WITH OPENCV
//-------------------------------------------
#ifdef USE_OPENCV


/// create an Image object wrapping a CvMat object
template<typename T, int CHANNEL_COUNT> Image<T, CHANNEL_COUNT>::Image(cv::Mat &mat) {
	assertAlways(mat.channels() == CHANNEL_COUNT);
	m_width = mat.cols;
	m_height = mat.rows;
	int bytesPerElement = 0;
	if (mat.depth() == CV_8U) {  // can we use mat.elemSize instead?
		bytesPerElement = 1;
	} else {
		fatalError("depth not supported");
	}
	int rowWidth = mat.cols * bytesPerElement * mat.channels();  // this won't work if there is any per-row padding
	m_rowBytes = rowWidth * sizeof(T);
	m_raw = (T *) mat.data;
	m_cvMat = mat;
	m_deleteRaw = false;

	// create row pointers
	m_ptr = new T*[ m_height ];
	if (m_ptr == NULL) fatalError( "error allocating Image pointers" );
	for (int i = 0; i < m_height; i++)
		m_ptr[ i ] = m_raw + i * rowWidth;
}


/// create cv::Mat object if needed
template<typename T, int CHANNEL_COUNT> void Image<T, CHANNEL_COUNT>::createCvMat() const {
    assert( m_cvMat.empty() );

	// determine parameters of cv::Mat constructor
	cv::Size size = cv::Size(m_width, m_height);
	int type = CV_MAKETYPE(cv::DataType<T>::type, CHANNEL_COUNT);

	// create cv::Mat object
	m_cvMat = cv::Mat(size, type, m_raw);
}


#endif // USE_OPENCV


//-------------------------------------------
// IMAGE TYPEDEFS 
//-------------------------------------------


/// common image types
typedef Image<unsigned char, 3> ImageColorU;
typedef Image<unsigned short, 3> ImageColorS;
typedef Image<int, 3> ImageColorI;
typedef Image<float, 3> ImageColorF;
typedef Image<unsigned char, 1> ImageGrayU;
typedef Image<unsigned short, 1> ImageGrayS;
typedef Image<int, 1> ImageGrayI;
typedef Image<float, 1> ImageGrayF;


} // end namespace sbl
#endif // _SBL_IMAGE_H_
