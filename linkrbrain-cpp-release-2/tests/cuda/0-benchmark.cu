#include <stdio.h>
#include <cuda.h>


#define CHECK_CUDA_ERROR(cuda_result) \
    if ((int)cuda_result != 0) { \
        fprintf(stderr, "CUDA native error at line %d in %s : %s - %s\n", __LINE__, __FILE__, cudaGetErrorName(cuda_result), cudaGetErrorString(cuda_result)); \
    }
//
// #define CHECK_CUDA_ERROR(cuda_result) \
//     if ((int)cuda_result != 0) { \
//         const char* error_name; \
//         const char* error_string; \
//         cuGetErrorName(cuda_result, &error_name); \
//         cuGetErrorString(cuda_result, &error_string); \
//         fprintf(stderr, "CUDA native error: %s - %s", error_name, error_string); \
//     }


__global__ void CUDA_test(float a, float* in1, float* in2, float* out, size_t n) {
    size_t i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        const float d = in1[i] - in2[i];
        const float dd = (i == 0) ? 0.0 : (in1[i+1] - in1[i]);
        out[i] = a * sqrt(in1[i] + in2[i] / a) / in1[i] + in1[i] / in2[i] - a;
        out[i] += d*d + dd*dd;
        out[i] -= a / d + dd - sqrt(a);
        for (float s=0.f; s<10.f; ++s) {
            out[i] += ++s;
        }
        // printf("thread %d, block %d, index %d, value %f\n", threadIdx.x, blockIdx.x, i, out[i]);
    }
}


class Calculator {
public:

    Calculator() {
        _n = 0;
        _threads = 128;
        //  CHECK_CUDA_ERROR(cudaDeviceSetCacheConfig(cudaFuncCachePreferL1));
        //  CHECK_CUDA_ERROR(cudaDeviceSetCacheConfig(cudaFuncCachePreferShared));
        //  CHECK_CUDA_ERROR(cudaSetDeviceFlags(cudaDeviceScheduleBlockingSync | cudaDeviceMapHost));
        CHECK_CUDA_ERROR(cudaSetDeviceFlags(cudaDeviceScheduleBlockingSync));
    }
    ~Calculator() {
        deallocate();
    }

    void allocate(size_t n) {
        if (n != _n) {
            deallocate();
            _n = n;
            _size = _n * sizeof(float);
            CHECK_CUDA_ERROR(cudaMalloc((void**) &_in1, _size));
            CHECK_CUDA_ERROR(cudaMalloc((void**) &_in2, _size));
            CHECK_CUDA_ERROR(cudaMalloc((void**) &_out, _size));
        }
    }
    void deallocate() {
        if (_n != 0) {
            CHECK_CUDA_ERROR(cudaFree((void*) _in1));
            CHECK_CUDA_ERROR(cudaFree((void*) _in2));
            CHECK_CUDA_ERROR(cudaFree((void*) _out));
        }
    }

    void test(float a, float* in1, float* in2, float* out, size_t n) {
        allocate(n);
        CHECK_CUDA_ERROR(cudaMemcpy(_in1, in1, _size, cudaMemcpyHostToDevice));
        CHECK_CUDA_ERROR(cudaMemcpy(_in2, in2, _size, cudaMemcpyHostToDevice));
        size_t blocks = ceil(n / _threads);
        CUDA_test<<<blocks, _threads>>>(a, _in1, _in2, _out, n);
        CHECK_CUDA_ERROR(cudaDeviceSynchronize());
        CHECK_CUDA_ERROR(cudaMemcpy(out, _out, _size, cudaMemcpyDeviceToHost));
    }

    void set_threads(size_t threads) {
        _threads = threads;
    }

private:
    size_t _threads;
    size_t _n;
    size_t _size;
    float* _in1;
    float* _in2;
    float* _out;
};



static Calculator calculator;

void set_gpu_threads(size_t threads) {
    calculator.set_threads(threads);
}
void test_gpu(float a, float* in1, float* in2, float* out, size_t n) {
    calculator.test(a, in1, in2, out, n);
}
