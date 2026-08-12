#include <err.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
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

#define Pr .299
#define Pg .587
#define Pb .114

static int width_shrunk;
static int height_shrunk;
static int size  = 60;
static int color = false;

// stole this code from here: https://alienryderflex.com/saturation.html
static void changeSaturation(unsigned *R, unsigned *G, unsigned *B,
                             double change) {
	double P = sqrt((*R) * (*R) * Pr + //
	                (*G) * (*G) * Pg + //
	                (*B) * (*B) * Pb);

	*R = P + ((*R) - P) * change;
	*G = P + ((*G) - P) * change;
	*B = P + ((*B) - P) * change;
}

const struct option long_options[] = {
		{"version",       no_argument, NULL, 'v'},
		{  "image", required_argument, NULL, 'i'},
		{  "color",       no_argument, NULL, 'c'},
		{   "size", required_argument, NULL, 's'},
		{   "help",       no_argument, NULL, 'h'},
		{		 NULL,								 0, NULL,   0}
};

int main(int argc, char **argv) {
	int         opt = 0;
	// int         opt_index  = 0;
	const char *image_path = argv[1];
	while ((opt = getopt(argc, argv, "vi:cs:h")) != -1) {
		switch (opt) {
		case 'c':
			color = 1;
			// Both --color and -c are handled here
			break;
		case 'i': image_path = optarg; break;
		case 's': size = atoi(optarg); break;
		default: break;
		}
	}

	const char ASCIIMAP[] = "N@#W$9876543210?!abc;:+=-_,.  ";
	const int  num_char   = sizeof ASCIIMAP - 1;

	int      width, height, original_channels; // NOLINT
	stbi_uc *img = stbi_load(image_path, &width, &height, &original_channels, 3);
	if (!img) errx(1, "Failed to load image %s", image_path);

	const double scale_factor = size / fmax(height, width);
	width_shrunk              = (int)(width * scale_factor * 2);
	height_shrunk             = (int)(height * scale_factor);

	stbi_uc *map = malloc((long)width_shrunk * height_shrunk * 3UL);
	stbir_resize(img, width, height, 0, map, width_shrunk, height_shrunk, 0,
	             STBIR_RGB, STBIR_TYPE_UINT8, STBIR_EDGE_ZERO,
	             STBIR_FILTER_BOX); // stb the goat

	for (int y = 0; y != height_shrunk; y++) {
		for (int x = 0; x != width_shrunk; x++) {
			int      i = ((y * width_shrunk) + x) * 3;
			unsigned r = map[i + 0];
			unsigned g = map[i + 1];
			unsigned b = map[i + 2];
			changeSaturation(&r, &g, &b, 1.5);

			// what if change saturation causes some weird shit, this is to fix
			// that
			if (r > 255) {
				r = 255;
			} else if (r < 0) {
				r = 0;
			}
			if (g > 255) {
				g = 255;
			} else if (g < 0) {
				g = 0;
			}
			if (b > 255) {
				b = 255;
			} else if (b < 0) {
				b = 0;
			}

			double brightness = r * 0.2126 + g * 0.7152 + b * 0.0722;
			int    index      = (int)((brightness * (num_char - 1)) / 255);
			if (color) // checks if colour was toggled on or off
			{
				printf("\033[38;2;%d;%d;%dm%c\033[0m", (int)r, (int)g, (int)b,
				       ASCIIMAP[index]);

			} else {
				printf("%c", ASCIIMAP[index]);
			}
		}
		printf("\n");
	}
	free(map);
	free(img); // no ram hogging in here :D
}

// this is to give the data to emscripten so that it can resize the div
EMSCRIPTEN_KEEPALIVE int  image_width() { return width_shrunk; }
EMSCRIPTEN_KEEPALIVE int  image_height() { return height_shrunk; }
EMSCRIPTEN_KEEPALIVE void set_size(int a) { size = a; }
EMSCRIPTEN_KEEPALIVE void set_color(int z) { color = z; }

// i hope you did not find my comments annoying, jassi out :)
// vim: ts=2
