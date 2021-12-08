#ifndef LINKRBRAIN2019__SRC__CLI__DISPLAY__SCREEN_HPP
#define LINKRBRAIN2019__SRC__CLI__DISPLAY__SCREEN_HPP


#include <vector>
#include <string>
#include <ostream>
#include <sstream>

#include <string.h> // memcpy

#include "./get_terminal_size.hpp"


namespace CLI::Display {


    class Canvas {
    public:

        enum Mode {
            Pixel   = 0x01,
            Custom  = 0x03,
            Stretch = 0x10,
            Fit     = 0x30,
        };

        enum Style {
            Default = 0x00,
            // alignment
            Left    = 0x00,
            Center  = 0x02,
            Right   = 0x04,
            Top     = 0x00,
            Middle  = 0x20,
            Bottom  = 0x40,
            // color (see https://misc.flogisoft.com/bash/tip_colors_and_formatting)
            Black       = 0x000100,
            Red         = 0x000200,
            Green       = 0x000400,
            Yellow      = 0x000800,
            Blue        = 0x001000,
            Magenta     = 0x002000,
            Cyan        = 0x004000,
            LightGray   = 0x008000,
            DarkGray    = 0x010100,
            LightRed    = 0x010200,
            LightGreen  = 0x010400,
            LightYellow = 0x010800,
            LightBlue   = 0x011000,
            LightMagenta= 0x012000,
            LightCyan   = 0x014000,
            White       = 0x018000,
        };

        Canvas(const size_t width=0, const size_t height=0, const int style=0) {
            if (width == 0) {
                _width = get_terminal_width();
            } else {
                _width = width;
            }
            if (height == 0) {
                _height = get_terminal_height() - 1;
            } else {
                _height = height;
            }
            _style = style;
            set_mode(Pixel);
        }

        const int get_width() const {
            return _width;
        }
        const int get_height() const {
            return _height;
        }

        void set_style(const int style=0) {
            _style = style;
        }
        void set_mode(const Mode& mode, const double scale_x=1., const double scale_y=1., const double offset_x=0., const double offset_y=0.) {
            _mode = mode;
            _scale_x = scale_x;
            _scale_y = scale_y;
            _offset_x = offset_x;
            _offset_y = offset_y;
        }

        void write(const double x, const double y, const std::string& text, int style=0) {
            if (style == 0) {
                style = _style;
            }
            _patterns.push_back({x, y, text, style});
        }

        void clear() {
            _patterns.clear();
        }

        void render(std::ostream& os) {
            if (_patterns.size()) {
                // check boundaries
                double x_min = _patterns[0].x;
                double x_max = _patterns[0].x;
                double y_min = _patterns[0].y;
                double y_max = _patterns[0].y;
                for (const Pattern& pattern : _patterns) {
                    if (pattern.x < x_min)  x_min = pattern.x;
                    if (pattern.y < y_min)  y_min = pattern.y;
                    if (pattern.x > x_max)  x_max = pattern.x;
                    if (pattern.y > y_max)  y_max = pattern.y;
                }
                // what if min & max are the same?
                if (x_min == x_max) {
                    --x_min;
                    ++x_max;
                }
                if (y_min == y_max) {
                    --y_min;
                    ++y_max;
                }
                // deduce scaling parameters
                if (_mode == Stretch || _mode == Fit) {
                    _offset_x = -x_min;
                    _offset_y = -y_min;
                    _scale_x = (double)(_width-1) / (x_max - x_min);
                    _scale_y = (double)(_height-1) / (y_max - y_min);
                    if (_mode == Fit) {
                        if (2. * _scale_x < _scale_y) {
                            _scale_y = _scale_x / 2.;
                        } else {
                            _scale_x = _scale_y * 2.;
                        }
                    }
                }
            }
            // actually render things
            Result result(_width, _height);
            for (const Pattern& pattern : _patterns) {
                result.write(
                    _scale_x * (pattern.x + _offset_x),
                    _scale_y * (pattern.y + _offset_y),
                    pattern.text,
                    pattern.style);
            }
            // send result to buffer
            os << result.data;
        }

    private:

        struct ResultLine {
            ResultLine(int _size, char* _data) : size(_size), data(_data)  {}
            void write(const int x, const std::string& text) {
                int start = 0;
                int end = text.size();
                if (x < 0) {
                    start = -x;
                }
                if (x + text.size() > size) {
                    end = size - x;
                }
                if (end > start && start + x < size && start < size && end - start < size) {
                    memcpy(
                        data + start + x,
                        text.data() + start,
                        end - start);
                }
            }
            void write(const int x, const std::string& text, const int style) {
                int offset = 0;
                if (style & Style::Center) {
                    offset = -text.size() / 2;
                } else if (style & Style::Right) {
                    offset = -text.size();
                }
                write(x + offset, text);
            }
            int size;
            char* data;
        };
        struct Result {
            Result(const int _width, const int _height) : width(_width), height(_height) {
                data.assign(width*height, ' ');
                for (int i = 0; i < height; i++) {
                    lines.push_back({width, data.data() + i*width});
                }
            }
            void write(const int x, const int y, const std::string& text, const int style=0) {
                // cut text into segments
                std::stringstream buffer(text);
                std::string segment;
                std::vector<std::string> segments;
                while (std::getline(buffer, segment, '\n')) {
                    segments.push_back(segment);
                }
                // write segments according to alignment
                int offset = 0;
                if (style & Style::Middle) {
                    offset = segments.size() / 2;
                } else if (style & Style::Bottom) {
                    offset = 1 - segments.size();
                }
                int Y = y + offset;
                for (const std::string& segment : segments) {
                    if (Y >= 0 && Y < height) {
                        lines[Y].write(x, segment, style);
                    }
                    ++Y;
                }
            }
            int width;
            int height;
            std::vector<ResultLine> lines;
            std::string data;
        };
        struct Pattern {
            double x;
            double y;
            std::string text;
            int style;
        };

        Mode _mode;
        int _width;
        int _height;
        int _style;
        double _scale_x;
        double _scale_y;
        double _offset_x;
        double _offset_y;
        std::vector<Pattern> _patterns;

    };


    std::ostream& operator<< (std::ostream& os, Canvas& canvas) {
        canvas.render(os);
        return os;
    }

} // CLI::Display


#endif // LINKRBRAIN2019__SRC__CLI__DISPLAY__SCREEN_HPP
