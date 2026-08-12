#include <err.h>
#include <getopt.h>
#include <unistd.h>

#include <cmath>
#include <cstdint>
#include <iostream>
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
static constexpr void changeSaturation(pixel &p, double change) {
	constexpr double Pr = .299;
	constexpr double Pg = .587;
	constexpr double Pb = .114;

	u8 R = p.red;
	u8 G = p.green;
	u8 B = p.blue;

	const double P = sqrt(R * R * Pr + //
	                      G * G * Pg + //
	                      B * B * Pb);

	p.red   = fmin(fabs(P + ((R)-P) * change), 255);
	p.green = fmin(fabs(P + ((G)-P) * change), 255);
	p.blue  = fmin(fabs(P + ((B)-P) * change), 255);
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

	constexpr char ASCIIMAP[] = "N@#W$9876543210?!abc;:+=-_,.  ";
	constexpr int  num_char   = sizeof ASCIIMAP - 1;

	int      width, height, original_channels; // NOLINT
	stbi_uc *img = stbi_load(image_path, &width, &height, &original_channels, 3);
	if (!img) errx(1, "Failed to load image %s", image_path);

	const double scale_factor = size / fmax(height, width);
	width_shrunk              = (int)(width * scale_factor * 2);
	height_shrunk             = (int)(height * scale_factor);

	stbi_uc *map = new stbi_uc[sizeof(pixel) * width_shrunk * height_shrunk];
	stbir_resize(img, width, height, 0, map, width_shrunk, height_shrunk, 0,
	             STBIR_RGB, STBIR_TYPE_UINT8, STBIR_EDGE_ZERO,
	             STBIR_FILTER_BOX); // stb the goat

	for (int y = 0; y != height_shrunk; y++) {
		for (int x = 0; x != width_shrunk; x++) {
			int   i = ((y * width_shrunk) + x) * 3;
			pixel p = {map[i], map[i + 1], map[i + 2]};
			changeSaturation(p, 1.5);

			const double brightness =
					p.red * 0.2126 + p.green * 0.7152 + p.blue * 0.0722;
			const int index = brightness * (num_char - 1) / 255;
			if (color)

				std::cout << "\033[38;2;"                     //
									<< static_cast<int>(p.red) << ";"   //
									<< static_cast<int>(p.green) << ";" //
									<< static_cast<int>(p.blue) << "m"  //
									<< ASCIIMAP[index] << "\033[0m";
			else std::cout << ASCIIMAP[index];
		}

		std::cout << std::endl;
	}
	delete[] map;
	delete[] img;
}

extern "C" {
// -----------------------------------------------------------------------------
// this is to give the data to emscripten so that it can resize the div
EMSCRIPTEN_KEEPALIVE int  image_width() { return width_shrunk; }
EMSCRIPTEN_KEEPALIVE int  image_height() { return height_shrunk; }
EMSCRIPTEN_KEEPALIVE void set_size(int a) { size = a; }
EMSCRIPTEN_KEEPALIVE void set_color(int z) { color = z; }
}

// vim: ts=2
