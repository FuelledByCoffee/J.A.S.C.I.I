#include <algorithm>
#include <cstdlib>
#include <err.h>
#include <getopt.h>
#include <memory>
#include <sstream>
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

static int g_target_width = 160;
static int g_target_height;
static int color = false;

// -----------------------------------------------------------------------------
// this is to give the data to emscripten so that it can resize the div
extern "C" {
EMSCRIPTEN_KEEPALIVE int  image_width() { return g_target_width; }
EMSCRIPTEN_KEEPALIVE int  image_height() { return g_target_height; }
EMSCRIPTEN_KEEPALIVE void set_size(int a) { g_target_width = a; }
EMSCRIPTEN_KEEPALIVE void set_color(int z) { color = z; }
}

using u8 = std::uint8_t;

struct pixel {
	u8 red;
	u8 green;
	u8 blue;
};

template <typename pixel_type = pixel, //
          typename free_func  = decltype(&stbi_image_free)>
struct image {
	using u_ptr = std::unique_ptr<pixel_type, free_func>;
	image(u8 *data, int height, int width, free_func deleter = stbi_image_free)
		: m_data(reinterpret_cast<pixel_type *>(data), deleter), m_height(height),
			m_width(width) {}
	u_ptr m_data;
	int   m_height = 0;
	int   m_width  = 0;

	pixel_type &operator[](int i) { return m_data.get()[i]; }
};

// -----------------------------------------------------------------------------
// stole this code from here: https://alienryderflex.com/saturation.html
[[nodiscard]]
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
[[nodiscard]]
static auto load_image(const char *image_path) {
	int   width, height, original_channels; // NOLINT
	auto *img = stbi_load(image_path, &width, &height, &original_channels, 3);
	if (!img) errx(2, "Failed to load image %s", image_path);
	return image(img, height, width, stbi_image_free);
}

// -----------------------------------------------------------------------------
static auto make_ascii_art(auto &&img) {
	constexpr char ASCIIMAP[] = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/"
															"\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";
	constexpr int  num_chars  = sizeof ASCIIMAP - 1;

	std::stringstream art;
	const double      aspect_ratio           = double(img.m_height) / img.m_width;
	constexpr double  char_aspect_adjustment = 0.55;
	g_target_height =
			(int)(g_target_width * aspect_ratio * char_aspect_adjustment);

	for (int y = 0; y != g_target_height; y++) {
		for (int x = 0; x != g_target_width; x++) {
			const int   column = int(double(x) / g_target_width * img.m_width);
			const int   row    = int(double(y) / g_target_height * img.m_height);
			const int   i      = row * img.m_width + column;
			const pixel p      = changeSaturation(img[i], 1.5);

			const double brightness = 0.299 * p.red +   //
			                          0.587 * p.green + //
			                          0.114 * p.blue;

			const unsigned index = unsigned(brightness / 255.0 * (num_chars - 1));
			if (color)
				art << std::format("\033[38;2;{:d};{:d};{:d}m{}\033[0m", //
				                   p.red, p.green, p.blue, ASCIIMAP[index]);
			else art << ASCIIMAP[index];
		}
		art << '\n';
	}
	return art.str();
}

// -----------------------------------------------------------------------------
int main(int argc, char **argv) {

	constexpr struct option long_options[] = {
			{"version",       no_argument, NULL, 'v'},
			{  "image", required_argument, NULL, 'i'},
			{  "color",       no_argument, NULL, 'c'},
			{   "size", required_argument, NULL, 's'},
			{   "help",       no_argument, NULL, 'h'},
			{     NULL,								 0, NULL,   0}
  };

	int         opt        = 0;
	const char *image_path = argv[1];
	while ((opt = getopt_long(argc, argv, "vi:cs:h", long_options, NULL)) != -1) {
		switch (opt) {
			case 'c': color = 1; break;
			case 'i': image_path = optarg; break;
			case 's': g_target_width = atoi(optarg); break;
			default: break;
		}
	}
	if (optind < argc) image_path = argv[optind];
	if (!image_path) errx(1, "Need an image");

	auto img = load_image(image_path);
	std::cout << make_ascii_art(std::move(img));
}

// vim: ts=2
