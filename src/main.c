#include <err.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <stb_image.h>
#ifdef __EMSCRIPTEN__
	#include <emscripten.h>
#else
	#define EMSCRIPTEN_KEEPALIVE
#endif

typedef uint8_t u8;

typedef struct {
	u8 red;
	u8 green;
	u8 blue;
} pixel;

static int g_target_height;
static int g_target_width = 90; // width
static int g_color        = false;

// -----------------------------------------------------------------------------
// this is to give the data to emscripten so that it can resize the div
EMSCRIPTEN_KEEPALIVE int  image_width() { return g_target_width; }
EMSCRIPTEN_KEEPALIVE int  image_height() { return g_target_height; }
EMSCRIPTEN_KEEPALIVE void set_size(int a) { g_target_width = a; }
EMSCRIPTEN_KEEPALIVE void set_color(int z) { g_color = z; }

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
static pixel *load_image(const char *image_path, int *height, int *width) {
	int    original_channels;
	pixel *img =
			(pixel *)stbi_load(image_path, width, height, &original_channels, 3);
	if (!img) errx(1, "Failed to load image %s", image_path);

	const double aspect_ratio           = (double)*height / *width;
	const double char_aspect_adjustment = 0.55;
	g_target_height =
			(int)(g_target_width * aspect_ratio * char_aspect_adjustment);

	return img;
}

// -----------------------------------------------------------------------------
static void print_ascii_art(pixel *img, int height, int width) { // NOLINT
	// const char ASCIIMAP[] = "N@#W$9876543210?!abc;:+=-_,.  ";
	const char ASCIIMAP[] = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/"
													"\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";
	const int  num_char   = sizeof ASCIIMAP - 1;

	for (int y = 0; y != g_target_height; y++) {
		for (int x = 0; x != g_target_width; x++) {
			const int   recalc_x = (int)((double)x / g_target_width * width);
			const int   recalc_y = (int)((double)y / g_target_height * height);
			const int   i        = recalc_y * width + recalc_x;
			const pixel p = changeSaturation(img[i], 1.5);

			const double brightness = 0.299 * p.red +   //
			                          0.587 * p.green + //
			                          0.114 * p.blue;

			const int index = (int)((brightness / 255.0) * (num_char - 1));
			if (g_color)
				printf("\033[38;2;%d;%d;%dm%c\033[0m", p.red, p.green, p.blue,
				       ASCIIMAP[index]);
			else putchar(ASCIIMAP[index]);
		}
		putchar('\n');
	}
}

// -----------------------------------------------------------------------------
int main(int argc, char **argv) {

	const struct option long_options[] = {
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
			case 'c': g_color = 1; break;
			case 'i': image_path = optarg; break;
			case 's': g_target_width = atoi(optarg); break;
			default: break;
		}
	}
	if (optind < argc) image_path = argv[optind];
	if (!image_path) errx(1, "Need image!");

	int    height, width; // NOLINT
	pixel *img = load_image(image_path, &height, &width);
	print_ascii_art(img, height, width);
	free(img);
}

// vim: ts=2
