#include "rtweekend.h"
#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include "ctpl_stl.h"
#include <vector>
#include <future>

#include <iostream>


color ray_color(const ray& r, const hittable& world, int depth) {
    hit_record rec;
    if (depth <= 0)
        return color(0, 0, 0);

    if (world.hit(r, 0.001, infinity, rec)) {
        ray scattered;
        color attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
            return attenuation * ray_color(scattered, world, depth-1);
        }
        return color(0, 0, 0);
    }
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0-t)*color(1.0, 1.0, 1.0) + t*color(0.5, 0.7, 1.0);
}

hittable_list my_scene() {
    hittable_list world;

    auto ground_material = make_shared<lambertian>(color(0.5, 0.7, 0.5));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    auto material1 = make_shared<dielectric>(0.5);
    world.add(make_shared<sphere>(point3(1, 1.5, -4.5), 1.5, material1));
    auto material2 = make_shared<lambertian>(color(0.9, 0.5, 0.1));
    world.add(make_shared<sphere>(point3(-1, 3, 0), 3.0, material2));
    auto material3 = make_shared<metal>(color(0.2, 0.9, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(1, 1.5, 4.5), 1.5, material3));

    auto material4 = make_shared<metal>(color(0.7, 0.7, 0.7), 0.0);
    world.add(make_shared<sphere>(point3(6, 0.5, 0), 0.5, material4));
    world.add(make_shared<sphere>(point3(5.5, 0.5, 1), 0.5, material4));
    world.add(make_shared<sphere>(point3(5.5, 0.5, -1), 0.5, material4));
    world.add(make_shared<sphere>(point3(5, 0.5, 2), 0.5, material4));
    world.add(make_shared<sphere>(point3(5, 0.5, -2), 0.5, material4));

    return world;
}

int main()
{
    // Image
    const auto aspect_ratio = 3.0 / 2.0;
    const int image_width = 1200;
    const int image_height = static_cast<int>(image_width / aspect_ratio);

    // Modify thigs
    const int samples_per_pixel = 20;
    const int max_depth = 10;


    // World
    auto  world = my_scene();


    // Camera
    point3 lookfrom(30,20,5);
    point3 lookat(0,1,0);
    vec3 vup(0,1,0);
    auto dist_to_focus = 30.0;
    auto aperture = 0.1;

    camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

    // Threading
    const int num_of_threads = std::thread::hardware_concurrency()/2+1;         //8 Threads in my case
    const int num_of_jobs= num_of_threads * 4;          // that means if all jobs are equal then each thread would work 4 times, but they aren't equal so this is better
    const int lines_per_thread = image_height/num_of_jobs; 
    ctpl::thread_pool pool(num_of_threads);
    std::vector<std::future<void>> jobs_results;

    color **image_buffer = new color*[image_height];
    for (int i = 0; i < image_height; ++i) {
        image_buffer[i] = new color[image_width];
    }

    int lines_finished = 0;
    int starting_line = image_height-1;
    while (starting_line >= 0) {
        int finishing_line = starting_line-lines_per_thread+1;
        if (finishing_line < 0)
            finishing_line = 0;
        jobs_results.push_back(pool.push([=, &lines_finished](int) {
            for (int j = starting_line; j >= finishing_line; --j) {
                for (int i = 0; i < image_width; ++i) {
                    color pixel_color(0, 0, 0);
                    for (int s = 0; s < samples_per_pixel; ++s) {
                        auto u = double(i + random_double()) / (image_width-1);
                        auto v = double(j + random_double()) / (image_height-1);
                        ray r = cam.get_ray(u, v);
                        pixel_color += ray_color(r, world, max_depth);
                    }
                    image_buffer[j][i] = pixel_color;
                }
                lines_finished += 1;
                std::cerr << "\rFinished: " << (lines_finished*100)/image_height << "%";
            }
        } ));
        starting_line -= lines_per_thread;
    }
    for (int i = 0; i < jobs_results.size(); ++i) {
        jobs_results[i].get();          //Wait for all jobs to be done first
    }


    // Writing to STDOUT
    std::cerr << "\nWriting to stdout" << std::endl;
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    for (int j = image_height-1; j >= 0; --j) {
        for (int i = 0; i < image_width; ++i) {
            write_color(std::cout, image_buffer[j][i], samples_per_pixel);
        }
    }
    std::cerr << "\nDone.\n";
}
