#include "Compilation/Cuda/Compiler.hpp"

#include "Types/Array3D.hpp"

#include "Exceptions/Exception.hpp"
#include "Logging/Loggers.hpp"
auto& logger = Logging::get_logger();


void debug(float a, float *x, float *y, float *out, size_t n, size_t d) {
    for (size_t i = 0; i < d; ++i) {
      std::cout << a << " * " << x[i] << " + " << y[i]
                << " = " << out[i] << '\n';
    }
    for (size_t i = n-d; i < n; ++i) {
      std::cout << a << " * " << x[i] << " + " << y[i]
                << " = " << out[i] << '\n';
    }
    logger.debug("Showed results");
}
void saxpy_cpu(float a, float *x, float *y, float *out, size_t n) {
    for (size_t tid=0; tid<n; ++tid) {
        out[tid] = a * x[tid] + y[tid];
    }
}

const std::string function_name = "saxpy";
const std::string code = R"###(
    extern "C" __global__
    void saxpy(float a, float *x, float *y, float *out, size_t n)
    {
      size_t tid = blockIdx.x * blockDim.x + threadIdx.x;
      if (tid < n) {
        out[tid] = a * x[tid] + y[tid];
      }
    }

    template <typename T>
    struct Point {
        T x, y, z, weight;
    };

    extern "C" __global__
    void project_points_float(
        float* density_data,
        Point<float>* points_data, int points_count,
        float cx, float cy, float cz,
        int i_size, int j_size, int k_size,
        float diameter, float resolution
    ) {
        Point<float> point = points_data[(blockIdx.x * blockDim.x) + threadIdx.x];
        //
        int i_min = ceil(point.x - diameter) / resolution;
        if (i_min < 0) i_min = 0;
        int i_max = ceil(point.x + diameter) / resolution;
        if (i_max >= i_size) i_min = i_size - 1;
        //
        int i_min = ceil(point.x - diameter) / resolution;
        if (i_min < 0) i_min = 0;
        int i_max = ceil(point.x + diameter) / resolution;
        if (i_max >= i_size) i_min = i_size - 1;
        //
        int i_min = ceil(point.x - diameter) / resolution;
        if (i_min < 0) i_min = 0;
        int i_max = ceil(point.x + diameter) / resolution;
        if (i_max >= i_size) i_min = i_size - 1;
    }

    extern "C" __global__
    void project_float(
        float* density, int n,
        int d_i, int d_j, int d_k,
        int k_i, int k_j,
        float d_x, float d_y, float d_z,
        float resolution, float diameter, float weight
    ) {
        const int i = (blockIdx.x * blockDim.x) + threadIdx.x;
        const int j = (blockIdx.y * blockDim.y) + threadIdx.y;
        const int k = (blockIdx.z * blockDim.z) + threadIdx.z;
        const int index =
            (i + d_i) * k_i +
            (j + d_j) * k_j +
            (k + d_k);
        if (index < 0 || index >= n) {
            return;
        }
        // printf("blockIdx  %6d%6d%6d\nblockDim  %6d%6d%6d\nthreadIdx %6d%6d%6d\nindexes   %6d%6d%6d   ->%6d / %-6d\n\n",
        //     blockIdx.x, blockIdx.y, blockIdx.z,
        //     blockDim.x, blockDim.y, blockDim.z,
        //     threadIdx.x, threadIdx.y, threadIdx.z,
        //     i, j, k, index, n
        // );
        // return;
        const float dx = resolution * (i + d_i) + d_x;
        const float dy = resolution * (j + d_j) + d_y;
        const float dz = resolution * (k + d_k) + d_z;
        // printf("|  i = %-6d  |  x : %-6d  |  y : %-6d  |  z : %-6d  |  \n", i, x, y, z);
        const float distance = sqrt(dx*dx + dy*dy + dz*dz);
        float score = 0.;
        if (distance < diameter) {
            const float k = distance / diameter;
            score = weight * (0.5*k * (k*k - 3.0) + 1.0);
            density[i] += score;
            // printf("|  i = %-6d  |  x : %-6d , %6.2f  |  y : %-6d , %6.2f  |  z : %-6d , %6.2f  |  %6.2f  |  %6.2f  \n", index, i, dx, j, dy, k, dz, distance, score);
            // printf("|  index = %-6d  |  %6.2f    |  %6.2f  \n", index, distance, score);
            printf("|  index = %4d |  dx, dy, dz = %5.2f, %5.2f, %5.2f | distance = %6.2f | score = %6.2f |\n",
                index,
                dx, dy, dz,
                distance, score
            );
            // printf("|  index = %-4d  |  i, j, k = %3d, %3d, %3d  |  dx, dy, dz = %5.2f, %5.2f, %5.2f | distance = %6.2f | score = %6.2f\n",
            //     index,
            //     i, j, k,
            //     d_x, d_y, d_z,
            //     distance, score);
        }
    }

)###";
const std::vector<std::string> options = {};


using namespace Compilation::Cuda;


int main(int argc, char const *argv[]) {

    Compiler compiler;
    logger.notice("Initialized CUDA compiler");
    Program& program = compiler.compile(code, options);
    logger.notice("Compiled CUDA program");
    Function& saxpy_gpu = program.get_function(function_name);
    logger.notice("Extracted CUDA saxpy function");
    Function& project_float = program.get_function("project_float");
    // Function& project_double = program.get_function("project_double");
    logger.notice("Extracted CUDA project function");
    logger.message("CUDA function is ready");

    // Theoretical use: saxpy
    {
        size_t NUM_THREADS = 1<<7;
        size_t NUM_BLOCKS = 8000;
        size_t n = NUM_THREADS * NUM_BLOCKS;
        size_t bufferSize = n * sizeof(float);
        float a = 5.1f;
        float *hX = new float[n], *hY = new float[n], *hOut = new float[n];
        for (size_t i = 0; i < n; ++i) {
          hX[i] = static_cast<float>(i);
          hY[i] = static_cast<float>(i * 2);
        }
        CUdeviceptr dX, dY, dOut;
        CHECK_CUDA_ERROR(cuMemAlloc(&dX, bufferSize));
        CHECK_CUDA_ERROR(cuMemAlloc(&dY, bufferSize));
        CHECK_CUDA_ERROR(cuMemAlloc(&dOut, bufferSize));
        CHECK_CUDA_ERROR(cuMemcpyHtoD(dX, hX, bufferSize));
        CHECK_CUDA_ERROR(cuMemcpyHtoD(dY, hY, bufferSize));
        void* arguments[] = {&a, &dX, &dY, &dOut, &n};
        saxpy_gpu.call(arguments, NUM_BLOCKS, NUM_THREADS);
        logger.notice("Executed GPU function");
        CHECK_CUDA_ERROR(cuMemsetD8(dOut, 0, bufferSize));
        logger.debug("Cleared GPU data");
        saxpy_gpu.call(arguments, NUM_BLOCKS, NUM_THREADS);
        logger.notice("Executed GPU function");
        CHECK_CUDA_ERROR(cuMemcpyDtoH(hOut, dOut, bufferSize));
        logger.debug("Copied data from device");
        CHECK_CUDA_ERROR(cuMemsetD8(dOut, 0, bufferSize));
        logger.debug("Cleared GPU data");
        saxpy_gpu.call(arguments, NUM_BLOCKS, NUM_THREADS);
        logger.notice("Executed GPU function");
        CHECK_CUDA_ERROR(cuMemcpyDtoH(hOut, dOut, bufferSize));
        logger.debug("Copied data from device");
        debug(a, hX, hY, hOut, n, 5);
        saxpy_cpu(a, hX, hY, hOut, n);
        logger.notice("Executed CPU function");
        debug(a, hX, hY, hOut, n, 5);
        // Release resources.
        CHECK_CUDA_ERROR(cuMemFree(dX));
        CHECK_CUDA_ERROR(cuMemFree(dY));
        CHECK_CUDA_ERROR(cuMemFree(dOut));
    }

    // Abstract application
    {
        Types::Point<float> center(0, 0.5, 2.0);
        float resolution = 2.0;
        float diameter = 4.0;
        float weight = 1.0;
        Types::PointExtrema<float> extrema;
        extrema.min.x = -10.2;
        extrema.min.y = -3;
        extrema.min.z = -4;
        extrema.max.x = +7.2;
        extrema.max.y = +5;
        extrema.max.z = +8;
        Types::Array3D<float> density(extrema, 2.);
        logger.debug("Allocated density, has", density.get_size(), "points for extrema", extrema);
        density.allocate_gpu();
        logger.debug("Allocated density (GPU-side)");
        size_t n = density.get_size();
        int k_i = density.get_x_factor();
        int k_j = density.get_y_factor();
        Types::Point<float> origin = {
            density.get_x0() + resolution * round((center.x - diameter - density.get_x0()) / resolution),
            density.get_y0() + resolution * round((center.y - diameter - density.get_y0()) / resolution),
            density.get_z0() + resolution * round((center.z - diameter - density.get_z0()) / resolution)
        };
        logger.debug("center =", center);
        logger.debug("origin =", origin);
        const size_t size = 2.0 * diameter / resolution + 1;
        int d_i = (origin.x - density.get_x0()) / resolution;
        int d_j = (origin.y - density.get_y0()) / resolution;
        int d_k = (origin.z - density.get_z0()) / resolution;
        std::cout << "di, dj, dk = " << d_i << ", " << d_j << ", " << d_k << '\n';
        float d_x = density.get_x0() - center.x;
        float d_y = density.get_y0() - center.y;
        float d_z = density.get_z0() - center.z;
        std::cout << "dx, dy, dz = " << d_x << ", " << d_y << ", " << d_z << '\n';
        void* density_arguments[] = {
            & density.get_gpu_data(),
            & n,
            & d_i,
            & d_j,
            & d_k,
            & k_i,
            & k_j,
            & d_x,
            & d_y,
            & d_z,
            & resolution,
            & diameter,
            & weight
        };
        project_float.call(
            density_arguments,
            size, size, size,
            1, 1, 1
        );
        logger.notice("Computed density map using GPU");
        density.gpu2cpu();
        logger.notice("Copied density map data to CPU");
        for (int i=0, n=density.get_size(); i<n; ++i) {
            auto point = density.compute_point(i);
            int index = density.compute_index(point.x, point.y, point.z);
            if (i != index) {
                except("Index mismatch:", i, "!=", index, "; (x, y, z) =", point.x, ",", point.y, ",", point.z);
            }
            float value = density.get_cpu_data()[i];
            if (value == 0.) {
                continue;
            }
            // if (i != 1) {
            //     continue;
            // }
            std::cout
                << "i = " << i << " | "
                << "\t x = " << point.x
                << "\t y = " << point.y
                << "\t z = " << point.z
                << "\t\t value = " << value
            << '\n';
        }
    }

    return 0;
}
