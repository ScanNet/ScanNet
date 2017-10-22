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
 
#ifndef _CUTIL_GL_INLINE_H_
#define _CUTIL_GL_INLINE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <cuda.h>
#include <cutil.h>
#include <cufft.h>
#include <cuda_runtime_api.h>
#include <cuda_gl_interop.h>


inline int cutilGLDeviceInit(int ARGC, char **ARGV)
{
    int deviceCount;
    cutilSafeCallNoSync(cudaGetDeviceCount(&deviceCount));
    if (deviceCount == 0) {
        fprintf(stderr, "CUTIL CUDA error: no devices supporting CUDA.\n");
        exit(-1);
    }
    int dev = 0;
    cutGetCmdLineArgumenti(ARGC, (const char **) ARGV, "device", &dev);
    if (dev < 0)
        dev = 0;
    if (dev > deviceCount-1) {
		fprintf(stderr, "\n");
		fprintf(stderr, ">> %d CUDA capable GPU device(s) detected. <<\n", deviceCount);
        fprintf(stderr, ">> cutilDeviceInit (-device=%d) is not a valid GPU device. <<\n", dev);
		fprintf(stderr, "\n");
        return -dev;
    }
    cudaDeviceProp deviceProp;
    cutilSafeCallNoSync(cudaGetDeviceProperties(&deviceProp, dev));
    if (deviceProp.major < 1) {
        fprintf(stderr, "cutil error: device does not support CUDA.\n");
        exit(-1);                                                  \
    }
    if (cutCheckCmdLineFlag(ARGC, (const char **) ARGV, "quiet") == CUTFalse)
        fprintf(stderr, "Using device %d: %s\n", dev, deviceProp.name);
    cutilSafeCall(cudaGLSetGLDevice(dev));
    return dev;
}

inline int cutilGLDeviceInitDrv(int cuDevice, int ARGC, char ** ARGV) 
{
    cuDevice = 0;
    int deviceCount = 0;
    CUresult err = cuInit(0);
    if (CUDA_SUCCESS == err)
        cutilDrvSafeCallNoSync(cuDeviceGetCount(&deviceCount));
    if (deviceCount == 0) {
        fprintf(stderr, "CUTIL DeviceInitDrv error: no devices supporting CUDA\n");
        exit(-1);
    }

    int dev = 0;
    cutGetCmdLineArgumenti(ARGC, (const char **) ARGV, "device", &dev);
    if (dev < 0)
        dev = 0;
    if (dev > deviceCount-1) {
		fprintf(stderr, "\n");
		fprintf(stderr, ">> %d CUDA capable GPU device(s) detected. <<\n", deviceCount);
        fprintf(stderr, ">> cutilDeviceInit (-device=%d) is not a valid GPU device. <<\n", dev);
		fprintf(stderr, "\n");
        return -dev;
    }
    cutilDrvSafeCallNoSync(cuDeviceGet(&cuDevice, dev));
    char name[100];
    cuDeviceGetName(name, 100, cuDevice);
    if (cutCheckCmdLineFlag(ARGC, (const char **) ARGV, "quiet") == CUTFalse) {
        fprintf(stderr, "Using device %d: %s\n", dev, name);
    }
    return dev;
}

// This function will pick the best CUDA device available with OpenGL interop
inline int cutilChooseCudaGLDevice(int argc, char **argv)
{
	int devID = 0;
    // If the command-line has a device number specified, use it
    if( cutCheckCmdLineFlag(argc, (const char**)argv, "device") ) {
		devID = cutilGLDeviceInit(argc, argv);
		if (devID < 0) {
		   printf("exiting...\n");
		   cutilExit(argc, argv);
		   exit(0);
		}
    } else {
        // Otherwise pick the device with highest Gflops/s
		devID = cutGetMaxGflopsDeviceId();
        cudaGLSetGLDevice( devID );
    }
	return devID;
}

#endif // _CUTIL_GL_INLINE_H_
