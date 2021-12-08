#include "Cuda.hpp"

#include "Generators/AnyRandom.hpp"

#include "Exceptions/Exception.hpp"
#include "Logging/Loggers.hpp"
auto& logger = Logging::get_logger();


static const size_t n = 1e4;

const std::string code = R"###(
    b_data[i] = a_data[i];
)###";


int main(int argc, char const *argv[]) {

    // prepare input & output
    std::vector<double> a, b;
    a.resize(n);
    b.resize(n);
    for (int i=0; i<n; ++i) {
        a[i] = (double)i / 10.;
        b[i] = (double)i / 100.;
    }
    logger.notice("Generated", n, "input values");
    std::vector<double> c = b;
    logger.notice("Prepared output");
    logger.message("Prepared input & output");

    // compile program
    Computing::Cuda cuda;
    Computing::CudaFunction& test_cuda = cuda.create_function("square");
    test_cuda.set_body(code)
        .add_argument<double>(Computing::CudaFunction::InputVector, "a")
        .add_argument<double>(Computing::CudaFunction::OutputVector, "b");
    logger.notice("Instanciated function");
    test_cuda.compile();
    logger.message("Successfully compiled function");
    test_cuda(a.size(), a, b);
    logger.notice("Executed function");
    test_cuda(a.size(), a, b);
    logger.notice("Executed function");
    for (int i=0; i<20; ++i) {
        std::cout << a[i] << "\t" << b[i] << '\n';
    }

    // the end!
    return 0;
}
