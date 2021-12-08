#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__PARSING__NIFTIPARSER_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__PARSING__NIFTIPARSER_HPP


#include <filesystem>

#include <nifti/nifti1_io.h>

#include "Types/Array3D.hpp"
#include "Logging/Loggable.hpp"
#include "LinkRbrain/Models/Group.hpp"


namespace LinkRbrain::Parsing {

    class NiftiParser : public Logging::Loggable {
    public:

        NiftiParser() {}


        template <typename T, typename T2>
        static void copy_data(Types::Array3D<T2>& array3d, const nifti_image* nim) {
            const T* input_data = (T*) nim->data;
            T2* output_data = array3d.get_data();
            for (size_t k = 0; k < nim->nz; k++) {
                for (size_t j = 0; j < nim->ny; j++) {
                    for (size_t i = 0; i < nim->nx; i++) {
                        output_data[array3d.make_index(i, j, k)] = (*input_data++);
                    }
                }
            }
        }


        template <typename T2>
        void parse(Types::Array3D<T2>& array3d, const std::filesystem::path& path) {
            // open image
            nifti_image* nim = nifti_image_read(path.native().c_str(), 1);
            if (!nim) {
                throw Exceptions::NotFoundException("Cannot open Nifti file: " + path.native());
            }
            get_logger().debug("Opened Nifti file ", path);
            // check file header
            get_logger().debug(nim->ndim, " dimensions");
            if (nim->ndim != 3) {
                throw Exceptions::BadDataException("Nifti has " + std::to_string(nim->ndim) + " dimensions, should be 3: " + path.native());
                get_logger().debug("Opened Nifti file ", path);
            }
            // retrieve basic info from Nifti
            Types::Point<T2> d;
            Types::PointExtrema<T2> extrema;
            Types::Point<T2> resolution = {nim->dx, nim->dy, nim->dz};
            if (nim->qform_code > 0) {
                d = {nim->qto_xyz.m[0][0], nim->qto_xyz.m[1][1], nim->qto_xyz.m[2][2]};
                extrema.integrate(Types::Point<T2>(nim->qoffset_x, nim->qoffset_y, nim->qoffset_z));
                extrema.integrate(Types::Point<T2>(nim->qoffset_x + (nim->nx - 1) * d.x, nim->qoffset_y + (nim->ny - 1) * d.y, nim->qoffset_z + (nim->nz - 1) * d.z));
            } else if (nim->sform_code > 0) {
                d = {nim->sto_xyz.m[0][0], nim->sto_xyz.m[1][1], nim->sto_xyz.m[2][2]};
                extrema.integrate(Types::Point<T2>(nim->sto_xyz.m[0][3], nim->sto_xyz.m[1][3], nim->sto_xyz.m[2][3]));
                extrema.integrate(Types::Point<T2>(nim->sto_xyz.m[0][3] + (nim->nx - 1) * d.x, nim->sto_xyz.m[1][3] + (nim->ny - 1) * d.y, nim->sto_xyz.m[2][3] + (nim->nz - 1) * d.z));
            } else {
                throw Exceptions::BadDataException("Neither QFORM nor SFORM can be retrieved in Nifti: " + std::to_string(nim->datatype));
            }
            array3d.set(extrema, resolution);
            get_logger().notice("Initialized 3D array");
            get_logger().debug(nim->nx, " x ", nim->ny, " x ", nim->nz);
            get_logger().debug(
                extrema.min.x, "...", extrema.max.x, " ; ",
                extrema.min.y, "...", extrema.max.y, " ; ",
                extrema.min.z, "...", extrema.max.z);
            // read data
            nifti_image_load(nim);
            get_logger().debug("Loaded data from Nifti file ", path);
            switch (nim->datatype) {
                case DT_UINT8:
                    copy_data<uint8_t>(array3d, nim);
                    break;
                case DT_UINT16:
                    copy_data<uint16_t>(array3d, nim);
                    break;
                case DT_UINT32:
                    copy_data<uint32_t>(array3d, nim);
                    break;
                case DT_INT8:
                    copy_data<int8_t>(array3d, nim);
                    break;
                case DT_INT16:
                    copy_data<int16_t>(array3d, nim);
                    break;
                case DT_INT32:
                    copy_data<int32_t>(array3d, nim);
                    break;
                case DT_FLOAT32:
                    copy_data<float>(array3d, nim);
                    break;
                case DT_FLOAT64:
                    copy_data<double>(array3d, nim);
                    break;
                default:
                    throw Exceptions::BadDataException("Unexpected data type in Nifti: " + std::to_string(nim->datatype));
            }
            get_logger().notice("Copied data to 3D array");
            nifti_image_free(nim);
            get_logger().debug("Done reading Nifti file ", path);
        }

        template <typename T>
        void parse(LinkRbrain::Models::Group<T>& group, const std::filesystem::path& path, const T resolution=4.) {
            Types::Array3D<T> array3d;
            parse(array3d, path);
            for (const auto& voxel : array3d) {
                const T weight = *voxel.value;
                group.integrate_point({
                    round(voxel.coordinates.x / resolution) * resolution,
                    round(voxel.coordinates.y / resolution) * resolution,
                    round(voxel.coordinates.z / resolution) * resolution,
                    *voxel.value
                });
            }
            double max_weight = 0.0;
            for (const auto& point : group.get_points()) {
                const double weight = std::abs(point.weight);
                if (weight > max_weight) {
                    max_weight = weight;
                }
            }
            if (max_weight) {
                for (auto& point : group.get_points()) {
                    point.weight /= max_weight;
                }
            }
        }

    protected:

        virtual const std::string get_logger_name() {
            return "NiftiParser";
        }

    };


} // LinkRbrain::Parsing


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__PARSING__NIFTIPARSER_HPP
