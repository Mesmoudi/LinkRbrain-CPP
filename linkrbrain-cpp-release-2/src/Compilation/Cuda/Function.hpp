#ifndef LINKRBRAIN2019__SRC__COMPILATION__CUDA__FUNCTION_HPP
#define LINKRBRAIN2019__SRC__COMPILATION__CUDA__FUNCTION_HPP


namespace Compilation::Cuda {

    class Function : public Logging::Loggable {
    public:

        Function(const std::string& name, CUfunction kernel) :
            _name(name),
            _kernel(kernel) {}

        void call(void** arguments, const size_t grid_size_x, const size_t grid_size_y, const size_t threading_x, const size_t threading_y) {
            call(arguments, grid_size_x, grid_size_y, 1, threading_x, threading_y, 1);
        }
        void call(void** arguments, const size_t grid_size_x, const size_t threading_x) {
            call(arguments, grid_size_x, 1, 1, threading_x, 1, 1);
        }
        void call(void** arguments, const size_t grid_size_x, const size_t grid_size_y, const size_t grid_size_z, const size_t threading_x, const size_t threading_y, const size_t threading_z) {
            // get_logger().debug("Preparing to execute function `" + _name + "`");
            CHECK_CUDA_ERROR(cuLaunchKernel(
                _kernel,
                grid_size_x, grid_size_y, grid_size_z, // maximum grid values per dimensions
                threading_x, threading_y, threading_z, // number of threads per dimensions
                0, NULL, // shared memory & stream
                arguments, // actual arguments for the function
                NULL // extra arguments
            ));
            // get_logger().debug("Executed function `" + _name + "`");
            CHECK_CUDA_ERROR(cuCtxSynchronize());
            // get_logger().debug("Synchronized after executing function `" + _name + "`");
        }

    protected:

        virtual const std::string get_logger_name() {
            return "CUDA::Function[" + _name + "]";
        }

    private:

        const std::string& _name;
        CUfunction _kernel;

    };

} // Compilation::Cuda


#endif // LINKRBRAIN2019__SRC__COMPILATION__CUDA__FUNCTION_HPP
