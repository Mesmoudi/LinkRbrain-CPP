#include "LinkRbrain/Models/Point.hpp"


#define CUDA_ARRAY3D_X (origin.x + resolution * blockIdx.x)
#define CUDA_ARRAY3D_Y (origin.y + resolution * blockIdx.y)
#define CUDA_ARRAY3D_Z (origin.z + resolution * blockIdx.z)
#define CUDA_ARRAY3D_INDEX (blockIdx.x * gridDim.y * gridDim.z + blockIdx.y * gridDim.z + blockIdx.z)


template <typename T>
class Array3D : public Logging::Loggable {
public:

    template <typename T2>
    Array3D(const LinkRbrain::Models::TemplatedExtrema<T2>& extrema, const T resolution) :
        _x0(resolution * round(extrema.min.x / resolution)),
        _y0(resolution * round(extrema.min.y / resolution)),
        _z0(resolution * round(extrema.min.z / resolution)),
        _x_size(std::ceil(extrema.max.x - extrema.min.x)),
        _y_size(std::ceil(extrema.max.y - extrema.min.y)),
        _z_size(std::ceil(extrema.max.z - extrema.min.z)),
        _x_factor(_y_size * _z_size),
        _y_factor(_z_size),
        _size(_x_size * _y_size * _z_size),
        _data_size(_size * sizeof(T)),
        _cpu_data(NULL),
        _gpu_data(0),
        _resolution(resolution)
    {
        allocate_cpu();
    }
    ~Array3D() {
        deallocate_cpu();
        deallocate_gpu();
    }

    const float get_x_factor() const {
        return _x_factor;
    }
    const float get_y_factor() const {
        return _y_factor;
    }
    const size_t& get_size() const {
        return _size;
    }
    const size_t& get_x_size() const {
        return _x_size;
    }
    const size_t& get_y_size() const {
        return _y_size;
    }
    const size_t& get_z_size() const {
        return _z_size;
    }
    const float get_x0() const {
        return _x0;
    }
    const float get_y0() const {
        return _y0;
    }
    const float get_z0() const {
        return _z0;
    }

    void deallocate_cpu() {
        if (_cpu_data != NULL) {
            free(_cpu_data);
            _cpu_data = NULL;
        }
    }
    void deallocate_gpu() {
        if (_gpu_data != 0) {
            CHECK_CUDA_WARNING(cuMemFree(_gpu_data), get_logger());
            _gpu_data = 0;
        }
    }
    void allocate_cpu() {
        deallocate_cpu();
        _cpu_data = (T*) malloc(_data_size);
        if (_cpu_data == NULL) {
            except("Could not allocate", _data_size, "bytes for Array3D");
        }
    }
    void allocate_gpu() {
        deallocate_gpu();
        CHECK_CUDA_ERROR(cuMemAlloc(&_gpu_data, _data_size));
    }

    inline const size_t compute_index(const T& x, const T& y, const T& z) const {
        return
            round((x - _x0) / _resolution) * _x_factor +
            round((y - _y0) / _resolution) * _y_factor +
            round((z - _z0) / _resolution);
    }
    inline const LinkRbrain::Models::TemplatedWeightedPoint<T> compute_point(size_t index) const {
        const T Z = index % _z_size;
        index /= _z_size;
        const T Y = index % _y_size;
        index /= _y_size;
        const T X = index;
        return {
            _x0 + X * _resolution,
            _y0 + Y * _resolution,
            _z0 + Z * _resolution,
        };
    }

    inline const T& get_cpu_value(const T& x, const T& y, const T& z) const {
        return _cpu_data[compute_index(x, y, z)];
    }
    inline void set_cpu_value(const size_t& x, const size_t& y, const size_t& z, const T& value) {
        return _cpu_data[compute_index(x, y, z)] = value;
    }

    inline void clear_cpu_data() {
        memset(_cpu_data, 0, _data_size);
    }
    inline void clear_gpu_data() {
        CHECK_CUDA_ERROR(cuMemsetD8(_gpu_data, 0, _data_size));
    }

    inline CUdeviceptr& get_gpu_data() {
        return _gpu_data;
    }
    inline T* get_cpu_data() {
        return _cpu_data;
    }

    inline void cpu2gpu() {
        CHECK_CUDA_ERROR(cuMemcpyHtoD(_gpu_data, _cpu_data, _data_size));
    }
    inline void gpu2cpu() {
        CHECK_CUDA_ERROR(cuMemcpyDtoH(_cpu_data, _gpu_data, _data_size));
    }

protected:

    inline const std::string& get_logger_name() const {
        static const std::string name = "Array3D";
        return name;
    }

private:

    size_t _x_size, _y_size, _z_size;
    size_t _size;
    size_t _data_size;
    T* _cpu_data;
    CUdeviceptr _gpu_data;
    T _resolution;
    T _x0, _y0, _z0;
    T _x_factor, _y_factor;
};
