#ifndef GLOBAL_DEFINES_H
#define GLOBAL_DEFINES_H


#define MLIB_CUDA_SAFE_CALL(b) { if(b != cudaSuccess) throw MLIB_EXCEPTION(std::string(cudaGetErrorString(b)) + ":" + std::string(__FUNCTION__)); }
#define MLIB_CUDA_SAFE_FREE(b) { if(!b) { MLIB_CUDA_SAFE_CALL(cudaFree(b)); b = NULL; } }
#define MLIB_CUDA_CHECK_ERR(msg) {  cudaError_t err = cudaGetLastError();	if (err != cudaSuccess) { throw MLIB_EXCEPTION(cudaGetErrorString( err )); } }




#define MAX_NUM_LABELS_PER_SCENE 80 //current max = 76

#endif //GLOBAL_DEFINES_H