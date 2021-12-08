#include "Exceptions/Exception.hpp"
#include "Logging/Loggable.hpp"

#include <cuda.h>
#include <cuda_runtime_api.h>


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

#define CHECK_CUDA_ERROR(cuda_result) \
    if ((int)cuda_result != 0) { except(make_cuda_error_string(cuda_result)); }
#define CHECK_CUDA_WARNING(cuda_result, logger) \
    if ((int)cuda_result != 0) { logger.warning(make_cuda_error_string(cuda_result)); }


class Cuda : public Logging::Loggable {
public:

    Cuda() {
        CHECK_CUDA_ERROR(cuInit(0));
        get_logger().debug("Successfully initialized CUDA");
        int devices_count;
        CHECK_CUDA_ERROR(cuDeviceGetCount(&devices_count));
        get_logger().debug("Found", devices_count, "CUDA-capable device(s)");
        CHECK_CUDA_ERROR(cuDeviceGet(&_cuDevice, 0));
        char device_name[1024];
        CHECK_CUDA_ERROR(cuDeviceGetName(device_name, 1024, _cuDevice));
        int max_block_dim_x, max_block_dim_y, max_block_dim_z;
        CHECK_CUDA_ERROR(cuDeviceGetAttribute(&max_block_dim_x, CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X, _cuDevice));
        CHECK_CUDA_ERROR(cuDeviceGetAttribute(&max_block_dim_y, CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y, _cuDevice));
        CHECK_CUDA_ERROR(cuDeviceGetAttribute(&max_block_dim_z, CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z, _cuDevice));
        get_logger().debug("Maximum block size for device: ", max_block_dim_x, "x", max_block_dim_y, "x", max_block_dim_z);
        int max_grid_dim_x, max_grid_dim_y, max_grid_dim_z;
        CHECK_CUDA_ERROR(cuDeviceGetAttribute(&max_grid_dim_x, CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X, _cuDevice));
        CHECK_CUDA_ERROR(cuDeviceGetAttribute(&max_grid_dim_y, CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y, _cuDevice));
        CHECK_CUDA_ERROR(cuDeviceGetAttribute(&max_grid_dim_z, CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Z, _cuDevice));
        get_logger().debug("Maximum grid size for device: ", max_grid_dim_x, "x", max_grid_dim_y, "x", max_grid_dim_z);
        get_logger().notice("Selected device:", device_name);
        CHECK_CUDA_ERROR(cuCtxCreate(&_context, 0, _cuDevice));
        get_logger().debug("Created CUDA context");
    }
    ~Cuda() {
        CHECK_CUDA_WARNING(cuCtxDestroy(_context), get_logger());
        get_logger().debug("Destroyed CUDA context");
    }

protected:

    const std::string& get_logger_name() const {
        static const std::string name = "Cuda";
        return name;
    }

private:

    CUdevice _cuDevice;
    CUcontext _context;

};
