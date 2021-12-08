#ifndef LINKRBRAIN2019__SRC__CLI__DISPLAY__STYLE_HPP
#define LINKRBRAIN2019__SRC__CLI__DISPLAY__STYLE_HPP


#include <string>
#include <math.h>


namespace CLI::Display {

    template <typename T>
    class StyleAttribute {
    public:
        StyleAttribute() : _dirty(false) {}
        StyleAttribute& operator = (const T& value) {
            _dirty = true;
            _value = value;
            return *this;
        }
        operator const T& () const {
            return _value;
        }
        const bool is_dirty() const {
            return _dirty;
        }
    private:
        bool _dirty;
        T _value;
    };


    struct Style {

        Style(const bool _preserve = true) : preserve(_preserve) {}

        Style& set_foreground(const int index) {
            foreground_index = index;
            return *this;
        }
        Style& set_background(const int index) {
            background_index = index;
            return *this;
        }

        Style& set_foreground_rgb(const float r, const float g, const float b) {
            return set_foreground(compute_index_rgb(r, g, b));
        }
        Style& set_background_rgb(const float r, const float g, const float b) {
            return set_background(compute_index_rgb(r, g, b));
        }

        Style& set_foreground_hsl(const float h, const float s, const float l) {
            return set_foreground(compute_index_hsl(h, s, l));
        }
        Style& set_background_hsl(const float h, const float s, const float l) {
            return set_background(compute_index_hsl(h, s, l));
        }

        Style& set_foreground_gray(const float l) {
            return set_foreground(compute_index_gray(l));
        }
        Style& set_background_gray(const float l) {
            return set_background(compute_index_gray(l));
        }

        Style& set_foreground_heatmap(const float x, const bool hue_only=false) {
            return set_foreground(compute_index_heatmap(x, hue_only));
        }
        Style& set_background_heatmap(const float x, const bool hue_only=false) {
            return set_background(compute_index_heatmap(x, hue_only));
        }

        Style& set_bold(const bool _bold = true) {
            bold = _bold;
            return *this;
        }
        Style& set_underlined(const bool _underlined = true) {
            underlined = _underlined;
            return *this;
        }
        Style& set_blink(const bool _blink = true) {
            blink = _blink;
            return *this;
        }

        operator std::string () const {
            std::string formatting;
            // clear
            if (!preserve) {
                formatting += "\e[0m";
            }
            // bold
            if (bold.is_dirty()) {
                if (bold) {
                    formatting += "\e[1m";
                } else {
                    formatting += "\e[21m";
                }
            }
            // underlined
            if (underlined.is_dirty()) {
                if (underlined) {
                    formatting += "\e[4m";
                } else {
                    formatting += "\e[24m";
                }
            }
            // blink
            if (blink.is_dirty()) {
                if (blink) {
                    formatting += "\e[5m";
                } else {
                    formatting += "\e[25m";
                }
            }
            // colors
            if (foreground_index.is_dirty()) {
                formatting += "\e[38;5;";
                formatting += std::to_string(foreground_index);
                formatting += "m";
            }
            if (background_index.is_dirty()) {
                formatting += "\e[48;5;";
                formatting += std::to_string(background_index);
                formatting += "m";
            }
            // that was it!
            return formatting;
        }

        bool preserve;
        StyleAttribute<bool> bold;
        StyleAttribute<bool> underlined;
        StyleAttribute<bool> blink;
        StyleAttribute<int> foreground_index;
        StyleAttribute<int> background_index;

        static int expand05(const float x) {
            int result = 6.f * x;
            if (result > 5) {
                return 5;
            } else {
                return result;
            }
        }
        static int compute_index_gray(const float l) {
            const int x = 26 * l;
            switch (x) {
                case 0:
                    return 16;
                case 25:
                case 26:
                    return 15;
                default:
                    return 231 + x;
            }
        }
        static int compute_index_rgb(const float r, const float g, const float b) {
            return 16 + expand05(b) + 6*expand05(g) + 36*expand05(r);
        }
        static int compute_index_hsl(float h, const float s, const float l) {
            h = fmod(h, 360.f);
            const float c = (1.f - abs(2.f * l - 1.f)) * s;
            const float h0 = h / 60.f;
            const float x = c * (1.f - abs(fmod(h0, 2.f) - 1));
            const float m = l - c / 2.f;
            float r0, g0, b0;
            switch ((int) h0) {
                case 0:
                    r0 = c;
                    g0 = x;
                    b0 = 0;
                    break;
                case 1:
                    r0 = x;
                    g0 = c;
                    b0 = 0;
                    break;
                case 2:
                    r0 = 0;
                    g0 = c;
                    b0 = x;
                    break;
                case 3:
                    r0 = 0;
                    g0 = x;
                    b0 = c;
                    break;
                case 4:
                    r0 = x;
                    g0 = 0;
                    b0 = c;
                    break;
                case 5:
                    r0 = c;
                    g0 = 0;
                    b0 = x;
                    break;
            }
            return compute_index_rgb(r0 + m, g0 + m, b0 + m);
        }
        static int compute_index_heatmap(const float x, const bool hue_only=false) {
            if (hue_only) {
                return compute_index_hsl(240.f * (1.f - x), 1.f, .5f);
            } else {
                const float y = 360.f * x - 60.f;
                if (y < 0.f) {
                    return compute_index_hsl(240.f, 1.f, .5f + y / 120.f);
                }
                if (y > 240.f) {
                    return compute_index_hsl(0.f, 1.f, .5 + (y - 240.f) / 120.f);
                }
                return compute_index_hsl(240.f - y, 1.f, 0.5);
            }
        }

    };


    std::ostream& operator << (std::ostream& buffer, const Style& style) {
        return (buffer << (std::string) style);
    }


} // CLI::Display


#endif // LINKRBRAIN2019__SRC__CLI__DISPLAY__STYLE_HPP
