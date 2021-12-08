#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__VIEW_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__VIEW_HPP


#include "LinkRbrain/Parsing/NiftiParser.hpp"

#include "CLI/Arguments/Command.hpp"

#include "./linkrbrain.hpp"


namespace LinkRbrain::Commands {

    static Types::Array3D<T> array3d;

    void linkrbrain_image(const CLI::Arguments::CommandResult& options) {
        if (options.get("source") == "nifti") {
            // fetch data from file
            LinkRbrain::Parsing::NiftiParser parser;
            parser.parse(array3d, options.get("path"));
            std::cerr << "Parsed " << options.get("path") << "\n\n";
        } else {
            // oopsee...
            std::cerr << "Unrecognized source type: " << options.get("source") << "\n";
            exit(-1);
        }
    }

    void linkrbrain_image_view(const CLI::Arguments::CommandResult& options) {
        // extract color mode from parameters
        Types::Array3D<T>::ColorMode color_mode = Types::Array3D<T>::ColorMode::Heatmap;
        const std::string color_mode_string = options.get("color-mode");
        if (color_mode_string == "grayscale") {
            color_mode = Types::Array3D<T>::ColorMode::Grayscale;
        } else if (color_mode_string == "hue") {
            color_mode = Types::Array3D<T>::ColorMode::Hue;
        }
        // axis
        int axis = options.get("axis")[0] - 'x';
        // position
        T position;
        if (options.has("position")) {
            position = std::stof(options.get("position"));
        } else {
            const auto& extrema = array3d.get_extrema();
            position = (extrema.min.values[axis] + extrema.max.values[axis]) / 2.;
        }
        // show data
        array3d.show(
            axis,
            position,
            color_mode
        );
    }

    void linkrbrain_image_list(const CLI::Arguments::CommandResult& options) {
        for (const auto& point : array3d) {
            const T& value = * point.value;
            if (!value) {
                continue;
            }
            std::cout
                << point.coordinates.x << ' '
                << point.coordinates.y << ' '
                << point.coordinates.z << ' '
                << value << '\n';
        }
    }

} // LinkRbrain::Commands


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__VIEW_HPP
