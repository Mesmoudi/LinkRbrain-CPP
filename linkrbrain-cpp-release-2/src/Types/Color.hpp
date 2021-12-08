#ifndef LINKRBRAIN2019__SRC__TYPES__COLOR_HPP
#define LINKRBRAIN2019__SRC__TYPES__COLOR_HPP


#include <cmath>


namespace Types {

    struct Color {
        Color(const float gray=0.f) : r(gray), g(gray), b(gray), a(gray) {}
        Color(const float _r, const float _g, const float _b, const float _a=1.f) : r(_r), g(_g), b(_b), a(_a) {}
        union {
            float r;
            float red;
        };
        union {
            float g;
            float green;
        };
        union {
            float b;
            float blue;
        };
        union {
            float a;
            float alpha;
        };

        operator std::string() const {
            return "Color(" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ", " + ")";
        }

        const float compute_distance(const Color& other) const {
            return std::abs(r - other.r) + std::abs(g - other.g) + std::abs(b - other.b);
        }

        static const Color from_hsl(float h, float s, float l) {
            // see https://stackoverflow.com/questions/3423214/convert-hsb-hsv-color-to-hsl
            const float v = l + s * std::min(l, 1.f - l);
            const float s_ = (v == 0.f) ? 0.f : (2.f * (1.f - l / v));
            return from_hsv(h, s_, v);
        }
        static const Color from_hsv(float h, float s, float v) {
            // constrol data
            h = 6.f * std::fmod(h, 1.f);
            if (h < 0.f) s += 6.f;
            if (s < 0.f) s = 0.f;
            if (s > 1.f) s = 1.f;
            if (v < 0.f) v = 0.f;
            if (v > 1.f) v = 1.f;
            if (s == 0) return {v, v, v};
            // calculate things
            int i = h;
            float f = h - i;
            float p = v * (1.f - s);
            float q = v * (1.f - s * f);
            float t = v * (1.f - s * (1.f - f));
            // contemplate various cases, return result accordingly
            switch(i) {
        		case 0: return {v, t, p};
        		case 1: return {q, v, p};
        		case 2: return {p, v, t};
        		case 3: return {p, q, v};
        		case 4: return {t, p, v};
        		case 5: return {v, p, q};
        	}
            // code should never get here
            return {0, 0, 0};
        }

    };

} // PDF


#endif // LINKRBRAIN2019__SRC__TYPES__COLOR_HPP
