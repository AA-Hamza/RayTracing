#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"
#include "rtweekend.h"

#include <iostream>

void write_color(std::ostream &out, const color pixel_color, int samples_per_pixel) {
    double r = pixel_color.x();
    double g = pixel_color.y();
    double b = pixel_color.z();

    r /= static_cast<double>(samples_per_pixel);
    g /= static_cast<double>(samples_per_pixel);
    b /= static_cast<double>(samples_per_pixel);
    /*
    auto scale = 1.0 / samples_per_pixel;
    r *= scale;
    g *= scale;
    b *= scale;
    */
    out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << " "
        << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << " "
        << static_cast<int>(256 * clamp(b, 0.0, 0.999)) << "\n";
}

#endif
