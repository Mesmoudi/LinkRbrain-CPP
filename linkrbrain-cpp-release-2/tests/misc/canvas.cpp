#include <iostream>

#include "CLI/Display/Canvas.hpp"

#include <cmath>


typedef CLI::Display::Canvas::Style Style;


int main(int argc, char const *argv[]) {
    /*
    {
        CLI::Display::Canvas canvas;
        canvas.set_mode(CLI::Display::Canvas::Mode::Pixel);
        const int w = canvas.get_width();
        const int h = canvas.get_height();
        // center
        canvas.set_style(Style::Center);
        for (size_t i = 0; i < h; i++) {
            canvas.write(0, i, "**********");
        }
        // diagonal
        canvas.set_style(Style::Left);
        for (size_t i = 0; i < h; i++) {
            char c = 'A' + (i % 26);
            canvas.write(i, i, std::string(3, c));
        }
        // diagonal II
        canvas.set_style(Style::Center);
        for (size_t i = 0; i < h; i+=2) {
            char c = 'A' + (i % 26);
            canvas.write(20+2*i, i, "|\n-" + std::string(1, c) + "-\n|");
        }
        // to the right
        canvas.set_style();
        canvas.write(w, h-1, "Now testing...\nStyle::Right\nwith\nStyle::Bottom", Style::Right | Style::Bottom);
        // render!
        std::cout << canvas << '\n';
    }
    */

    {
        CLI::Display::Canvas canvas;
        canvas.set_mode(CLI::Display::Canvas::Mode::Fit);
        size_t count = 128;
        double k = 2. * M_PI / (double)count;
        for (size_t i = 0; i < count; i++) {
            double theta = (double)i * k;
            canvas.write(cos(theta), sin(theta), "--+--", Style::Center);
        }
        std::cout << canvas << '\n';
    }
    getchar();
    return 0;
}
