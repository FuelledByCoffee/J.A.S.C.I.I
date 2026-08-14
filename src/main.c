#include <err.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef __EMSCRIPTEN__
	#include <emscripten.h>
#else
	#define EMSCRIPTEN_KEEPALIVE
#endif
#include <stb_image.h>
#include <stb_image_resize2.h>

typedef uint8_t u8;

typedef struct {
	u8 red;
	u8 green;
	u8 blue;
} pixel;

static int g_width_shrunk;
static int g_height_shrunk;
static int g_size  = 60;
static int g_color = false;

// -----------------------------------------------------------------------------
// stole this code from here: https://alienryderflex.com/saturation.html
static pixel changeSaturation(pixel p, double change) {
	const double Pr = .299;
	const double Pg = .587;
	const double Pb = .114;

	u8 R = p.red;
	u8 G = p.green;
	u8 B = p.blue;

	const double P = sqrt(R * R * Pr + //
	                      G * G * Pg + //
	                      B * B * Pb);

	p.red   = (u8)fmin(fabs(P + ((R)-P) * change), 255U);
	p.green = (u8)fmin(fabs(P + ((G)-P) * change), 255U);
	p.blue  = (u8)fmin(fabs(P + ((B)-P) * change), 255U);
	return p;
}

// -----------------------------------------------------------------------------
static pixel *load_image(const char *image_path) {
	int      width, height, original_channels; // NOLINT
	stbi_uc *img = stbi_load(image_path, &width, &height, &original_channels, 3);
	if (!img) errx(1, "Failed to load image %s", image_path);

	const double scale_factor = g_size / fmax(height, width);
	g_width_shrunk            = (int)(width * scale_factor * 2);
	g_height_shrunk           = (int)(height * scale_factor);

	pixel *img_resized =
			stbir_resize(img, width, height, 0, NULL, g_width_shrunk, g_height_shrunk,
	                 0, STBIR_RGB, STBIR_TYPE_UINT8, STBIR_EDGE_ZERO,
	                 STBIR_FILTER_BOX); // stb the goat
	free(img);
	return img_resized;
}

// -----------------------------------------------------------------------------
static void print_ascii_art(pixel *img, int height, int width) { // NOLINT
	const char ASCIIMAP[] = "N@#W$9876543210?!abc;:+=-_,.  ";
	const int  num_char   = sizeof ASCIIMAP - 1;

	for (int y = 0; y != height; y++) {
		for (int x = 0; x != width; x++) {
			const int   i = y * width + x;
			const pixel p = changeSaturation(img[i], 1.5);

			const double brightness = p.red * 0.2126 +   //
			                          p.green * 0.7152 + //
			                          p.blue * 0.0722;

			const int index = (int)brightness * (num_char - 1) / 255;
			if (g_color)
				printf("\033[38;2;%d;%d;%dm%c\033[0m", p.red, p.green, p.blue,
				       ASCIIMAP[index]);
			else putchar(ASCIIMAP[index]);
		}
		putchar('\n');
	}
}

// -----------------------------------------------------------------------------
static const struct option long_options[] = {
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
			case 'c': g_color = 1; break;
			case 'i': image_path = optarg; break;
			case 's': g_size = atoi(optarg); break;
			default: break;
		}
	}
	if (optind < argc) image_path = argv[optind];
	if (!image_path) errx(1, "Need image!");

	pixel *img = load_image(image_path);
	print_ascii_art(img, g_height_shrunk, g_width_shrunk);
	free(img);
}

// -----------------------------------------------------------------------------
// this is to give the data to emscripten so that it can resize the div
EMSCRIPTEN_KEEPALIVE int  image_width() { return g_width_shrunk; }
EMSCRIPTEN_KEEPALIVE int  image_height() { return g_height_shrunk; }
EMSCRIPTEN_KEEPALIVE void set_size(int a) { g_size = a; }
EMSCRIPTEN_KEEPALIVE void set_color(int z) { g_color = z; }

// vim: ts=2
