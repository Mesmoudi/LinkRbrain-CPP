#include "Exceptions/Exception.hpp"
#include "Logging/Loggers.hpp"
auto& logger = Logging::get_logger();

#include <math.h>


static float error = 1e-4;
// static size_t n = 1 << 5;
// static size_t threads = 4;
static size_t n = 1 << 24;
static size_t threads = 1 << 8;
static size_t blocks = ceil(n / threads);
static size_t size = n * sizeof(float);


void set_gpu_threads(size_t threads);
void test_gpu(float a, float* in1, float* in2, float* out, size_t n);

void test_cpu(float a, float* in1, float* in2, float* out, size_t n) {
    for (size_t i=0; i<n; ++i) {
        const float d = in1[i] - in2[i];
        const float dd = (i == 0) ? 0.0 : (in1[i+1] - in1[i]);
        out[i] = a * sqrt(in1[i] + in2[i] / a) / in1[i] + in1[i] / in2[i] - a;
        out[i] += d*d + dd*dd;
        out[i] -= a / d + dd - sqrt(a);
        for (float s=0.f; s<10.f; ++s) {
            out[i] += ++s;
        }
    }
}


int main(int argc, char const *argv[]) {

    // prepare data
    float a = 2.0;
    float* in1 = (float*) malloc(size);
    float* in2 = (float*) malloc(size);
    float* cpu_out = (float*) malloc(size);
    float* gpu_out = (float*) malloc(size);
    for (size_t i=0; i<n; ++i) {
        in1[i] = (float) (i + 1.0);
        in2[i] = (in1[i] + 3.0) / 2.0;
    }
    logger.notice("Prepared CPU data");

    // CPU test
    test_cpu(a, in1, in2, cpu_out, n);
    logger.debug("Executed CPU test function");
    memset(cpu_out, 0, size);
    logger.debug("Blanked CPU output");
    test_cpu(a, in1, in2, cpu_out, n);
    logger.notice("Executed CPU test function, again");

    // GPU test
    set_gpu_threads(threads);
    test_gpu(a, in1, in2, gpu_out, n);
    logger.debug("Executed GPU test function");
    test_gpu(a, in1, in2, gpu_out, n);
    logger.notice("Executed GPU test function, again, with", blocks, "blocks and", threads, "threads");

    // comparison
    for (size_t i=0; i<n; ++i) {
        const float d = fabs(cpu_out[i] - gpu_out[i]);
        const float dd = d / fabs(cpu_out[i]);
        if (d > error && dd > error) {
            // logger.warning("Error at index", i, ": cpu_out =", cpu_out[i], ", gpu_out =", gpu_out[i]);
            except("Error at index", i, ": cpu_out =", cpu_out[i], ", gpu_out =", gpu_out[i]);
        }
    }
    logger.debug("Tested all values, same for GPU and CPU");

    // the end
    return 0;
}
