#ifndef LINKRBRAIN2019__SRC__COMPILATION__CUDA__COMPILER_HPP
#define LINKRBRAIN2019__SRC__COMPILATION__CUDA__COMPILER_HPP


#include "Exceptions/Exception.hpp"
#include "Logging/Loggable.hpp"

#include <unordered_map>


#include "./helpers.hpp"
#include "./Program.hpp"


namespace Compilation::Cuda {

    class Compiler : public Logging::Loggable {
    public:

        Compiler() : _counter(0) {
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
            get_logger().debug("Selected device:", device_name);
            CHECK_CUDA_ERROR(cuCtxCreate(&_context, 0, _cuDevice));
            get_logger().debug("Created CUDA context");
        }
        ~Compiler() {
            CHECK_CUDA_WARNING(cuCtxDestroy(_context), get_logger());
            get_logger().debug("Destroyed CUDA context");
        }

        Program& compile(const std::string& code, const std::vector<std::string>& options={}) {
            const std::string name = std::string("program_") + std::to_string(++_counter) + ".cu";
            std::shared_ptr<Program> program;
            Program* _program = new Program(name, code, options);
            program.reset(_program);
            _programs.insert({name, program});
            return *_program;
        }

    protected:

        virtual const std::string get_logger_name() {
            return "CUDA::Compiler";
        }

    private:

        CUdevice _cuDevice;
        CUcontext _context;
        size_t _counter;
        std::unordered_map<std::string, std::shared_ptr<Program>> _programs;

    };


} // Compilation::Cuda


#endif // LINKRBRAIN2019__SRC__COMPILATION__CUDA__COMPILER_HPP
