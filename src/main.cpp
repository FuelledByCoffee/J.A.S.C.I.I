#include <algorithm>
#include <cstdlib>
#include <err.h>
#include <getopt.h>
#include <memory>
#include <unistd.h>

#include <cmath>
#include <cstdint>
#include <iostream>
#include <utility>
#ifdef __EMSCRIPTEN__
	#include <emscripten.h>
#else
	#define EMSCRIPTEN_KEEPALIVE
#endif
#include <stb_image.h>
#include <stb_image_resize2.h>

using u8 = std::uint8_t;

struct pixel {
	u8 red;
	u8 green;
	u8 blue;
};

static int width_shrunk;
static int height_shrunk;
static int size  = 60;
static int color = false;

// -----------------------------------------------------------------------------
// stole this code from here: https://alienryderflex.com/saturation.html
static constexpr auto changeSaturation(pixel p, double change) -> pixel {
	constexpr double Pr = .299;
	constexpr double Pg = .587;
	constexpr double Pb = .114;

	u8 R = p.red;
	u8 G = p.green;
	u8 B = p.blue;

	const double P = sqrt(R * R * Pr + //
	                      G * G * Pg + //
	                      B * B * Pb);

	p.red   = std::min(std::fabs(P + ((R)-P) * change), 255.);
	p.green = std::min(std::fabs(P + ((G)-P) * change), 255.);
	p.blue  = std::min(std::fabs(P + ((B)-P) * change), 255.);
	return p;
}

// -----------------------------------------------------------------------------
static auto load_image(const char *image_path) {
	int  width, height, original_channels; // NOLINT
	auto img = std::unique_ptr<stbi_uc, decltype(&std::free)>{
			stbi_load(image_path, &width, &height, &original_channels, 3), std::free};

	if (!img) errx(2, "Failed to load image %s", image_path);

	const double scale_factor = size / std::fmax(height, width);
	width_shrunk              = width * scale_factor * 2; // Thanks bolt!
	height_shrunk             = height * scale_factor;

	return std::unique_ptr<pixel, decltype(&std::free)>(
			reinterpret_cast<pixel *>(stbir_resize(
					img.get(), width, height, 0, nullptr, width_shrunk, height_shrunk, 0,
					STBIR_RGB, STBIR_TYPE_UINT8, STBIR_EDGE_ZERO, STBIR_FILTER_BOX)),
			std::free);
}

// -----------------------------------------------------------------------------
static auto print_ascii_art(auto &&img) {
	constexpr char ASCIIMAP[] = "N@#W$9876543210?!abc;:+=-_,.  ";
	constexpr int  num_char   = sizeof ASCIIMAP - 1;

	for (int y = 0; y != height_shrunk; y++) {
		for (int x = 0; x != width_shrunk; x++) {
			const int   i = y * width_shrunk + x;
			const pixel p = changeSaturation(img.get()[i], 1.5);

			const double brightness = p.red * 0.2126 +   //
			                          p.green * 0.7152 + //
			                          p.blue * 0.0722;

			const unsigned index = unsigned(brightness * (num_char - 1) / 255);
			if (color)
				std::cout << std::format("\033[38;2;{:d};{:d};{:d}m{}\033[0m", //
				                         p.red, p.green, p.blue, ASCIIMAP[index]);
			else std::cout << ASCIIMAP[index];
		}

		std::cout << '\n';
	}
}

// -----------------------------------------------------------------------------
static constexpr struct option long_options[] = {
		{"version",       no_argument, NULL, 'v'},
		{  "image", required_argument, NULL, 'i'},
		{  "color",       no_argument, NULL, 'c'},
		{   "size", required_argument, NULL, 's'},
		{   "help",       no_argument, NULL, 'h'},
		{		 NULL,								 0, NULL,   0}
};

// -----------------------------------------------------------------------------
int main(int argc, char **argv) {
	int         opt        = 0;
	const char *image_path = argv[1];
	while ((opt = getopt_long(argc, argv, "vi:cs:h", long_options, NULL)) != -1) {
		switch (opt) {
			case 'c': color = 1; break;
			case 'i': image_path = optarg; break;
			case 's': size = atoi(optarg); break;
			default: break;
		}
	}
	if (optind < argc) image_path = argv[optind];
	if (!image_path) errx(1, "Need an image");

	const auto map = load_image(image_path);
	print_ascii_art(std::move(map));
}

// -----------------------------------------------------------------------------
// this is to give the data to emscripten so that it can resize the div
extern "C" {
EMSCRIPTEN_KEEPALIVE int  image_width() { return width_shrunk; }
EMSCRIPTEN_KEEPALIVE int  image_height() { return height_shrunk; }
EMSCRIPTEN_KEEPALIVE void set_size(int a) { size = a; }
EMSCRIPTEN_KEEPALIVE void set_color(int z) { color = z; }
}

// vim: ts=2
