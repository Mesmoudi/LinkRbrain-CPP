#include "Generators/ColorName.hpp"

#include <iostream>
#include <iomanip>


void test_color(const Types::Color& color) {
    std::cout << std::left << std::setw(48) << std::string(color);
    std::cout << Generators::ColorName::get_english_name(color) << '\n';
}


int main(int argc, char const *argv[]) {
    test_color({0, 0, 0});
    test_color({0, .8, 0});
    test_color({0, .8, .2});
    const float saturation = 1.f;
    for (float hue = 0.f; hue < 1.f; hue += .1) {
        std::cout << "\nhue = " << hue << '\n';
        test_color(Types::Color::from_hsv(hue, saturation, 0.00));
        test_color(Types::Color::from_hsv(hue, saturation, 0.25));
        test_color(Types::Color::from_hsv(hue, saturation, 0.50));
        test_color(Types::Color::from_hsv(hue, saturation, 1.00));
    }
    std::cout << "\n\nLinkRbrain colors" << '\n';
    for (float hue = 0.f; hue < 1.f; hue += .05) {
        test_color(Types::Color::from_hsv(hue, 1, 0.8));
    }
    return 0;
}
