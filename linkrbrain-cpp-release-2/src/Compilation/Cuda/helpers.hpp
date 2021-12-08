#include <nvrtc.h>
#include <cuda.h>
#include <cuda_runtime_api.h>

#include <string>


#define CHECK_CUDA_ERROR(cuda_result) \
    if ((int)cuda_result != 0) { except(make_cuda_error_string(cuda_result)); }
#define CHECK_CUDA_WARNING(cuda_result, logger) \
    if ((int)cuda_result != 0) { logger.warning(make_cuda_error_string(cuda_result)); }


#define CHECK_NVRTC_ERROR(nvrtc_result) \
    if (nvrtc_result != NVRTC_SUCCESS) { except("NVRTC error:", nvrtcGetErrorString(nvrtc_result)); }
#define CHECK_NVRTC_WARNING(nvrtc_result, logger) \
    if (nvrtc_result != NVRTC_SUCCESS) { logger.warning(nvrtcGetErrorString(nvrtc_result)); }


inline const std::string make_cuda_error_string(const CUresult cuda_result) {
    const char* error_name;
    const char* error_string;
    cuGetErrorName(cuda_result, &error_name);
    cuGetErrorString(cuda_result, &error_string);
    return std::string("CUDA native error: ") + error_name + " - " + error_string;
}
inline const std::string make_cuda_error_string(const cudaError cuda_error) {
    return std::string("CUDA native error: ") + cudaGetErrorName(cuda_error) + " - " + cudaGetErrorString(cuda_error);
}
