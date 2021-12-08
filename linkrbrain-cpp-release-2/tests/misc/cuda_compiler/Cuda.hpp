#include "Exceptions/Exception.hpp"
#include "Logging/Loggable.hpp"

#include <nvrtc.h>
#include <cuda.h>
#include <cuda_runtime_api.h>
#include <math.h>

#include <typeinfo>
#include <vector>
#include <atomic>


inline const std::string make_cuda_error_string(const CUresult cuda_result) {
    const char* error_name;
    const char* error_string;
    cuGetErrorName(cuda_result, &error_name);
    cuGetErrorString(cuda_result, &error_string);
    return std::string("CUDA error: ") + error_name + " - " + error_string;
}

#define CHECK_CUDA_ERROR(cuda_result) \
    if (cuda_result != CUDA_SUCCESS) { make_cuda_error_string(cuda_result); }

#define CHECK_NVRTC_ERROR(nvrtc_result) \
    if (nvrtc_result != NVRTC_SUCCESS) { except("NVRTC error:", nvrtcGetErrorString(nvrtc_result)); }


namespace Computing {


    class CudaProgram : public Logging::Loggable {
    public:

        CudaProgram(const std::string& source_code, const std::string& name, const std::vector<std::string>& options={}) :
            _source_code(source_code),
            _name(name),
            _options(options),
            _nvrtc_ptx_size(0),
            _nvrtc_ptx_data(NULL)
        {
            // create program
            CHECK_NVRTC_ERROR(
                nvrtcCreateProgram(
                    &_nvrtc_program,
                    _source_code.c_str(),
                    _name.c_str(),      // name
                    0,                  // numHeaders
                    NULL,               // headers
                    NULL                // includeNames
                )
            );
            // interpret options
            int options_size = _options.size();
            const char** options_data = options_size ?
                (const char**) malloc(options_size * sizeof(char*)) : NULL;
            for (int i=0; i<options_size; ++i) {
                options_data[i] = _options[i].c_str();
            }
            // compile program
            auto result = nvrtcCompileProgram(
                _nvrtc_program,
                options_size,
                options_data
            );
            // interpret compilation log & result
            size_t compilation_log_size = 0;
            CHECK_NVRTC_ERROR(
                nvrtcGetProgramLogSize(_nvrtc_program, &compilation_log_size)
            );
            char* compilation_log_data = (char*) malloc(compilation_log_size * sizeof(char));
            CHECK_NVRTC_ERROR(
                nvrtcGetProgramLog(_nvrtc_program, compilation_log_data)
            );
            if (compilation_log_data[0]) {
                get_logger().warning("NVRTC Compilation log:", compilation_log_data);
            }
            free(compilation_log_data);
            CHECK_NVRTC_ERROR(result);
            // Obtain PTX from the program.
            CHECK_NVRTC_ERROR(nvrtcGetPTXSize(_nvrtc_program, &_nvrtc_ptx_size));
            _nvrtc_ptx_data = (char*) malloc(_nvrtc_ptx_size);
            CHECK_NVRTC_ERROR(nvrtcGetPTX(_nvrtc_program, _nvrtc_ptx_data));
            get_logger().debug("Compiled program:", _name);
        }
        ~CudaProgram() {
            free(_nvrtc_ptx_data);
            auto nvrtc_result = nvrtcDestroyProgram(&_nvrtc_program);
            if (nvrtc_result != NVRTC_SUCCESS) {
                get_logger().warning("NVRTC error:", nvrtcGetErrorString(nvrtc_result));
            }
            get_logger().debug("Destroyed program:", _name);
        }

        const std::string& get_name() const {
            return _name;
        }
        const std::string& get_source_code() const {
            return _source_code;
        }
        static std::string get_nvrtc_version() {
            int major, minor;
            CHECK_NVRTC_ERROR(nvrtcVersion(&major, &minor));
            return std::to_string(major) + "." + std::to_string(minor);
        }
        const char* get_ptx() const {
            return _nvrtc_ptx_data;
        }

    protected:

        virtual const std::string& get_logger_name() const {
            static const std::string name = "CudaProgram";
            return name;
        }

    private:

        const std::string _name;
        const std::string _source_code;
        const std::vector<std::string> _options;
        nvrtcProgram _nvrtc_program;
        size_t _nvrtc_ptx_size;
        char* _nvrtc_ptx_data;

    };


    class CudaFunction : public Logging::Loggable {
    public:

        enum Cardinality {
            Scalar,
            InputVector,
            OutputVector
        };
        struct Argument {
            Cardinality cardinality;
            size_t type_hash;
            size_t type_size;
            size_t full_type_hash;
            std::string name;
        };
        struct CudaArguments {
            size_t current_index;
            std::vector<void*> pointers;
            std::vector<void*> dynamic_pointers;
            std::vector<CUdeviceptr> dynamic_cuda_pointers;
            std::vector<std::tuple<CUdeviceptr, void*, size_t>> output_cuda_pointers;
            CudaArguments() : current_index(0) {}
            void close() {
                for (auto& [cuda_pointer, pointer, size] : output_cuda_pointers) {
                    CHECK_CUDA_ERROR(cuMemcpyDtoH(pointer, cuda_pointer, size));
                }
                for (void* dynamic_pointer : dynamic_pointers) {
                    free(dynamic_pointer);
                }
                for (CUdeviceptr& dynamic_cuda_pointer : dynamic_cuda_pointers) {
                    CHECK_CUDA_ERROR(cuMemFree(dynamic_cuda_pointer));
                }
            }
        };

        CudaFunction(CudaFunction&& source) :
            _moved(false),
            _compiled(source._compiled),
            _identifier(source._identifier),
            _code_body(source._code_body),
            _arguments(source._arguments),
            _program(source._program),
            _module(source._module),
            _kernel(source._kernel),
            _threads_count(128)
        {
            source._moved = true;
            get_logger().debug("Used move operator for function `" + _identifier + "`");
        }
        CudaFunction(const std::string& identifier) :
            _moved(false),
            _compiled(false),
            _identifier(identifier)
        {
            get_logger().debug("Instanciated function `" + _identifier + "`");
        }
        ~CudaFunction() {
            if (!_moved && _compiled) {
                CUresult cuda_result = cuModuleUnload(_module);
                if (cuda_result != CUDA_SUCCESS) {
                    get_logger().warning("Error while destroying Cuda function:", make_cuda_error_string(cuda_result));
                }
                get_logger().debug("Destroyed function `" + _identifier + "`");
            }
        }

        static const std::string& get_type_name(const size_t type_hash) {
            static bool computed = false;
            static std::unordered_map<size_t, std::string> names;
            if (!computed) {
                #define CUDAFUNCTION_GETTYPENAME_INSERT(TYPE) \
                    names[typeid(TYPE).hash_code()] = #TYPE; \
                    names[typeid(std::vector<TYPE>).hash_code()] = "std::vector<" #TYPE ">";
                CUDAFUNCTION_GETTYPENAME_INSERT(float)
                CUDAFUNCTION_GETTYPENAME_INSERT(double)
                CUDAFUNCTION_GETTYPENAME_INSERT(long double)
                CUDAFUNCTION_GETTYPENAME_INSERT(int8_t)
                CUDAFUNCTION_GETTYPENAME_INSERT(uint8_t)
                CUDAFUNCTION_GETTYPENAME_INSERT(int16_t)
                CUDAFUNCTION_GETTYPENAME_INSERT(uint16_t)
                CUDAFUNCTION_GETTYPENAME_INSERT(int32_t)
                CUDAFUNCTION_GETTYPENAME_INSERT(uint32_t)
                CUDAFUNCTION_GETTYPENAME_INSERT(int64_t)
                CUDAFUNCTION_GETTYPENAME_INSERT(uint64_t)
                #undef CUDAFUNCTION_GETTYPENAME_INSERT
            }
            const auto it = names.find(type_hash);
            if (it != names.end()) {
                return it->second;
            }
            except("Unrecognized type hash:", type_hash);
        }

        template <typename T>
        CudaFunction& add_argument(const std::string& name) {
            return add_argument<T>(Cardinality::Scalar, name);
        }
        template <typename T>
        CudaFunction& add_argument(const Cardinality cardinality, const std::string& name) {
            _arguments.push_back({
                .cardinality = cardinality,
                .type_hash = typeid(T).hash_code(),
                .type_size = sizeof(T),
                .full_type_hash = (cardinality == Scalar) ? typeid(T).hash_code() : typeid(std::vector<T>).hash_code(),
                .name = name,
            });
            return *this;
        }
        CudaFunction& set_body(const std::string& code) {
            _code_body = code;
            return *this;
        }

        inline const std::string generate_code() {
            std::string code = "extern \"C\" __global__ void f(size_t* i_max, ";
            bool first = true;
            for (auto& argument : _arguments) {
                if (first) {
                    first = false;
                } else {
                    code += ", ";
                }
                switch (argument.cardinality) {
                    case Scalar:
                        code += get_type_name(argument.type_hash) + " " + argument.name;
                        break;
                    case InputVector:
                    case OutputVector:
                        code += get_type_name(argument.type_hash) + "* " + argument.name + "_data";
                        code += ", size_t* " + argument.name + "_size";
                        break;
                }
            }
            code += ") {\n";
            code += "const size_t i = blockIdx.x * blockDim.x + threadIdx.x;\n";
            // code += "if (i >= i_max) { return; }";
            code += _code_body;
            code += "\n}";
            get_logger().debug("Generated code for function `" + _identifier + "`:\n\n" + code);
            return code;
        }
        inline void compile(const std::vector<std::string>& options={}) {
            if (_compiled) {
                return;
            }
            _program.reset(
                new CudaProgram(
                    generate_code(),
                    _identifier + ".cu",
                    options
                )
            );
            CHECK_CUDA_ERROR(cuModuleLoadDataEx(&_module, _program->get_ptx(), 0, 0, 0));
            CHECK_CUDA_ERROR(cuModuleGetFunction(&_kernel, _module, "f"));
            get_logger().debug("Extracted module & kernel for function `" + _identifier + "`");
            _compiled = true;
        }

        template <typename ...Types>
        inline void run(size_t i_max, Types& ... Args) {
            CudaArguments cuda_arguments;
            cuda_arguments.pointers.push_back(&i_max);
            parse_arguments(cuda_arguments, Args...);
            get_logger().debug("Parsed", cuda_arguments.pointers.size(), "arguments for function `" + _identifier + "`");
            CHECK_CUDA_ERROR(cuLaunchKernel(_kernel,
                ceil(i_max / _threads_count), 1, 1,    // grid dim
                _threads_count, 1, 1,   // block dim
                0, NULL,             // shared mem and stream
                cuda_arguments.pointers.data(), 0
            ));
            CHECK_CUDA_ERROR(cuCtxSynchronize());
            cuda_arguments.close();
        }
        template <typename ...Types>
        inline void operator() (size_t i_max, Types& ... Args) {
            run(i_max, Args...);
        }

    private:

        template <typename T>
        void parse_arguments(CudaArguments& cuda_arguments, T& arg) {
            const Argument& argument = _arguments[cuda_arguments.current_index];
            // check argument type
            if (typeid(T).hash_code() != argument.full_type_hash) {
                except("Wrong type for argument #" +
                    std::to_string(cuda_arguments.current_index + 1) +
                    " when calling function `" +
                    _identifier +
                    ": expected `" + get_type_name(typeid(T).hash_code()) + "`" +
                    ", got `" + get_type_name(argument.full_type_hash) + "`"
                );
            }
            // integrate pointer
            switch (argument.cardinality) {
                case Scalar: {
                    cuda_arguments.pointers.push_back(&arg);
                } break;
                case InputVector: {
                    size_t* size = (size_t*) malloc(sizeof(size_t));
                    *size = arg.size();
                    cuda_arguments.pointers.push_back(&arg);
                    cuda_arguments.dynamic_pointers.push_back(size);
                    const size_t full_size = *size * argument.type_size;
                    CUdeviceptr cuda_pointer;
                    CHECK_CUDA_ERROR(cuMemAlloc(&cuda_pointer, full_size));
                    CHECK_CUDA_ERROR(cuMemcpyHtoD(cuda_pointer, ((std::vector<int>&) arg).data(), full_size));
                    cuda_arguments.dynamic_cuda_pointers.push_back(cuda_pointer);
                } break;
                case OutputVector: {
                    size_t* size = (size_t*) malloc(sizeof(size_t));
                    *size = arg.size();
                    cuda_arguments.pointers.push_back(&arg);
                    cuda_arguments.dynamic_pointers.push_back(size);
                    const size_t full_size = *size * argument.type_size;
                    CUdeviceptr cuda_pointer;
                    CHECK_CUDA_ERROR(cuMemAlloc(&cuda_pointer, full_size));
                    CHECK_CUDA_ERROR(cuMemcpyHtoD(cuda_pointer, ((std::vector<int>&) arg).data(), full_size));
                    cuda_arguments.dynamic_cuda_pointers.push_back(cuda_pointer);
                    cuda_arguments.output_cuda_pointers.push_back({cuda_pointer, &(arg[0]), full_size});
                } break;
            }
        }
        template <typename T, typename ...Types>
        void parse_arguments(CudaArguments& cuda_arguments, T& arg, Types& ... Args) {
            parse_arguments(cuda_arguments, arg);
            ++cuda_arguments.current_index;
            parse_arguments(cuda_arguments, Args...);
        }

        bool _moved;
        bool _compiled;
        const std::string _identifier;
        std::string _code_body;
        std::vector<Argument> _arguments;
        std::shared_ptr<CudaProgram> _program;
        CUmodule _module;
        CUfunction _kernel;
        size_t _threads_count;

    };


    class Cuda : public Logging::Loggable {
    public:

        Cuda() {
            CHECK_CUDA_ERROR(cuInit(0));
            CHECK_CUDA_ERROR(cuDeviceGet(&_cuDevice, 0));
            CHECK_CUDA_ERROR(cuCtxCreate(&_context, 0, _cuDevice));
        }
        ~Cuda() {
            _functions.clear();
            CUresult cuda_result = cuCtxDestroy(_context);
            if (cuda_result != CUDA_SUCCESS) {
                get_logger().warning("Error while destroying Cuda module:", make_cuda_error_string(cuda_result));
            }
            get_logger().debug("Destroyed Cuda");
        }

        CudaFunction& create_function(const std::string& identifier) {
            const size_t i = _functions.size();
            _functions.push_back({identifier});
            return _functions[i];
        }

    protected:

        virtual const std::string& get_logger_name() const {
            static const std::string name = "Cuda";
            return name;
        }

    private:

        CUdevice _cuDevice;
        CUcontext _context;
        std::vector<CudaFunction> _functions;

    };


} // Computing
