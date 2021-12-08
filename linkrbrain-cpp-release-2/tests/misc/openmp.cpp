static const int n = 1e7;
static const double max_error = 1e-100;

#include <iostream>
#include <vector>
#include <omp.h>

#include "Exceptions/Exception.hpp"
#include "Logging/Loggers.hpp"
auto logger = Logging::get_logger();
#include "Generators/AnyRandom.hpp"


void test_openmp() {

}
void test_threads() {
    #pragma omp parallel num_threads(4)
    {
        #pragma omp critical
        logger.debug("Thread id is", omp_get_thread_num());
    }
}
void test_normal_computation(int n, double* a, double* b, double* c) {
    for (int i=1; i<n; ++i) {
        // c[i] = a[i] * a[i] / b[i];
        c[i] = ((a[i] + a[i-1]) / 2.0) / b[i];
    }
}
void test_openmp_computation(int n, double* a, double* b, double* c) {
    int i;
    #pragma omp parallel for private(i)
    for (i=1; i<n; i++)
    {
        // c[i] = a[i] * a[i] / b[i];
        c[i] = ((a[i] + a[i-1]) / 2.0) / b[i];
    }
}
void test_parallel_openmp_computation_subdomain(int i_min, int i_max, double* a, double* b, double* c) {
    int i;
    for (i=i_min; i<i_max; i++) {
        c[i] = ((a[i] + a[i-1]) / 2.0) / b[i];
    }
}
void test_parallel_openmp_computation(int n, double* a, double* b, double* c) {
    int thread_index, thread_count;
    int i_step, i_min, i_max;
    #pragma omp parallel default(shared) private(thread_index, thread_count, i_step, i_min, i_max)
    {
        thread_index = omp_get_thread_num();
        thread_count = omp_get_num_threads();
        i_step = n / thread_count; /* size of partition */
        if (thread_index == 0) {
            i_min = 1;
        } else  {
            i_min = thread_index * i_step;
        }
        if (thread_index == thread_index-1) {
            i_max = n;
        } else {
            i_max = i_min + i_step;
        }
        test_parallel_openmp_computation_subdomain(i_min, i_max, a, b, c);
        // logger.debug("thread", thread_index, "/", thread_count, " : going from", i_min, "to", i_max);
    }
}
void test_results(const std::vector<std::vector<double>> c_list) {
    for (int j=1; j<c_list.size(); ++j) {
        const std::vector<double>& c1 = c_list[j-1];
        const std::vector<double>& c2 = c_list[j];
        for (int i=0; i<n; ++i) {
            if (!std::isnan(c1[i]) && !std::isnan(c2[i]) && (c1[i] || c2[i])) {
                const double diff = std::abs(c1[i] - c2[i]);
                const double error = diff / std::max(std::abs(c1[i]), std::abs(c2[i]));
                if (error > max_error && diff > max_error) {
                    except("Values do not match at index", j, "-", i, " : ", c1[i], "!=", c2[i], ", diff is", diff, ", error is", error);
                }
            }
        }
    }
}

// void


int main(int argc, char const *argv[]) {
    test_openmp();
    logger.message("Ready for testing");
    test_threads();
    logger.message("Tested threads");
    std::vector<double> a, b, c1, c2, c3;
    a.resize(n);
    b.resize(n);
    c1.resize(n);
    c2.resize(n);
    c3.resize(n);
    Generators::AnyRandom().randomize(a.data(), sizeof(double) * a.size());
    Generators::AnyRandom().randomize(b.data(), sizeof(double) * b.size());
    // for (int i=0; i<n; ++i) {
    //     a[i] = Generators::Random::generate<double>();
    //     b[i] = Generators::Random::generate<double>();
    // }
    logger.notice("Generated", a.size(), "items for source vectors");

    test_normal_computation(n, a.data(), b.data(), c1.data());
    logger.debug("Computed normally");
    c1.resize(0);
    c1.resize(n);
    logger.debug("Reset data");
    test_normal_computation(n, a.data(), b.data(), c1.data());
    logger.notice("Computed normally");

    test_openmp_computation(n, a.data(), b.data(), c2.data());
    logger.debug("Computed using OpenMP");
    c2.resize(0);
    c2.resize(n);
    logger.debug("Reset data");
    test_openmp_computation(n, a.data(), b.data(), c2.data());
    logger.notice("Computed using OpenMP");

    test_parallel_openmp_computation(n, a.data(), b.data(), c3.data());
    logger.debug("Computed using parallel OpenMP");
    c3.resize(0);
    c3.resize(n);
    logger.debug("Reset data");
    test_parallel_openmp_computation(n, a.data(), b.data(), c3.data());
    logger.notice("Computed using parallel OpenMP");

    test_results({c1, c2, c3});
    logger.notice("Compared data");
    logger.message("Tested computation");
    return 0;
}
