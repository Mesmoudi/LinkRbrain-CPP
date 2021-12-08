#include "CLI/Display/Style.hpp"

#include <iostream>


using Style = CLI::Display::Style;


int main(int argc, char const *argv[]) {
    std::cout << Style() << "Normal" << Style(false) << '\n';
    std::cout << Style().set_bold() << "Bold" << Style(false) << '\n';
    std::cout << Style().set_underlined() << "Underlined" << Style(false) << '\n';
    std::cout << Style().set_underlined().set_bold() << "Bold & underlined" << Style(false) << '\n';
    std::cout << Style().set_blink() << "Blink" << Style(false) << '\n';
    std::cout << Style().set_bold().set_blink() << "Bold & blink" << Style(false) << '\n';
    std::cout << Style().set_underlined().set_blink() << "Underlined & blink" << Style(false) << '\n';
    std::cout << Style().set_underlined().set_bold().set_blink() << "Bold & underlined & blink" << Style(false) << '\n';
    std::cout << '\n';
    std::cout << Style().set_foreground_rgb(.5, 1, .8) << "/* message */" << Style(false) << '\n';
    std::cout << Style().set_background_rgb(.5, 1, .8).set_foreground_rgb(.5, 1, .8) << "/* message */" << Style(false) << '\n';
    std::cout << Style().set_background_rgb(0, .4, .3).set_foreground_rgb(1, .5, .1) << "/* message */" << Style(false) << '\n';
    std::cout << "/* message */" << Style(false) << '\n';
    std::cout << '\n';
    for (float x=0; x<=1.f; x+=1.f/42.f) {
        std::cout << Style().set_background_gray(x);
        std::cout << "Gray " << x << "    ";
        std::cout << Style(false);
        std::cout << '\n';
    }
    std::cout << '\n';
    for (float x=0; x<=360.f; x+=15.f) {
        std::cout << Style().set_background_hsl(x, 1, .5);
        std::cout << "Hue " << x << "    ";
        std::cout << Style(false);
        std::cout << '\n';
    }
    std::cout << '\n';
    for (float x=0; x<=1.f; x+=1.f/42.f) {
        std::cout << Style().set_background_heatmap(x);
        std::cout << "Heatmap " << x << "    ";
        std::cout << Style(false);
        std::cout << '\n';
    }
    return 0;
}
