#ifndef LINKRBRAIN2019__SRC__COMPILATION__CUDA__PROGRAM_HPP
#define LINKRBRAIN2019__SRC__COMPILATION__CUDA__PROGRAM_HPP


#include "./Function.hpp"


namespace Compilation::Cuda {

    class Program : public Logging::Loggable {
    public:

        Program(const std::string& name, const std::string& source_code, const std::vector<std::string>& options={}) :
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
            // Obtain PTX & module from the program.
            CHECK_NVRTC_ERROR(nvrtcGetPTXSize(_nvrtc_program, &_nvrtc_ptx_size));
            _nvrtc_ptx_data = (char*) malloc(_nvrtc_ptx_size);
            CHECK_NVRTC_ERROR(nvrtcGetPTX(_nvrtc_program, _nvrtc_ptx_data));
            CHECK_CUDA_ERROR(cuModuleLoadDataEx(&_cuda_module, _nvrtc_ptx_data, 0, 0, 0));
            // The end!
            CHECK_NVRTC_ERROR(nvrtcDestroyProgram(&_nvrtc_program));
            get_logger().debug("Compiled program:", _name);
        }
        ~Program() {
            free(_nvrtc_ptx_data);
        }

        const std::string& get_name() const {
            return _name;
        }
        const std::string& get_source_code() const {
            return _source_code;
        }

        Function& get_function(const std::string& name) {
            auto it = _functions.find(name);
            if (it != _functions.end()) {
                return * it->second.get();
            }
            //
            CUfunction kernel;
            CHECK_CUDA_ERROR(cuModuleGetFunction(&kernel, _cuda_module, name.c_str()));
            std::shared_ptr<Function> function;
            Function* _function = new Function(name, kernel);
            function.reset(_function);
            _functions.insert({name, function});
            return *_function;
        }

    protected:

        virtual const std::string get_logger_name() {
            return "CUDA::Program[" + _name + "]";
        }

    private:

        const std::string _name;
        const std::string _source_code;
        const std::vector<std::string> _options;
        nvrtcProgram _nvrtc_program;
        size_t _nvrtc_ptx_size;
        char* _nvrtc_ptx_data;
        CUmodule _cuda_module;
        std::unordered_map<std::string, std::shared_ptr<Function>> _functions;

    };

} // Compilation::Cuda


#endif // LINKRBRAIN2019__SRC__COMPILATION__CUDA__PROGRAM_HPP
