#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__PARSING__TEXTPARSER_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__PARSING__TEXTPARSER_HPP


#include <filesystem>

#include <nifti/nifti1_io.h>

#include "Types/Array3D.hpp"
#include "Logging/Loggable.hpp"


namespace LinkRbrain::Parsing {

    class TextParser : public Logging::Loggable {
    public:

        TextParser() {}


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

        template <typename T>
        void parse(LinkRbrain::Models::Group<T>& group, std::istream& source, const bool merge_points=true) {
            std::string line;
            while (std::getline(source, line)) {
                Types::Point<T> point;
                int count;
                // parse float
                if constexpr (sizeof(T) == 4) {
                    count = sscanf(
                        line.c_str(),
                        "%f %f %f %f",
                        &point.x, &point.y, &point.z, &point.weight);
                // parse double
                } else if constexpr (sizeof(T) == 8) {
                    count = sscanf(
                        line.c_str(),
                        "%lf %lf %lf %lf",
                        &point.x, &point.y, &point.z, &point.weight);
                // unknown value type
                } else {
                    count = 0;
                }
                // weight defaults to 1
                if (count < 4) {
                    point.weight = 1.;
                }
                // point into group
                if (merge_points) {
                    group.integrate_point(point);
                } else {
                    group.add_point(point);
                }
            }
        }

    protected:

        virtual const std::string get_logger_name() {
            return "TextParser";
        }

    };


} // LinkRbrain::Parsing


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__PARSING__TEXTPARSER_HPP
