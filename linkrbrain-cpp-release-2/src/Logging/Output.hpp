#ifndef LINKRBRAIN2019__SRC__LOGGING__OUTPUT_HPP
#define LINKRBRAIN2019__SRC__LOGGING__OUTPUT_HPP


#include "Exceptions/Exception.hpp"

#include "./Level.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>


namespace Logging {

    struct Output {

        enum Type {
            File,
            StandardError,
            StandardOutput,
        };

        Type type;
        std::ostream* buffer;
        std::shared_ptr<std::ostream> buffer_;
        std::filesystem::path path;
        bool color;
        Level level;

        Output(const std::filesystem::path& path, const bool open_stream=false, const Level _level=Level::Detail) :
            Output(File, path, open_stream, _level) {}

        Output(const Type& _type, const std::filesystem::path& _path, const bool open_stream=false, const Level _level=Level::Detail) :
            type(_type),
            path(_path),
            buffer(NULL),
            level(_level)
        {
            switch (type) {
                case StandardError:
                    buffer = &std::cerr;
                    break;
                case StandardOutput:
                    buffer = &std::cout;
                    break;
                case File:
                    if (open_stream) {
                        open();
                    }
                    break;
            }
        }
        Output(const Type& _type = StandardError) : type(_type) {
            switch (type) {
                case StandardError:
                    buffer = &std::cerr;
                    break;
                case StandardOutput:
                    buffer = &std::cout;
                    break;
                case File:
                    throw Exceptions::Exception("You should specify a path for Output when using 'File' output type");
            }
        }

        void open() {
            switch (type) {
                case StandardError:
                case StandardOutput:
                    break;
                case File:
                    std::cerr << ("Logger opening file: " + path.native() + "\n");
                    buffer = new std::ofstream(path, std::ofstream::out | std::ofstream::app);
                    buffer_.reset(buffer);
                    break;
            }
        }

        template <typename T>
        Output& operator << (const T& thing) {
            if (buffer != NULL) {
                *buffer << thing;
            }
            return *this;
        }

        Output& set_color(const bool _color) {
            color = _color;
            return *this;
        }

        Output& set_level(const Level _level) {
            level = _level;
            return *this;
        }
    };

} // Logging

#endif // LINKRBRAIN2019__SRC__LOGGING__OUTPUT_HPP
