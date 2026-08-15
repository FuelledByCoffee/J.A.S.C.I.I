#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <err.h>
#include <getopt.h>
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

namespace stbi {
struct image {
	image(u8 *data, int height, int width)
		: m_data(reinterpret_cast<pixel *>(data)), //
			m_height(height),                        //
			m_width(width) {}
	image(const image &other)
		: m_data(
					(pixel *)std::malloc(other.width() * other.height() * sizeof(pixel))),
			m_height(other.m_height), //
			m_width(other.m_width) {
		auto f = other.m_data;
		auto l = f + m_width * m_height;
		std::copy(f, l, m_data);
		std::cout << "Image copied!\n";
	}
	image(image &&other)
		: m_data(std::exchange(other.m_data, nullptr)), //
			m_height(other.m_height),                     //
			m_width(other.m_width) {}
	~image() { stbi_image_free(m_data); }

	auto operator[](int y, int x) -> pixel & { return m_data[m_width * y + x]; }
	auto width() const -> int { return m_width; }
	auto height() const -> int { return m_height; }

private:
	pixel *m_data   = nullptr;
	int    m_height = 0;
	int    m_width  = 0;
};
} // namespace stbi

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

	p.red   = u8(std::min(std::fabs(P + ((R)-P) * change), 255.));
	p.green = u8(std::min(std::fabs(P + ((G)-P) * change), 255.));
	p.blue  = u8(std::min(std::fabs(P + ((B)-P) * change), 255.));
	return p;
}

// -----------------------------------------------------------------------------
[[nodiscard]] static auto load_image(const char *image_path) -> stbi::image {
	int   width, height, original_channels; // NOLINT
	auto *img = stbi_load(image_path, &width, &height, &original_channels, 3);
	if (!img || original_channels != 3)
		errx(2, "Failed to load image %s", image_path);
	return {img, height, width};
}

// -----------------------------------------------------------------------------
[[nodiscard]] static auto make_ascii_art(stbi::image img) -> std::string {
	constexpr char ASCIIMAP[] = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/"
															"\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";
	constexpr int  num_chars  = sizeof ASCIIMAP - 1;

	std::stringstream art;
	const double      aspect_ratio           = double(img.height()) / img.width();
	constexpr double  char_aspect_adjustment = 0.55;
	g_target_height = int(g_target_width * aspect_ratio * char_aspect_adjustment);

	for (int y = 0; y != g_target_height; y++) {
		for (int x = 0; x != g_target_width; x++) {
			const int   row    = int(double(y) / g_target_height * img.height());
			const int   column = int(double(x) / g_target_width * img.width());
			const pixel p      = changeSaturation(img[row, column], 1.5);

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
