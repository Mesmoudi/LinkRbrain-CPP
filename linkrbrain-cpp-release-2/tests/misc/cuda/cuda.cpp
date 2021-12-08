#include "./CudaSphereCorrelator.hpp"

#include "Buffering/Reading/FileReader.hpp"
#include "LinkRbrain/Models/Dataset.hpp"

#include "Exceptions/Exception.hpp"
#include "Logging/Loggers.hpp"
auto& logger = Logging::get_logger();


typedef float Floating;
static const Floating diameter = 10.0;
static const Floating resolution = 2.0;


int main(int argc, char const *argv[]) {
    Cuda cuda;
    logger.notice("Instanciated CUDA module");
    LinkRbrain::Correlation::CudaSphereCorrelator correlator(resolution, diameter);
    logger.notice("Instanciated CudaSphereCorrelator");
    Buffering::Reading::FileReader reader("var/lib/linkrbrain/datasets/functions/data", Buffering::Format::Binary);
    LinkRbrain::Models::Dataset dataset("functions");
    reader >> dataset;
    logger.notice("Loaded functions");
    LinkRbrain::Models::TemplatedExtrema<Floating> extrema = dataset.compute_extrema();
    logger.debug("Got dataset extrema:", extrema);
    correlator.inflate_dimensions(extrema);
    logger.debug("Inflated functions extrema:", extrema);
    Array3D<Floating> projection(extrema, resolution);
    logger.debug("Instanciated 3D matrix");
    projection.allocate_gpu();
    projection.clear_gpu_data();
    logger.debug("Allocated & cleared 3D matrix (GPU-side)");
    for (const auto& group : dataset.get_groups()) {
        correlator.project_gpu(projection, group.get_points());
    }
    logger.debug("Projected dataset on matrix using GPU");
    logger.notice("Projected dataset on matrix");
    logger.message("Initialized things");
    projection.gpu2cpu();
    logger.debug("Copied 3D matrix from GPU");

    /* code */
    return 0;
}
