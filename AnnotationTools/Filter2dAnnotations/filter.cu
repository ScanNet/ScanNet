#ifndef _FILTER_
#define _FILTER_

#include <cutil_inline.h>
#include <cutil_math.h>
#include "GlobalDefines.h"
#include "cuda_SimpleMatrixUtil.h"


#define T_PER_BLOCK 16
#define MINF __int_as_float(0xff800000)


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convert Float4 Color to UCHAR4
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__global__ void convertColorFloat4ToUCHAR4Device(uchar4* d_output, float4* d_input, unsigned int width, unsigned int height)
{
	const int x = blockIdx.x*blockDim.x + threadIdx.x;
	const int y = blockIdx.y*blockDim.y + threadIdx.y;

	if (x >= width || y >= height) return;

	float4 color = d_input[y*width + x];
	d_output[y*width + x] = make_uchar4(color.x*255.0f, color.y*255.0f, color.z*255.0f, color.w*255.0f);
}

extern "C" void convertColorFloat4ToUCHAR4(uchar4* d_output, float4* d_input, unsigned int width, unsigned int height)
{
	const dim3 blockSize((width + T_PER_BLOCK - 1) / T_PER_BLOCK, (height + T_PER_BLOCK - 1) / T_PER_BLOCK);
	const dim3 gridSize(T_PER_BLOCK, T_PER_BLOCK);

	convertColorFloat4ToUCHAR4Device << <blockSize, gridSize >> >(d_output, d_input, width, height);

#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convert Color to Intensity
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__global__ void convertColorToIntensityFloatDevice(float* d_output, float4* d_input, unsigned int width, unsigned int height)
{
	const int x = blockIdx.x*blockDim.x + threadIdx.x;
	const int y = blockIdx.y*blockDim.y + threadIdx.y;

	if (x >= width || y >= height) return;

	const float4 color = d_input[y*width + x];
	d_output[y*width + x] = 0.299f*color.x + 0.587f*color.y + 0.114f*color.z;
}

extern "C" void convertColorToIntensityFloat(float* d_output, float4* d_input, unsigned int width, unsigned int height)
{
	const dim3 gridSize((width + T_PER_BLOCK - 1) / T_PER_BLOCK, (height + T_PER_BLOCK - 1) / T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	convertColorToIntensityFloatDevice << <gridSize, blockSize >> >(d_output, d_input, width, height);

#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convert Depth to Camera Space Positions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__global__ void convertDepthFloatToCameraSpaceFloat4Device(float4* d_output, float* d_input, float4x4 intrinsicsInv, 
	unsigned int width, unsigned int height)
{
	const unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;
	const unsigned int y = blockIdx.y*blockDim.y + threadIdx.y;

	if (x < width && y < height) {
		d_output[y*width+x] = make_float4(MINF, MINF, MINF, MINF);

		float depth = d_input[y*width+x];

		if(depth != MINF)
		{
			float4 cameraSpace(intrinsicsInv*make_float4((float)x*depth, (float)y*depth, depth, depth));
			d_output[y*width+x] = make_float4(cameraSpace.x, cameraSpace.y, cameraSpace.w, 1.0f);
		}
	}
}

extern "C" void convertDepthFloatToCameraSpaceFloat4(float4* d_output, float* d_input, float4x4 intrinsicsInv, unsigned int width, unsigned int height)
{
	const dim3 gridSize((width + T_PER_BLOCK - 1)/T_PER_BLOCK, (height + T_PER_BLOCK - 1)/T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	convertDepthFloatToCameraSpaceFloat4Device<<<gridSize, blockSize>>>(d_output, d_input, intrinsicsInv, width, height);

#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compute Normal Map
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__global__ void computeNormalsDevice(float4* d_output, float4* d_input, unsigned int width, unsigned int height)
{
	const unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;
	const unsigned int y = blockIdx.y*blockDim.y + threadIdx.y;

	if(x >= width || y >= height) return;

	d_output[y*width+x] = make_float4(MINF, MINF, MINF, MINF);

	if(x > 0 && x < width-1 && y > 0 && y < height-1)
	{
		const float4 CC = d_input[(y+0)*width+(x+0)];
		const float4 PC = d_input[(y+1)*width+(x+0)];
		const float4 CP = d_input[(y+0)*width+(x+1)];
		const float4 MC = d_input[(y-1)*width+(x+0)];
		const float4 CM = d_input[(y+0)*width+(x-1)];

		if(CC.x != MINF && PC.x != MINF && CP.x != MINF && MC.x != MINF && CM.x != MINF)
		{
			const float3 n = cross(make_float3(PC)-make_float3(MC), make_float3(CP)-make_float3(CM));
			const float  l = length(n);

			if(l > 0.0f)
			{
				d_output[y*width+x] = make_float4(n/-l, 1.0f);
			}
		}
	}
}

extern "C" void computeNormals(float4* d_output, float4* d_input, unsigned int width, unsigned int height)
{
	const dim3 gridSize((width + T_PER_BLOCK - 1)/T_PER_BLOCK, (height + T_PER_BLOCK - 1)/T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	computeNormalsDevice<<<gridSize, blockSize>>>(d_output, d_input, width, height);

#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}

/////////////////////////////////////////////
// Transform
/////////////////////////////////////////////

__global__ void transformNormalMapDevice(float4* d_normals, unsigned int imageWidth, unsigned int imageHeight, float4x4 transform)
{
	const int x = blockIdx.x*blockDim.x + threadIdx.x;
	const int y = blockIdx.y*blockDim.y + threadIdx.y;

	const int index = y*imageWidth+x;

	if(x >= 0 && x < imageWidth && y >= 0 && y < imageHeight)
	{
		if(d_normals[index].x != MINF)
		{
			float3 n = transform.getFloat3x3() * make_float3(d_normals[index].x,d_normals[index].y,d_normals[index].z); 
			d_normals[index] = make_float4(n, 0.0f);
		}
	}
}

extern "C" void transformNormalMap(float4* d_normals, unsigned int imageWidth, unsigned int imageHeight, float4x4 transform)
{
	const dim3 gridSize((imageWidth + T_PER_BLOCK - 1)/T_PER_BLOCK, (imageHeight + T_PER_BLOCK - 1)/T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	transformNormalMapDevice<<<gridSize, blockSize>>>(d_normals, imageWidth, imageHeight, transform);
	#ifdef _DEBUG
		cutilSafeCall(cudaDeviceSynchronize());
		cutilCheckMsg(__FUNCTION__);
	#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bilateral Filter Float Map
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline __device__ float gaussR(float sigma, float dist)
{
	return exp(-(dist*dist) / (2.0*sigma*sigma));
}

inline __device__ float linearR(float sigma, float dist)
{
	return max(1.0f, min(0.0f, 1.0f - (dist*dist) / (2.0*sigma*sigma)));
}

inline __device__ float gaussD(float sigma, int x, int y)
{
	return exp(-((x*x + y*y) / (2.0f*sigma*sigma)));
}

inline __device__ float gaussD(float sigma, int x)
{
	return exp(-((x*x) / (2.0f*sigma*sigma)));
}

__global__ void bilateralFilterFloatMapDevice(float* d_output, float* d_input, float sigmaD, float sigmaR, unsigned int width, unsigned int height)
{
	const int x = blockIdx.x*blockDim.x + threadIdx.x;
	const int y = blockIdx.y*blockDim.y + threadIdx.y;

	if (x >= width || y >= height) return;

	const int kernelRadius = (int)ceil(2.0*sigmaD);

	d_output[y*width + x] = MINF;

	float sum = 0.0f;
	float sumWeight = 0.0f;

	const float depthCenter = d_input[y*width + x];
	if (depthCenter != MINF)
	{
		for (int m = x - kernelRadius; m <= x + kernelRadius; m++)
		{
			for (int n = y - kernelRadius; n <= y + kernelRadius; n++)
			{
				if (m >= 0 && n >= 0 && m < width && n < height)
				{
					const float currentDepth = d_input[n*width + m];

					if (currentDepth != MINF) {
						const float weight = gaussD(sigmaD, m - x, n - y)*gaussR(sigmaR, currentDepth - depthCenter);

						sumWeight += weight;
						sum += weight*currentDepth;
					}
				}
			}
		}

		if (sumWeight > 0.0f) d_output[y*width + x] = sum / sumWeight;
	}
}

extern "C" void bilateralFilterFloatMap(float* d_output, float* d_input, float sigmaD, float sigmaR, unsigned int width, unsigned int height)
{
	const dim3 gridSize((width + T_PER_BLOCK - 1) / T_PER_BLOCK, (height + T_PER_BLOCK - 1) / T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	bilateralFilterFloatMapDevice << <gridSize, blockSize >> >(d_output, d_input, sigmaD, sigmaR, width, height);
#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bilateral Filter Float4 Map
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__global__ void bilateralFilterFloat4MapDevice(float4* d_output, float4* d_input, float sigmaD, float sigmaR, unsigned int width, unsigned int height)
{
	const int x = blockIdx.x*blockDim.x + threadIdx.x;
	const int y = blockIdx.y*blockDim.y + threadIdx.y;

	if (x >= width || y >= height) return;

	const int kernelRadius = (int)ceil(2.0*sigmaD);

	//d_output[y*width+x] = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
	d_output[y*width + x] = make_float4(MINF, MINF, MINF, MINF);

	float4 sum = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
	float sumWeight = 0.0f;

	const float4 depthCenter = d_input[y*width + x];
	if (depthCenter.x != MINF) {
		for (int m = x - kernelRadius; m <= x + kernelRadius; m++)
		{
			for (int n = y - kernelRadius; n <= y + kernelRadius; n++)
			{
				if (m >= 0 && n >= 0 && m < width && n < height)
				{
					const float4 currentDepth = d_input[n*width + m];

					if (currentDepth.x != MINF) {
						const float weight = gaussD(sigmaD, m - x, n - y)*gaussR(sigmaR, length(currentDepth - depthCenter));

						sum += weight*currentDepth;
						sumWeight += weight;
					}
				}
			}
		}
	}
	if (sumWeight > 0.0f) d_output[y*width + x] = sum / sumWeight;
}

extern "C" void bilateralFilterFloat4Map(float4* d_output, float4* d_input, float sigmaD, float sigmaR, unsigned int width, unsigned int height)
{
	const dim3 gridSize((width + T_PER_BLOCK - 1) / T_PER_BLOCK, (height + T_PER_BLOCK - 1) / T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	bilateralFilterFloat4MapDevice << <gridSize, blockSize >> >(d_output, d_input, sigmaD, sigmaR, width, height);
#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Gauss Filter Float Map
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__global__ void gaussFilterFloatMapDevice(float* d_output, float* d_input, float sigmaD, float sigmaR, unsigned int width, unsigned int height)
{
	const int x = blockIdx.x*blockDim.x + threadIdx.x;
	const int y = blockIdx.y*blockDim.y + threadIdx.y;

	if (x >= width || y >= height) return;

	const int kernelRadius = (int)ceil(2.0*sigmaD);

	d_output[y*width + x] = MINF;

	float sum = 0.0f;
	float sumWeight = 0.0f;

	const float depthCenter = d_input[y*width + x];
	if (depthCenter != MINF)
	{
		for (int m = x - kernelRadius; m <= x + kernelRadius; m++)
		{
			for (int n = y - kernelRadius; n <= y + kernelRadius; n++)
			{
				if (m >= 0 && n >= 0 && m < width && n < height)
				{
					const float currentDepth = d_input[n*width + m];

					if (currentDepth != MINF && fabs(depthCenter - currentDepth) < sigmaR)
					{
						const float weight = gaussD(sigmaD, m - x, n - y);

						sumWeight += weight;
						sum += weight*currentDepth;
					}
				}
			}
		}
	}

	if (sumWeight > 0.0f) d_output[y*width + x] = sum / sumWeight;
}

extern "C" void gaussFilterFloatMap(float* d_output, float* d_input, float sigmaD, float sigmaR, unsigned int width, unsigned int height)
{
	const dim3 gridSize((width + T_PER_BLOCK - 1) / T_PER_BLOCK, (height + T_PER_BLOCK - 1) / T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	gaussFilterFloatMapDevice << <gridSize, blockSize >> >(d_output, d_input, sigmaD, sigmaR, width, height);
#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Gauss Filter Float4 Map
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__global__ void gaussFilterFloat4MapDevice(float4* d_output, float4* d_input, float sigmaD, float sigmaR, unsigned int width, unsigned int height)
{
	const int x = blockIdx.x*blockDim.x + threadIdx.x;
	const int y = blockIdx.y*blockDim.y + threadIdx.y;

	if (x >= width || y >= height) return;

	const int kernelRadius = (int)ceil(2.0*sigmaD);

	//d_output[y*width+x] = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
	d_output[y*width + x] = make_float4(MINF, MINF, MINF, MINF);

	float4 sum = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
	float sumWeight = 0.0f;

	const float4 center = d_input[y*width + x];
	if (center.x != MINF) {
		for (int m = x - kernelRadius; m <= x + kernelRadius; m++)
		{
			for (int n = y - kernelRadius; n <= y + kernelRadius; n++)
			{
				if (m >= 0 && n >= 0 && m < width && n < height)
				{
					const float4 current = d_input[n*width + m];

					if (current.x != MINF) {
						if (length(center - current) < sigmaR)
						{
							const float weight = gaussD(sigmaD, m - x, n - y);

							sumWeight += weight;
							sum += weight*current;
						}
					}
				}
			}
		}
	}

	if (sumWeight > 0.0f) d_output[y*width + x] = sum / sumWeight;
}

extern "C" void gaussFilterFloat4Map(float4* d_output, float4* d_input, float sigmaD, float sigmaR, unsigned int width, unsigned int height)
{
	const dim3 gridSize((width + T_PER_BLOCK - 1) / T_PER_BLOCK, (height + T_PER_BLOCK - 1) / T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	gaussFilterFloat4MapDevice << <gridSize, blockSize >> >(d_output, d_input, sigmaD, sigmaR, width, height);
#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compute Edge Mask
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__global__ void computeMaskEdgeMapFloat4Device(unsigned char* d_output, float4* d_input, float* d_indepth, float threshold, unsigned int width, unsigned int height)
{
	const unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;
	const unsigned int y = blockIdx.y*blockDim.y + threadIdx.y;

	if (x >= width || y >= height) return;

	d_output[y*width + x] = 1;
	d_output[width*height + y*width + x] = 1;

	const float thre = threshold *threshold *3.0f;
	if (x > 0 && y > 0 && x < width - 1 && y < height - 1)
	{
		if (d_indepth[y*width + x] == MINF)
		{
			d_output[y*width + x] = 0;
			d_output[y*width + x - 1] = 0;
			d_output[width*height + y*width + x] = 0;
			d_output[width*height + (y - 1)*width + x] = 0;

			return;
		}

		const float4& p0 = d_input[(y + 0)*width + (x + 0)];
		const float4& p1 = d_input[(y + 0)*width + (x + 1)];
		const float4& p2 = d_input[(y + 1)*width + (x + 0)];

		float dU = sqrt(((p1.x - p0.x)*(p1.x - p0.x) + (p1.y - p0.y) * (p1.y - p0.y) + (p1.z - p0.z)*(p1.z - p0.z)) / 3.0f);
		float dV = sqrt(((p2.x - p0.x)*(p2.x - p0.x) + (p2.y - p0.y) * (p2.y - p0.y) + (p2.z - p0.z)*(p2.z - p0.z)) / 3.0f);

		//float dgradx = abs(d_indepth[y*width+x-1] + d_indepth[y*width+x+1] - 2.0f * d_indepth[y*width+x]);
		//float dgrady = abs(d_indepth[y*width+x-width] + d_indepth[y*width+x+width] - 2.0f * d_indepth[y*width+x]);


		if (dU > thre) d_output[y*width + x] = 0;
		if (dV > thre) d_output[width*height + y*width + x] = 0;

		//remove depth discontinuities
		const int r = 1;
		const float thres = 0.01f;

		const float pCC = d_indepth[y*width + x];
		for (int i = -r; i <= r; i++)
		{
			for (int j = -r; j <= r; j++)
			{
				int currentX = x + j;
				int currentY = y + i;

				if (currentX >= 0 && currentX < width && currentY >= 0 && currentY < height)
				{
					float d = d_indepth[currentY*width + currentX];

					if (d != MINF && abs(pCC - d) > thres)
					{
						d_output[y*width + x] = 0;
						d_output[width*height + y*width + x] = 0;
						return;
					}
				}
			}
		}
	}
}

extern "C" void computeMaskEdgeMapFloat4(unsigned char* d_output, float4* d_input, float* d_indepth, float threshold, unsigned int width, unsigned int height)
{
	const dim3 gridSize((width + T_PER_BLOCK - 1) / T_PER_BLOCK, (height + T_PER_BLOCK - 1) / T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	computeMaskEdgeMapFloat4Device << <gridSize, blockSize >> >(d_output, d_input, d_indepth, threshold, width, height);
#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Resample Float Map
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline __device__ float bilinearInterpolationFloat(float x, float y, float* d_input, unsigned int imageWidth, unsigned int imageHeight)
{
	const int2 p00 = make_int2(floor(x), floor(y));
	const int2 p01 = p00 + make_int2(0.0f, 1.0f);
	const int2 p10 = p00 + make_int2(1.0f, 0.0f);
	const int2 p11 = p00 + make_int2(1.0f, 1.0f);

	const float alpha = x - p00.x;
	const float beta = y - p00.y;

	float s0 = 0.0f; float w0 = 0.0f;
	if (p00.x < imageWidth && p00.y < imageHeight) { float v00 = d_input[p00.y*imageWidth + p00.x]; if (v00 != MINF) { s0 += (1.0f - alpha)*v00; w0 += (1.0f - alpha); } }
	if (p10.x < imageWidth && p10.y < imageHeight) { float v10 = d_input[p10.y*imageWidth + p10.x]; if (v10 != MINF) { s0 += alpha *v10; w0 += alpha; } }

	float s1 = 0.0f; float w1 = 0.0f;
	if (p01.x < imageWidth && p01.y < imageHeight) { float v01 = d_input[p01.y*imageWidth + p01.x]; if (v01 != MINF) { s1 += (1.0f - alpha)*v01; w1 += (1.0f - alpha); } }
	if (p11.x < imageWidth && p11.y < imageHeight) { float v11 = d_input[p11.y*imageWidth + p11.x]; if (v11 != MINF) { s1 += alpha *v11; w1 += alpha; } }

	const float p0 = s0 / w0;
	const float p1 = s1 / w1;

	float ss = 0.0f; float ww = 0.0f;
	if (w0 > 0.0f) { ss += (1.0f - beta)*p0; ww += (1.0f - beta); }
	if (w1 > 0.0f) { ss += beta *p1; ww += beta; }

	if (ww > 0.0f) return ss / ww;
	else		  return MINF;
}

__global__ void resampleFloatMapDevice(float* d_colorMapResampledFloat, float* d_colorMapFloat, unsigned int inputWidth, unsigned int inputHeight, unsigned int outputWidth, unsigned int outputHeight)
{
	const int x = blockIdx.x*blockDim.x + threadIdx.x;
	const int y = blockIdx.y*blockDim.y + threadIdx.y;

	if (x < outputWidth && y < outputHeight)
	{
		const float scaleWidth = (float)(inputWidth - 1) / (float)(outputWidth - 1);
		const float scaleHeight = (float)(inputHeight - 1) / (float)(outputHeight - 1);

		const unsigned int xInput = (unsigned int)(x*scaleWidth + 0.5f);
		const unsigned int yInput = (unsigned int)(y*scaleHeight + 0.5f);

		if (xInput < inputWidth && yInput < inputHeight)
		{
			d_colorMapResampledFloat[y*outputWidth + x] = bilinearInterpolationFloat((float)x*scaleWidth, (float)y*scaleHeight, d_colorMapFloat, inputWidth, inputHeight);
		}
	}
}

extern "C" void resampleFloatMap(float* d_colorMapResampledFloat, unsigned int outputWidth, unsigned int outputHeight, float* d_colorMapFloat, unsigned int inputWidth, unsigned int inputHeight)
{
	const dim3 gridSize((outputWidth + T_PER_BLOCK - 1) / T_PER_BLOCK, (outputHeight + T_PER_BLOCK - 1) / T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	resampleFloatMapDevice << <gridSize, blockSize >> >(d_colorMapResampledFloat, d_colorMapFloat, inputWidth, inputHeight, outputWidth, outputHeight);
#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Resample Float4 Map
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline __device__ float4 bilinearInterpolationFloat4(float x, float y, float4* d_input, unsigned int imageWidth, unsigned int imageHeight)
{
	const int2 p00 = make_int2(floor(x), floor(y));
	const int2 p01 = p00 + make_int2(0.0f, 1.0f);
	const int2 p10 = p00 + make_int2(1.0f, 0.0f);
	const int2 p11 = p00 + make_int2(1.0f, 1.0f);

	const float alpha = x - p00.x;
	const float beta = y - p00.y;

	//const float INVALID = 0.0f;
	const float INVALID = MINF;

	float4 s0 = make_float4(0.0f, 0.0f, 0.0f, 0.0f); float w0 = 0.0f;
	if (p00.x < imageWidth && p00.y < imageHeight) { float4 v00 = d_input[p00.y*imageWidth + p00.x]; if (v00.x != INVALID && v00.y != INVALID && v00.z != INVALID) { s0 += (1.0f - alpha)*v00; w0 += (1.0f - alpha); } }
	if (p10.x < imageWidth && p10.y < imageHeight) { float4 v10 = d_input[p10.y*imageWidth + p10.x]; if (v10.x != INVALID && v10.y != INVALID && v10.z != INVALID) { s0 += alpha *v10; w0 += alpha; } }

	float4 s1 = make_float4(0.0f, 0.0f, 0.0f, 0.0f); float w1 = 0.0f;
	if (p01.x < imageWidth && p01.y < imageHeight) { float4 v01 = d_input[p01.y*imageWidth + p01.x]; if (v01.x != INVALID && v01.y != INVALID && v01.z != INVALID) { s1 += (1.0f - alpha)*v01; w1 += (1.0f - alpha); } }
	if (p11.x < imageWidth && p11.y < imageHeight) { float4 v11 = d_input[p11.y*imageWidth + p11.x]; if (v11.x != INVALID && v11.y != INVALID && v11.z != INVALID) { s1 += alpha *v11; w1 += alpha; } }

	const float4 p0 = s0 / w0;
	const float4 p1 = s1 / w1;

	float4 ss = make_float4(0.0f, 0.0f, 0.0f, 0.0f); float ww = 0.0f;
	if (w0 > 0.0f) { ss += (1.0f - beta)*p0; ww += (1.0f - beta); }
	if (w1 > 0.0f) { ss += beta *p1; ww += beta; }

	if (ww > 0.0f) return ss / ww;
	else		  return make_float4(MINF, MINF, MINF, MINF);
}

__global__ void resampleFloat4MapDevice(float4* d_colorMapResampledFloat4, float4* d_colorMapFloat4, unsigned int inputWidth, unsigned int inputHeight, unsigned int outputWidth, unsigned int outputHeight)
{
	const int x = blockIdx.x*blockDim.x + threadIdx.x;
	const int y = blockIdx.y*blockDim.y + threadIdx.y;

	if (x < outputWidth && y < outputHeight)
	{
		const float scaleWidth = (float)(inputWidth - 1) / (float)(outputWidth - 1);
		const float scaleHeight = (float)(inputHeight - 1) / (float)(outputHeight - 1);

		const unsigned int xInput = (unsigned int)(x*scaleWidth + 0.5f);
		const unsigned int yInput = (unsigned int)(y*scaleHeight + 0.5f);

		if (xInput < inputWidth && yInput < inputHeight)
		{
			d_colorMapResampledFloat4[y*outputWidth + x] = bilinearInterpolationFloat4(x*scaleWidth, y*scaleHeight, d_colorMapFloat4, inputWidth, inputHeight);
		}
	}
}

extern "C" void resampleFloat4Map(float4* d_colorMapResampledFloat4, unsigned int outputWidth, unsigned int outputHeight, float4* d_colorMapFloat4, unsigned int inputWidth, unsigned int inputHeight)
{
	const dim3 gridSize((outputWidth + T_PER_BLOCK - 1) / T_PER_BLOCK, (outputHeight + T_PER_BLOCK - 1) / T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	resampleFloat4MapDevice << <gridSize, blockSize >> >(d_colorMapResampledFloat4, d_colorMapFloat4, inputWidth, inputHeight, outputWidth, outputHeight);
#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Resample Unsigned Char Map
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__global__ void resampleUCharMapDevice(unsigned char* d_MapResampled, unsigned int outputWidth, unsigned int outputHeight,
	unsigned char* d_Map, unsigned int inputWidth, unsigned int inputHeight)
{
	const int x = blockIdx.x*blockDim.x + threadIdx.x;
	const int y = blockIdx.y*blockDim.y + threadIdx.y;

	if (x < outputWidth && y < outputHeight)
	{
		const float scaleWidth = (float)(inputWidth - 1) / (float)(outputWidth - 1);
		const float scaleHeight = (float)(inputHeight - 1) / (float)(outputHeight - 1);

		const unsigned int xInput = (unsigned int)(x*scaleWidth + 0.5f);
		const unsigned int yInput = (unsigned int)(y*scaleHeight + 0.5f);

		if (xInput < inputWidth && yInput < inputHeight)
		{
			d_MapResampled[y*outputWidth + x] = d_Map[yInput*inputWidth + xInput];
		}
	}
}

extern "C" void resampleUCharMap(unsigned char* d_MapResampled, unsigned int outputWidth, unsigned int outputHeight,
	unsigned char* d_Map, unsigned int inputWidth, unsigned int inputHeight)
{
	const dim3 gridSize((outputWidth + T_PER_BLOCK - 1) / T_PER_BLOCK, (outputHeight + T_PER_BLOCK - 1) / T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	resampleUCharMapDevice << <gridSize, blockSize >> >(d_MapResampled, outputWidth, outputHeight, d_Map, inputWidth, inputHeight);
#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convert Edge Mask to Float Map
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__global__ void convertEdgeMaskToFloatDevice(float* d_output, unsigned char* d_input, unsigned int width, unsigned int height)
{
	const int x = blockIdx.x*blockDim.x + threadIdx.x;
	const int y = blockIdx.y*blockDim.y + threadIdx.y;

	if (x >= width || y >= height) return;

	d_output[y*width + x] = min(d_input[y*width + x], d_input[width*height + y*width + x]);
}

extern "C" void convertEdgeMaskToFloat(float* d_output, unsigned char* d_input, unsigned int width, unsigned int height)
{
	const dim3 gridSize((width + T_PER_BLOCK - 1) / T_PER_BLOCK, (height + T_PER_BLOCK - 1) / T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	convertEdgeMaskToFloatDevice << <gridSize, blockSize >> >(d_output, d_input, width, height);
#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dilate Depth Map
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__global__ void dilateDepthMapDevice(float* d_output, float* d_input, float* d_inputOrig, int structureSize, int width, int height)
{
	const int x = blockIdx.x*blockDim.x + threadIdx.x;
	const int y = blockIdx.y*blockDim.y + threadIdx.y;

	if (x >= 0 && x < width && y >= 0 && y < height)
	{
		float sum = 0.0f;
		float count = 0.0f;
		float oldDepth = d_inputOrig[y*width + x];
		if (oldDepth != MINF && oldDepth != 0)
		{
			for (int i = -structureSize; i <= structureSize; i++)
			{
				for (int j = -structureSize; j <= structureSize; j++)
				{
					if (x + j >= 0 && x + j < width && y + i >= 0 && y + i < height)
					{
						const float d = d_input[(y + i)*width + (x + j)];

						if (d != MINF && d != 0.0f && fabs(d - oldDepth) < 0.05f)
						{
							sum += d;
							count += 1.0f;
						}
					}
				}
			}
		}

		if (count > ((2 * structureSize + 1)*(2 * structureSize + 1)) / 36) d_output[y*width + x] = 1.0f;
		else			 d_output[y*width + x] = MINF;
	}
}

extern "C" void dilateDepthMapMask(float* d_output, float* d_input, float* d_inputOrig, int structureSize, int width, int height)
{
	const dim3 gridSize((width + T_PER_BLOCK - 1) / T_PER_BLOCK, (height + T_PER_BLOCK - 1) / T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	dilateDepthMapDevice << <gridSize, blockSize >> >(d_output, d_input, d_inputOrig, structureSize, width, height);
#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Mean Filter Depth Map
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__global__ void removeDevMeanMapMaskDevice(float* d_output, float* d_input, int structureSize, int width, int height)
{
	const int x = blockIdx.x*blockDim.x + threadIdx.x;
	const int y = blockIdx.y*blockDim.y + threadIdx.y;

	d_output[y*width + x] = d_input[y*width + x];

	if (x >= 0 && x < width && y >= 0 && y < height)
	{
		float oldDepth = d_input[y*width + x];

		float mean = 0.0f;
		float meanSquared = 0.0f;
		float count = 0.0f;
		for (int i = -structureSize; i <= structureSize; i++)
		{
			for (int j = -structureSize; j <= structureSize; j++)
			{
				if (x + j >= 0 && x + j < width && y + i >= 0 && y + i < height)
				{
					float depth = d_input[(y + i)*width + (x + j)];
					if (depth == MINF)
					{
						depth = 8.0f;
					}

					if (depth > 0.0f)
					{
						mean += depth;
						meanSquared += depth*depth;
						count += 1.0f;
					}
				}
			}
		}

		mean /= count;
		meanSquared /= count;

		float stdDev = sqrt(meanSquared - mean*mean);

		if (fabs(oldDepth - mean) > 0.5f*stdDev)// || stdDev> 0.005f)
		{
			d_output[y*width + x] = MINF;
		}
	}
}

extern "C" void removeDevMeanMapMask(float* d_output, float* d_input, int structureSize, unsigned int width, unsigned int height)
{
	const dim3 gridSize((width + T_PER_BLOCK - 1) / T_PER_BLOCK, (height + T_PER_BLOCK - 1) / T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	removeDevMeanMapMaskDevice << <gridSize, blockSize >> >(d_output, d_input, structureSize, width, height);
#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}




// Nearest neighbour
inline __device__ bool getValueNearestNeighbourNoCheck(const float2& p, const float4* inputMap, unsigned int imageWidth, unsigned int imageHeight, float4* outValue)
{
	const int u = (int)(p.x + 0.5f);
	const int v = (int)(p.y + 0.5f);

	if (u < 0 || u > imageWidth || v < 0 || v > imageHeight) return false;

	*outValue = inputMap[v*imageWidth + u];

	return true;
}

inline __device__ bool getValueNearestNeighbour(const float2& p, const float4* inputMap, unsigned int imageWidth, unsigned int imageHeight, float4* outValue)
{
	bool valid = getValueNearestNeighbourNoCheck(p, inputMap, imageWidth, imageHeight, outValue);
	return valid && (outValue->x != MINF && outValue->y != MINF && outValue->z != MINF);
}

// Nearest neighbour
inline __device__ bool getValueNearestNeighbourFloatNoCheck(const float2& p, const float* inputMap, unsigned int imageWidth, unsigned int imageHeight, float* outValue)
{
	const int u = (int)(p.x + 0.5f);
	const int v = (int)(p.y + 0.5f);

	if (u < 0 || u > imageWidth || v < 0 || v > imageHeight) return false;

	*outValue = inputMap[v*imageWidth + u];

	return true;
}

inline __device__ bool getValueNearestNeighbourFloat(const float2& p, const float* inputMap, unsigned int imageWidth, unsigned int imageHeight, float* outValue)
{
	bool valid = getValueNearestNeighbourFloatNoCheck(p, inputMap, imageWidth, imageHeight, outValue);
	return valid && (*outValue != MINF);
}

/////////////////////////////////////////////
// Compute Intensity and Derivatives
/////////////////////////////////////////////

__global__ void computeIntensityAndDerivativesDevice(float* d_intensity, unsigned int imageWidth, unsigned int imageHeight, float4* d_intensityAndDerivatives)
{
	const int x = blockIdx.x*blockDim.x + threadIdx.x;
	const int y = blockIdx.y*blockDim.y + threadIdx.y;

	const int index = y*imageWidth + x;

	if (x >= 0 && x < imageWidth && y >= 0 && y < imageHeight)
	{
		d_intensityAndDerivatives[index] = make_float4(MINF, MINF, MINF, MINF);

		if (x > 0 && x < imageWidth - 1 && y > 0 && y < imageHeight - 1)
		{
			float pos00; bool valid00 = getValueNearestNeighbourFloat(make_float2(x - 1, y - 1), d_intensity, imageWidth, imageHeight, &pos00); if (!valid00) return;
			float pos01; bool valid01 = getValueNearestNeighbourFloat(make_float2(x - 1, y - 0), d_intensity, imageWidth, imageHeight, &pos01); if (!valid01) return;
			float pos02; bool valid02 = getValueNearestNeighbourFloat(make_float2(x - 1, y + 1), d_intensity, imageWidth, imageHeight, &pos02); if (!valid02) return;

			float pos10; bool valid10 = getValueNearestNeighbourFloat(make_float2(x - 0, y - 1), d_intensity, imageWidth, imageHeight, &pos10); if (!valid10) return;
			float pos11; bool valid11 = getValueNearestNeighbourFloat(make_float2(x - 0, y - 0), d_intensity, imageWidth, imageHeight, &pos11); if (!valid11) return;
			float pos12; bool valid12 = getValueNearestNeighbourFloat(make_float2(x - 0, y + 1), d_intensity, imageWidth, imageHeight, &pos12); if (!valid12) return;

			float pos20; bool valid20 = getValueNearestNeighbourFloat(make_float2(x + 1, y - 1), d_intensity, imageWidth, imageHeight, &pos20); if (!valid20) return;
			float pos21; bool valid21 = getValueNearestNeighbourFloat(make_float2(x + 1, y - 0), d_intensity, imageWidth, imageHeight, &pos21); if (!valid21) return;
			float pos22; bool valid22 = getValueNearestNeighbourFloat(make_float2(x + 1, y + 1), d_intensity, imageWidth, imageHeight, &pos22); if (!valid22) return;

			float resU = (-1.0f)*pos00 + (1.0f)*pos20 +
				(-2.0f)*pos01 + (2.0f)*pos21 +
				(-1.0f)*pos02 + (1.0f)*pos22;
			resU /= 8.0f;

			float resV = (-1.0f)*pos00 + (-2.0f)*pos10 + (-1.0f)*pos20 +
				(1.0f)*pos02 + (2.0f)*pos12 + (1.0f)*pos22;
			resV /= 8.0f;

			d_intensityAndDerivatives[index] = make_float4(pos11, resU, resV, 1.0f);
		}
	}
}

extern "C" void computeIntensityAndDerivatives(float* d_intensity, unsigned int imageWidth, unsigned int imageHeight, float4* d_intensityAndDerivatives)
{
	const dim3 gridSize((imageWidth + T_PER_BLOCK - 1) / T_PER_BLOCK, (imageHeight + T_PER_BLOCK - 1) / T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	computeIntensityAndDerivativesDevice << <gridSize, blockSize >> >(d_intensity, imageWidth, imageHeight, d_intensityAndDerivatives);
#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}


/////////////////////////////////////////////
// Compute grdient intensity magnitude
/////////////////////////////////////////////

__global__ void computeGradientIntensityMagnitudeDevice(float4* d_inputDU, float4* d_inputDV, unsigned int imageWidth, unsigned int imageHeight, float4* d_ouput)
{
	const int x = blockIdx.x*blockDim.x + threadIdx.x;
	const int y = blockIdx.y*blockDim.y + threadIdx.y;

	const int index = y*imageWidth + x;

	d_ouput[index] = make_float4(MINF, MINF, MINF, MINF);

	float4 DU = d_inputDU[index];
	float4 DV = d_inputDV[index];

	if (DU.x != MINF && DV.x != MINF)
	{
		float m = sqrtf(DU.x*DU.x + DV.x*DV.x);

		if (m > 0.005f)
		{
			d_ouput[index] = make_float4(m, m, m, 1.0f);
		}
	}
}

extern "C" void computeGradientIntensityMagnitude(float4* d_inputDU, float4* d_inputDV, unsigned int imageWidth, unsigned int imageHeight, float4* d_ouput)
{
	const dim3 gridSize((imageWidth + T_PER_BLOCK - 1) / T_PER_BLOCK, (imageHeight + T_PER_BLOCK - 1) / T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	computeGradientIntensityMagnitudeDevice << <gridSize, blockSize >> >(d_inputDU, d_inputDV, imageWidth, imageHeight, d_ouput);
#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Erode Depth Map
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__global__ void erodeDepthMapDevice(float* d_output, float* d_input, int structureSize, int width, int height, float dThresh, float fracReq)
{
	const int x = blockIdx.x*blockDim.x + threadIdx.x;
	const int y = blockIdx.y*blockDim.y + threadIdx.y;


	if (x >= 0 && x < width && y >= 0 && y < height)
	{


		unsigned int count = 0;

		float oldDepth = d_input[y*width + x];
		for (int i = -structureSize; i <= structureSize; i++)
		{
			for (int j = -structureSize; j <= structureSize; j++)
			{
				if (x + j >= 0 && x + j < width && y + i >= 0 && y + i < height)
				{
					float depth = d_input[(y + i)*width + (x + j)];
					if (depth == MINF || depth == 0.0f || fabs(depth - oldDepth) > dThresh)
					{
						count++;
						//d_output[y*width+x] = MINF;
						//return;
					}
				}
			}
		}

		unsigned int sum = (2 * structureSize + 1)*(2 * structureSize + 1);
		if ((float)count / (float)sum >= fracReq) {
			d_output[y*width + x] = MINF;
		}
		else {
			d_output[y*width + x] = d_input[y*width + x];
		}
	}
}

extern "C" void erodeDepthMap(float* d_output, float* d_input, int structureSize, unsigned int width, unsigned int height, float dThresh, float fracReq)
{
	const dim3 gridSize((width + T_PER_BLOCK - 1) / T_PER_BLOCK, (height + T_PER_BLOCK - 1) / T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	erodeDepthMapDevice << <gridSize, blockSize >> >(d_output, d_input, structureSize, width, height, dThresh, fracReq);
#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// filter annotations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__global__ void filterAnnotations_Kernel(unsigned char* d_outputInstance, const unsigned char* d_inputInstance,
	const float* d_depth, const float* d_intensity, const unsigned char* d_instanceToIdx,
	const unsigned char* d_idxToInstance, float* d_vote,
	int structureSize, int width, int height, float sigmaD, float sigmaR, float intensityScale)
{
	const int x = blockIdx.x*blockDim.x + threadIdx.x;
	const int y = blockIdx.y*blockDim.y + threadIdx.y;
	const int voteoffset = (y*width + x) * MAX_NUM_LABELS_PER_SCENE;

	if (x >= 0 && x < width && y >= 0 && y < height) {
		d_outputInstance[y*width + x] = d_inputInstance[y*width + x];
		float depthCenter = d_depth[y*width + x];
		float intensityCenter = d_intensity[y*width + x];
		for (int i = -structureSize; i <= structureSize; i++) {
			for (int j = -structureSize; j <= structureSize; j++) {
				if (x + j >= 0 && x + j < width && y + i >= 0 && y + i < height)
				{
					float depth = d_depth[(y + i)*width + (x + j)];
					float intensity = d_intensity[(y + i)*width + (x + j)];
					float intensityOffset = std::abs(intensityCenter - intensity) * intensityScale; //bring intensity to approx scale of depth
					float depthOffset = 0.0f;
					if (depthCenter != MINF && depth != MINF)
						depthOffset = std::abs(depthCenter - depth);
					const float weight = gaussD(sigmaD, j, i)*gaussR(sigmaR, depthOffset)*gaussR(sigmaR, intensityOffset);
					unsigned char val = d_inputInstance[(y + i)*width + (x + j)];
					unsigned char idx = d_instanceToIdx[val];
					d_vote[voteoffset + idx] += weight;
				}
			} //j
		} //i
		float maxWeight = 0.0f; unsigned char bestVal = 0; //TODO fix this part...
		for (int i = 0; i < MAX_NUM_LABELS_PER_SCENE; i++) {
			if (d_vote[voteoffset + i] > maxWeight) {
				maxWeight = d_vote[voteoffset + i];
				bestVal = d_idxToInstance[i];
			}
		}
		d_outputInstance[y*width + x] = bestVal;
	} //in bounds of image
}

extern "C" void filterAnnotations(unsigned char* d_outputInstance, const unsigned char* d_inputInstance,
	const float* d_depth, const float* d_intensity, const unsigned char* d_instanceToIdx,
	const unsigned char* d_idxToInstance, float* d_vote,
	int structureSize, int width, int height, float sigmaD, float sigmaR, float intensityScale)
{
	const dim3 gridSize((width + T_PER_BLOCK - 1) / T_PER_BLOCK, (height + T_PER_BLOCK - 1) / T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	cutilSafeCall(cudaMemset(d_vote, 0, sizeof(float)*width*height*MAX_NUM_LABELS_PER_SCENE));

	filterAnnotations_Kernel << <gridSize, blockSize >> >(d_outputInstance, d_inputInstance,
		d_depth, d_intensity, d_instanceToIdx, d_idxToInstance, d_vote,
		structureSize, width, height, sigmaD, sigmaR, intensityScale);

#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}


__global__ void convertInstanceToLabel_Kernel(unsigned short* d_outputLabel, const unsigned char* d_inputInstance,
	const unsigned short* d_instanceToLabel, unsigned int width, unsigned int height)
{
	const unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;
	const unsigned int y = blockIdx.y*blockDim.y + threadIdx.y;

	if (x < width && y < height) {
		d_outputLabel[y*width + x] = d_instanceToLabel[d_inputInstance[y*width + x]];
	} //in bounds of image
}

extern "C" void convertInstanceToLabel(unsigned short* d_outputLabel, const unsigned char* d_inputInstance,
	const unsigned short* d_instanceToLabel, unsigned int width, unsigned int height)
{
	const dim3 gridSize((width + T_PER_BLOCK - 1) / T_PER_BLOCK, (height + T_PER_BLOCK - 1) / T_PER_BLOCK);
	const dim3 blockSize(T_PER_BLOCK, T_PER_BLOCK);

	convertInstanceToLabel_Kernel << <gridSize, blockSize >> >(d_outputLabel, d_inputInstance,
		d_instanceToLabel, width, height);

	//TODO convert instance to label
#ifdef _DEBUG
	cutilSafeCall(cudaDeviceSynchronize());
	cutilCheckMsg(__FUNCTION__);
#endif
}

#endif // _FILTER_
