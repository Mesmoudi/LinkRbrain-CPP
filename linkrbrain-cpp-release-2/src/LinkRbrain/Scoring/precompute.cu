#include <vector>
#include <iostream>

namespace Types {
    #pragma pack(push,1)
    template <typename T>
    struct Point {
        T x, y, z, weight;
    };
    #pragma pack(pop)
}


#include <cuda.h>

template <typename T>
__global__ void _score_precompute_sphere(T* _precomputed, const Types::Point<T> p1, const size_t _groups_count, const Types::Point<T>* _groups_points, const size_t* _groups_offsets, const double diameter) {
    size_t i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= _groups_count) return;
    size_t offset_min = _groups_offsets[i];
    size_t offset_max = _groups_offsets[i + 1];
    T score = 0.0;
    for (size_t offset=offset_min; offset<offset_max; ++offset) {
        Types::Point<T> p2 = _groups_points[offset];
        // const T dx = p1.x - p2.x;
        // const T dy = p1.y - p2.y;
        // const T dz = p1.z - p2.z;
        // const T distance = sqrt(dx*dx + dy*dy + dz*dz);
        // const T distance = norm3d(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z);
        // const T x = distance / diameter;
        const T x = norm3d(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z) / diameter;
        if (x < 1.0) {
            score += sqrt(p1.weight * p2.weight) * (0.5*x * (x*x - 3.0) + 1.0);
        }
    }
    _precomputed[i] = score;
}


#define CHECK_CUDA_ERROR(cuda_result) \
    if ((int)cuda_result != 0) { \
        std::cerr << "CUDA native error at line " << __LINE__ << " in " << __FILE__ << " : " << cudaGetErrorName(cuda_result) << ", " << cudaGetErrorString(cuda_result) << "\n"; \
        exit(1); \
    }


template <typename T>
class Precomputer {
public:

    Precomputer(const int device, const size_t threads) :
        _device(device),
        _threads(threads),
        _groups_count(0),
        _groups_points(NULL),
        _groups_offsets(NULL),
        _precomputed(NULL)
    {
        int count;
        CHECK_CUDA_ERROR(cudaGetDeviceCount(&count));
        CHECK_CUDA_ERROR(cudaSetDevice(_device));
        CHECK_CUDA_ERROR(cudaSetDeviceFlags(cudaDeviceScheduleBlockingSync));
        CHECK_CUDA_ERROR(cudaGetDeviceProperties(&_gpu_properties, _device));
        if (threads > _gpu_properties.maxThreadsPerBlock) {
            _threads = _gpu_properties.maxThreadsPerBlock;
        }
        std::cout << count << " device(s)\n";
        std::cout << _gpu_properties.name << " selected\n";
        std::cout << _gpu_properties.totalGlobalMem << " bytes available (global memory)\n";
        std::cout << _gpu_properties.totalConstMem << " bytes available (constant memory)\n";
        std::cout << _gpu_properties.maxThreadsPerBlock << " threads maximum\n";
        std::cout << _gpu_properties.multiProcessorCount << " multiprocessors\n";
    }
    ~Precomputer() {
        deallocate();
    }

    void deallocate() {
        if (_groups_points != NULL) {
            CHECK_CUDA_ERROR(cudaFree((void*) _groups_points));
            _groups_points = NULL;
        }
        if (_groups_offsets != NULL) {
            CHECK_CUDA_ERROR(cudaFree((void*) _groups_offsets));
            _groups_offsets = NULL;
        }
        if (_precomputed != NULL) {
            CHECK_CUDA_ERROR(cudaFree((void*) _precomputed));
            _precomputed = NULL;
        }
    }

    void set_threads(const size_t threads) {
        _threads = threads;
        _blocks = ceil(_groups_count / _threads);
    }

    void set_groups(const size_t groups_count, const std::vector<Types::Point<T>>& groups_points, const std::vector<size_t>& groups_offsets) {
        deallocate();
        _groups_count = groups_count;
        _blocks = ceil(groups_count / _threads);
        CHECK_CUDA_ERROR(cudaMalloc((void**) &_groups_points, groups_points.size() * sizeof(Types::Point<T>)));
        CHECK_CUDA_ERROR(cudaMemcpy(_groups_points, groups_points.data(), groups_points.size() * sizeof(Types::Point<T>), cudaMemcpyHostToDevice));
        CHECK_CUDA_ERROR(cudaMalloc((void**) &_groups_offsets, groups_offsets.size() * sizeof(size_t)));
        CHECK_CUDA_ERROR(cudaMemcpy(_groups_offsets, groups_offsets.data(), groups_offsets.size() * sizeof(size_t), cudaMemcpyHostToDevice));
        CHECK_CUDA_ERROR(cudaMalloc((void**) &_precomputed, groups_count * sizeof(T)));
    }

    void precompute_sphere(std::vector<T>& precomputed, const Types::Point<T> center, const double diameter) {
        _score_precompute_sphere<T><<<_blocks, _threads>>>(
            _precomputed,
            center,
            _groups_count,
            _groups_points,
            _groups_offsets,
            diameter
        );
        CHECK_CUDA_ERROR(cudaDeviceSynchronize());
        CHECK_CUDA_ERROR(cudaMemcpy(&(precomputed[0]), _precomputed, _groups_count * sizeof(T), cudaMemcpyDeviceToHost));
    }

private:

    const int _device;
    cudaDeviceProp _gpu_properties;
    size_t _blocks;
    size_t _threads;
    //
    T* _precomputed;
    size_t _groups_count;
    Types::Point<T>* _groups_points;
    size_t* _groups_offsets;
};

template <typename T>
static Precomputer<T>* precomputer;

void CUDA_precomputing_start(const int device, const size_t threads) {
    precomputer<double> = new Precomputer<double>(device, threads);
}
void CUDA_precomputing_set_groups(const size_t groups_count, const std::vector<Types::Point<double>>& groups_points, const std::vector<size_t>& groups_offsets) {
    precomputer<double>->set_groups(groups_count, groups_points, groups_offsets);
}
void CUDA_precomputing_precompute_sphere(std::vector<double>& precomputed, const Types::Point<double> center, const double diameter) {
    precomputer<double>->precompute_sphere(precomputed, center, diameter);
}
void CUDA_precomputing_finish() {
    delete precomputer<double>;
}
