/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

// These are helper functions for the SDK samples (string parsing, timers, etc)
#ifndef SDK_HELPER_H
#define SDK_HELPER_H

#ifdef WIN32
#pragma warning(disable:4996)
#endif

// includes, project
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <assert.h>
#include <exception.h>
#include <math.h>

#include <fstream>
#include <vector>
#include <iostream>
#include <algorithm>

// includes, timer
#include <stopwatch_functions.h>
#include <string_helper.h>

#ifndef MIN
#define MIN(a,b) ((a < b) ? a : b)
#endif
#ifndef MAX
#define MAX(a,b) ((a > b) ? a : b)
#endif

// Beginning of GPU Architecture definitions
inline int _ConvertSMVer2Cores(int major, int minor)
{
    // Defines for GPU Architecture types (using the SM version to determine the # of cores per SM
    typedef struct {
       int SM; // 0xMm (hexidecimal notation), M = SM Major version, and m = SM minor version
       int Cores;
    } sSMtoCores;

    sSMtoCores nGpuArchCoresPerSM[] = 
    { { 0x10,  8 }, // Tesla Generation (SM 1.0) G80 class
      { 0x11,  8 }, // Tesla Generation (SM 1.1) G8x class
      { 0x12,  8 }, // Tesla Generation (SM 1.2) G9x class
      { 0x13,  8 }, // Tesla Generation (SM 1.3) GT200 class
      { 0x20, 32 }, // Fermi Generation (SM 2.0) GF100 class
      { 0x21, 48 }, // Fermi Generation (SM 2.1) GF10x class
      {   -1, -1 }
    };

    int index = 0;
    while (nGpuArchCoresPerSM[index].SM != -1) {
       if (nGpuArchCoresPerSM[index].SM == ((major << 4) + minor) ) {
          return nGpuArchCoresPerSM[index].Cores;
       }	
       index++;
    }
    printf("MapSMtoCores undefined SM %d.%d is undefined (please update to the latest SDK)!\n", major, minor);
    return -1;
}
// end of GPU Architecture definitions

// namespace unnamed (internal)
namespace 
{
    //! size of PGM file header 
    const unsigned int PGMHeaderSize = 0x40;

    // types

    //! Data converter from unsigned char / unsigned byte to type T
    template<class T>
    struct ConverterFromUByte;

    //! Data converter from unsigned char / unsigned byte 
    template<>
	struct ConverterFromUByte<unsigned char> 
	{
		//! Conversion operator
		//! @return converted value
		//! @param  val  value to convert
		float operator()( const unsigned char& val) 
		{
			return static_cast<unsigned char>(val);
		}
	};

    //! Data converter from unsigned char / unsigned byte to float
    template<>
	struct ConverterFromUByte<float> 
	{
		//! Conversion operator
		//! @return converted value
		//! @param  val  value to convert
		float operator()( const unsigned char& val) 
		{
			return static_cast<float>( val) / 255.0f;
		}
	};

    //! Data converter from unsigned char / unsigned byte to type T
    template<class T>
    struct ConverterToUByte;

    //! Data converter from unsigned char / unsigned byte to unsigned int
    template<>
    struct ConverterToUByte<unsigned char> 
    {
        //! Conversion operator (essentially a passthru
        //! @return converted value
        //! @param  val  value to convert
        unsigned char operator()( const unsigned char& val) 
        {
            return val;
        }
    };

    //! Data converter from unsigned char / unsigned byte to unsigned int
    template<>
    struct ConverterToUByte<float> 
    {
        //! Conversion operator
        //! @return converted value
        //! @param  val  value to convert
        unsigned char operator()( const float& val) 
        {
            return static_cast<unsigned char>( val * 255.0f);
        }
    };
}

#ifdef _WIN32
	#define FOPEN(fHandle,filename,mode) fopen_s(&fHandle, filename, mode)
	#define FOPEN_FAIL(result) (result != 0)
	#define SSCANF sscanf_s
#else
	#define FOPEN(fHandle,filename,mode) (fHandle = fopen(filename, mode))
	#define FOPEN_FAIL(result) (result == NULL)
	#define SSCANF sscanf
#endif

inline bool
__loadPPM( const char* file, unsigned char** data, 
         unsigned int *w, unsigned int *h, unsigned int *channels ) 
{
    FILE *fp = NULL;
    if( FOPEN_FAIL(FOPEN(fp, file, "rb")) ) 
    {
        std::cerr << "__LoadPPM() : Failed to open file: " << file << std::endl;
        return false;
    }

    // check header
    char header[PGMHeaderSize];
    if (fgets( header, PGMHeaderSize, fp) == NULL) {
       std::cerr << "__LoadPPM() : reading PGM header returned NULL" << std::endl;
       return false;
    }
    if (strncmp(header, "P5", 2) == 0)
    {
        *channels = 1;
    }
    else if (strncmp(header, "P6", 2) == 0)
    {
        *channels = 3;
    }
    else {
        std::cerr << "__LoadPPM() : File is not a PPM or PGM image" << std::endl;
        *channels = 0;
        return false;
    }

    // parse header, read maxval, width and height
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned int maxval = 0;
    unsigned int i = 0;
    while(i < 3) 
    {
        if (fgets(header, PGMHeaderSize, fp) == NULL) {
            std::cerr << "__LoadPPM() : reading PGM header returned NULL" << std::endl;
            return false;
        }
        if(header[0] == '#') 
            continue;

        if(i == 0) 
        {
            i += SSCANF( header, "%u %u %u", &width, &height, &maxval);
        }
        else if (i == 1) 
        {
            i += SSCANF( header, "%u %u", &height, &maxval);
        }
        else if (i == 2) 
        {
            i += SSCANF(header, "%u", &maxval);
        }
    }

    // check if given handle for the data is initialized
    if( NULL != *data) 
    {
        if (*w != width || *h != height) 
        {
            std::cerr << "__LoadPPM() : Invalid image dimensions." << std::endl;
        }
    } 
    else 
    {
        *data = (unsigned char*) malloc( sizeof( unsigned char) * width * height * *channels);
        *w = width;
        *h = height;
    }

    // read and close file
    if (fread( *data, sizeof(unsigned char), width * height * *channels, fp) == 0) {
        std::cerr << "__LoadPPM() read data returned error." << std::endl;
    }
    fclose(fp);

    return true;
}

template <class T>
inline bool
sdkLoadPGM( const char* file, T** data, unsigned int *w, unsigned int *h) 
{
    unsigned char* idata = NULL;
    unsigned int channels;
    if( true != __loadPPM(file, &idata, w, h, &channels)) 
    {
        return false;
    }

    unsigned int size = *w * *h * channels;

    // initialize mem if necessary
    // the correct size is checked / set in loadPGMc()
    if( NULL == *data) 
    {
        *data = (T*) malloc( sizeof(T) * size );
    }

    // copy and cast data
    std::transform( idata, idata + size, *data, ConverterFromUByte<T>());

    free( idata );

    return true;
}

template <class T>
inline bool
sdkLoadPPM4( const char* file, T** data, 
               unsigned int *w,unsigned int *h)
{
    unsigned char *idata = 0;
    unsigned int channels;
    
    if (__loadPPM( file, &idata, w, h, &channels)) {
        // pad 4th component
        int size = *w * *h;
        // keep the original pointer
        unsigned char* idata_orig = idata;
        *data = (T*) malloc( sizeof(T) * size * 4);
        unsigned char *ptr = *data;
        for(int i=0; i<size; i++) {
            *ptr++ = *idata++;
            *ptr++ = *idata++;
            *ptr++ = *idata++;
            *ptr++ = 0;
        }
        free( idata_orig);
        return true;
    }
    else
    {
        free ( idata);
        return false;
    }
}

inline bool
__savePPM( const char* file, unsigned char *data, 
         unsigned int w, unsigned int h, unsigned int channels) 
{
    assert( NULL != data);
    assert( w > 0);
    assert( h > 0);

    std::fstream fh( file, std::fstream::out | std::fstream::binary );
    if( fh.bad()) 
    {
        std::cerr << "__savePPM() : Opening file failed." << std::endl;
        return false;
    }

    if (channels == 1)
    {
        fh << "P5\n";
    }
    else if (channels == 3) {
        fh << "P6\n";
    }
    else {
        std::cerr << "__savePPM() : Invalid number of channels." << std::endl;
        return false;
    }

    fh << w << "\n" << h << "\n" << 0xff << std::endl;

    for( unsigned int i = 0; (i < (w*h*channels)) && fh.good(); ++i) 
    {
        fh << data[i];
    }
    fh.flush();

    if( fh.bad()) 
    {
        std::cerr << "__savePPM() : Writing data failed." << std::endl;
        return false;
    } 
    fh.close();

    return true;
}

template<class T>
inline bool
sdkSavePGM( const char* file, T *data, unsigned int w, unsigned int h) 
{
    unsigned int size = w * h;
    unsigned char* idata = 
      (unsigned char*) malloc( sizeof(unsigned char) * size);

    std::transform( data, data + size, idata, ConverterToUByte<T>());

    // write file
    bool result = __savePPM(file, idata, w, h, 1);

    // cleanup
    free( idata );

    return result;
}

inline bool
sdkSavePPM4ub( const char* file, unsigned char *data, 
              unsigned int w, unsigned int h) 
{
    // strip 4th component
    int size = w * h;
    unsigned char *ndata = (unsigned char*) malloc( sizeof(unsigned char) * size*3);
    unsigned char *ptr = ndata;
    for(int i=0; i<size; i++) {
        *ptr++ = *data++;
        *ptr++ = *data++;
        *ptr++ = *data++;
        data++;
    }

	bool result = __savePPM( file, ndata, w, h, 3);
	free (ndata);
	return result;
}


//////////////////////////////////////////////////////////////////////////////
//! Write a data file \filename 
//! @return true if writing the file succeeded, otherwise false
//! @param filename name of the source file
//! @param data  data to write
//! @param len  number of data elements in data, -1 on error
//! @param epsilon  epsilon for comparison
//////////////////////////////////////////////////////////////////////////////
template<class T, class S>
inline bool
sdkWriteFile( const char* filename, const T* data, unsigned int len,
              const S epsilon, bool verbose) 
{
    assert( NULL != filename);
    assert( NULL != data);

    // open file for writing
    std::fstream fh( filename, std::fstream::out);
    // check if filestream is valid
    if( ! fh.good()) 
    {
        if (verbose)
            std::cerr << "cutWriteFile() : Opening file failed." << std::endl;
        return false;
    }

    // first write epsilon
    fh << "# " << epsilon << "\n";

    // write data
    for( unsigned int i = 0; (i < len) && (fh.good()); ++i) 
    {
        fh << data[i] << ' ';
    }

    // Check if writing succeeded
    if( ! fh.good()) 
    {
        if (verbose)
            std::cerr << "cutWriteFile() : Writing file failed." << std::endl;
        return false;
    }

    // file ends with nl
    fh << std::endl;

    return true;
}

////////////////////////////////////////////////////////////////////////////// 
//! Compare two arrays of arbitrary type       
//! @return  true if \a reference and \a data are identical, otherwise false
//! @param reference  timer_interface to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
//! @param epsilon    epsilon to use for the comparison
//////////////////////////////////////////////////////////////////////////////
template<class T, class S>
inline bool
compareData( const T* reference, const T* data, const unsigned int len, 
             const S epsilon, const float threshold) 
{
    assert( epsilon >= 0);

    bool result = true;
    unsigned int error_count = 0;

    for( unsigned int i = 0; i < len; ++i) {
        float diff = (float)reference[i] - (float)data[i];
        bool comp = (diff <= epsilon) && (diff >= -epsilon);
        result &= comp;

        error_count += !comp;

#ifdef _DEBUG
        if( ! comp) 
        {
            std::cerr << "ERROR, i = " << i << ",\t " 
                << reference[i] << " / "
                << data[i] 
                << " (reference / data)\n";
        }
#endif
    }

    if (threshold == 0.0f) {
        return (result) ? true : false;
    } else {
        if (error_count) {
            printf("%4.2f(%%) of bytes mismatched (count=%d)\n", (float)error_count*100/(float)len, error_count);
        }
        return (len*threshold > error_count) ? true : false;
    }
}

#ifndef __MIN_EPSILON_ERROR
#define __MIN_EPSILON_ERROR 1e-3f
#endif

////////////////////////////////////////////////////////////////////////////// 
//! Compare two arrays of arbitrary type       
//! @return  true if \a reference and \a data are identical, otherwise false
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
//! @param epsilon    epsilon to use for the comparison
//! @param epsilon    threshold % of (# of bytes) for pass/fail
//////////////////////////////////////////////////////////////////////////////
template<class T, class S>
inline bool
compareDataAsFloatThreshold( const T* reference, const T* data, const unsigned int len, 
                             const S epsilon, const float threshold) 
{
    assert( epsilon >= 0);

    // If we set epsilon to be 0, let's set a minimum threshold
    float max_error = MAX( (float)epsilon, __MIN_EPSILON_ERROR );
    int error_count = 0;
    bool result = true;

    for( unsigned int i = 0; i < len; ++i) {
        float diff = fabs((float)reference[i] - (float)data[i]);
        bool comp = (diff < max_error);
        result &= comp;

        if( ! comp) 
        {
            error_count++;
#ifdef _DEBUG
            if (error_count < 50) {
               printf("\n    ERROR(epsilon=%4.3f), i=%d, (ref)0x%02x / (data)0x%02x / (diff)%d\n", 
	                max_error, i, 
                        *(unsigned int *)&reference[i],
                        *(unsigned int *)&data[i], 
                         (unsigned int)diff);
            }
#endif
        }
    }

    if (threshold == 0.0f) {
        if (error_count) {
            printf("total # of errors = %d\n", error_count);
        }
        return (error_count == 0) ? true : false;
    } else {
        if (error_count) {
            printf("%4.2f(%%) of bytes mismatched (count=%d)\n", (float)error_count*100/(float)len, error_count);
        }
        return ((len*threshold > error_count) ? true : false);
    }
}

inline bool
sdkCompareL2fe( const float* reference, const float* data,
                const unsigned int len, const float epsilon ) 
{
    assert( epsilon >= 0);

    float error = 0;
    float ref = 0;

    for( unsigned int i = 0; i < len; ++i) {

        float diff = reference[i] - data[i];
        error += diff * diff;
        ref += reference[i] * reference[i];
    }

    float normRef = sqrtf(ref);
    if (fabs(ref) < 1e-7) {
#ifdef _DEBUG
        std::cerr << "ERROR, reference l2-norm is 0\n";
#endif
        return false;
    }
    float normError = sqrtf(error);
    error = normError / normRef;
    bool result = error < epsilon;
#ifdef _DEBUG
    if( ! result) 
    {
        std::cerr << "ERROR, l2-norm error " 
            << error << " is greater than epsilon " << epsilon << "\n";
    }
#endif

    return result;
}

inline bool 
sdkLoadPPMub( const char* file, unsigned char** data, 
              unsigned int *w,unsigned int *h)
{
    unsigned int channels;
    return __loadPPM( file, data, w, h, &channels);
}

inline bool
sdkLoadPPM4ub( const char* file, unsigned char** data, 
               unsigned int *w, unsigned int *h)
{
    unsigned char *idata = 0;
    unsigned int channels;
    
    if (__loadPPM( file, &idata, w, h, &channels)) {
        // pad 4th component
        int size = *w * *h;
        // keep the original pointer
        unsigned char* idata_orig = idata;
        *data = (unsigned char*) malloc( sizeof(unsigned char) * size * 4);
        unsigned char *ptr = *data;
        for(int i=0; i<size; i++) {
            *ptr++ = *idata++;
            *ptr++ = *idata++;
            *ptr++ = *idata++;
            *ptr++ = 0;
        }
        free( idata_orig );
        return true;
    }
    else
    {
        free( idata );
        return false;
    }
}


inline bool
sdkComparePPM( const char *src_file, const char *ref_file, 
			   const float epsilon, const float threshold, bool verboseErrors )
{
	unsigned char *src_data, *ref_data;
	unsigned long error_count = 0;
	unsigned int ref_width, ref_height;
	unsigned int src_width, src_height;

	if (src_file == NULL || ref_file == NULL) {
		if(verboseErrors) std::cerr << "PPMvsPPM: src_file or ref_file is NULL.  Aborting comparison\n";
		return false;
	}

    if(verboseErrors) {
        std::cerr << "> Compare (a)rendered:  <" << src_file << ">\n";
        std::cerr << ">         (b)reference: <" << ref_file << ">\n";
    }


	if (sdkLoadPPM4ub(ref_file, &ref_data, &ref_width, &ref_height) != true) 
	{
		if(verboseErrors) std::cerr << "PPMvsPPM: unable to load ref image file: "<< ref_file << "\n";
		return false;
	}

	if (sdkLoadPPM4ub(src_file, &src_data, &src_width, &src_height) != true) 
	{
		std::cerr << "PPMvsPPM: unable to load src image file: " << src_file << "\n";
		return false;
	}

	if(src_height != ref_height || src_width != ref_width)
	{
		if(verboseErrors) std::cerr << "PPMvsPPM: source and ref size mismatch (" << src_width << 
			"," << src_height << ")vs(" << ref_width << "," << ref_height << ")\n";
	}

	if(verboseErrors) std::cerr << "PPMvsPPM: comparing images size (" << src_width << 
		"," << src_height << ") epsilon(" << epsilon << "), threshold(" << threshold*100 << "%)\n";

	if (compareData( ref_data, src_data, src_width*src_height*4, epsilon, threshold ) == false) 
	{
		error_count=1;
	}

	if (error_count == 0) { 
		if(verboseErrors) std::cerr << "    OK\n\n"; 
	} else {
		if(verboseErrors) std::cerr << "    FAILURE!  "<<error_count<<" errors...\n\n";
	}
	return (error_count == 0)? true : false;  // returns true if all pixels pass
}


////////////////////////////////////////////////////////////////////////////////
//! Timer functionality

////////////////////////////////////////////////////////////////////////////////
//! Create a new timer
//! @return true if a time has been created, otherwise false
//! @param  name of the new timer, 0 if the creation failed
////////////////////////////////////////////////////////////////////////////////
inline bool
sdkCreateTimer( StopWatchInterface **timer_interface ) 
{
#ifdef _WIN32
	*timer_interface = (StopWatchInterface *)new StopWatchWin();
#else
	*timer_interface = (StopWatchInterface *)new StopWatchLinux();
#endif
	return (*timer_interface != NULL) ? true : false;
}


////////////////////////////////////////////////////////////////////////////////
//! Delete a timer
//! @return true if a time has been deleted, otherwise false
//! @param  name of the timer to delete
////////////////////////////////////////////////////////////////////////////////
inline bool
sdkDeleteTimer( StopWatchInterface **timer_interface ) 
{
	if (*timer_interface) delete *timer_interface;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
//! Start the time with name \a name
//! @param name  name of the timer to start
////////////////////////////////////////////////////////////////////////////////
inline bool
sdkStartTimer( StopWatchInterface **timer_interface ) 
{
	if (*timer_interface) (*timer_interface)->start();
    return true;
}

////////////////////////////////////////////////////////////////////////////////
//! Stop the time with name \a name. Does not reset.
//! @param name  name of the timer to stop
////////////////////////////////////////////////////////////////////////////////
inline bool
sdkStopTimer( StopWatchInterface **timer_interface ) 
{
	if (*timer_interface) (*timer_interface)->stop();
    return true;
}

////////////////////////////////////////////////////////////////////////////////
//! Resets the timer's counter.
//! @param name  name of the timer to reset.
////////////////////////////////////////////////////////////////////////////////
inline bool
sdkResetTimer( StopWatchInterface **timer_interface )
{
	if (*timer_interface) (*timer_interface)->reset();
    return true;
}

////////////////////////////////////////////////////////////////////////////////
//! Return the average time for timer execution as the total time
//! for the timer dividied by the number of completed (stopped) runs the timer 
//! has made.
//! Excludes the current running time if the timer is currently running.
//! @param name  name of the timer to return the time of
////////////////////////////////////////////////////////////////////////////////
inline float
sdkGetAverageTimerValue( StopWatchInterface **timer_interface )
{
	if (*timer_interface) 
		return (*timer_interface)->getAverageTime();
	else
		return 0.0f;
}

////////////////////////////////////////////////////////////////////////////////
//! Total execution time for the timer over all runs since the last reset
//! or timer creation.
//! @param name  name of the timer to obtain the value of.
////////////////////////////////////////////////////////////////////////////////
inline float
sdkGetTimerValue( StopWatchInterface **timer_interface )
{  
	if (*timer_interface) 
		return (*timer_interface)->getTime();
	else 
		return 0.0f;
}

#endif //  SDK_HELPER_H
