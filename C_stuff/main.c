#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
  #include <emscripten.h>
#else
  #define EMSCRIPTEN_KEEPALIVE
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

int width_shrunk;
int height_shrunk;

int main (void){
  int width, height, original_channels;
  int x = 0;
  int y = 0;
  int i = 0;
  char ASCIIMAP[29] = "N@#W$9876543210?!abc;:+=-,._ ";
  int num_char = strlen(ASCIIMAP);
  int index;
  int img_index;
  unsigned char *img = stbi_load(
    "googoo.png",
    &width, &height, &original_channels, 3
  );
  width_shrunk = width;
  height_shrunk = height;
  while (width_shrunk > 40 && height_shrunk > 40) {
    width_shrunk = width_shrunk * 0.95;
    height_shrunk = height_shrunk * 0.95;
  }
  width_shrunk = width_shrunk * 2;
  unsigned char *img_resize = malloc(width_shrunk * height_shrunk * 3);
  float brightness;
  if (img == NULL) {
    printf("failure\n");
    free(img_resize);
    free(img);
    return 1;
  }
  else {
    stbir_resize_uint8_srgb(img, width, height, 0, img_resize, width_shrunk, height_shrunk, 0, STBIR_RGB);
  }
  printf("\n");
  while (y != height_shrunk) {
    while (x != width_shrunk) {
      img_index = (((y * width_shrunk) + x) * 3);
      brightness = ((img_resize[img_index + 0] * 0.2126) + (img_resize[img_index + 1] * 0.7152) + (img_resize[img_index + 2] * 0.0722));
      index = (int)((brightness * (num_char - 1))/255);
      printf("%c", ASCIIMAP[index]);
      x++;
      i++;
    }
    printf("\n");
    x = 0;
    y++;
  }
  free(img_resize);
  free(img);
}

EMSCRIPTEN_KEEPALIVE
int image_width() {
    return width_shrunk;
}

EMSCRIPTEN_KEEPALIVE
int image_height() {
    return height_shrunk;
}