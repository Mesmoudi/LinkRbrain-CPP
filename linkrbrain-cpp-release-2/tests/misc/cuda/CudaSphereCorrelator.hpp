#include "./Array3D.hpp"
#include "LinkRbrain/Models/Point.hpp"
#include "LinkRbrain/Correlation/SphereCorrelator.hpp"

using Point = LinkRbrain::Models::WeightedPoint;
using Extrema = LinkRbrain::Models::Extrema;


template <typename T>
__global__ void SphereCorrelator__project_gpu(
    const T* densitymap,
    const LinkRbrain::Models::TemplatedWeightedPoint<T> origin,
    const LinkRbrain::Models::TemplatedWeightedPoint<T> center,
    const T resolution,
    const T diameter, const T weight
) {
    const T dx = CUDA_ARRAY3D_X - center.x;
    const T dy = CUDA_ARRAY3D_Y - center.y;
    const T dz = CUDA_ARRAY3D_Z - center.z;
    const T distance = sqrt(dx*dx + dy*dy + dz*dz);
    if (distance >= diameter) {
        return;
    }
    const double k = distance / diameter;
    densitymap[CUDA_ARRAY3D_INDEX] += weight * (0.5*k * (k*k - 3.0) + 1.0);
}


namespace LinkRbrain::Correlation {


    class CudaSphereCorrelator : SphereCorrelator {
    public:

        CudaSphereCorrelator(const double& resolution, const double& diameter) :
            SphereCorrelator(resolution, diameter),
            _diameter(diameter) {}

        template <typename T>
        void inflate_dimensions(LinkRbrain::Models::TemplatedExtrema<T>& extrema) {
            extrema.inflate_dimensions(_diameter);
        }

        template <typename T, typename T2>
        void project_gpu(Array3D<T>& densitymap, const LinkRbrain::Models::TemplatedWeightedPoint<T2>& center) {
            LinkRbrain::Models::TemplatedWeightedPoint<T> origin(
                resolution * std::floor((center.x - diameter) / resolution),
                resolution * std::floor((center.y - diameter) / resolution),
                resolution * std::floor((center.z - diameter) / resolution),
            );
            // SphereCorrelator__project_gpu<<<
            //
            // >>>(
            //     densitymap.get_gpu_data(),
            //     point,
            //     _resolution,
            //     _diameter
            // );
        }
        template <typename T, typename T2>
        void project_gpu(Array3D<T>& densitymap, const std::vector<LinkRbrain::Models::TemplatedWeightedPoint<T2>>& points) {
            for (const auto& point : points) {
                project_gpu(densitymap, point);
            }
        }

    protected:

        const std::string& get_logger_name() const {
            static const std::string name = "CudaSphereCorrelator";
            return name;
        }

    private:

        double _diameter;
        Extrema _extrema;

    };

} // LinkRbrain::Correlation
