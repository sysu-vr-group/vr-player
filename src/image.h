

#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdint.h>

typedef struct image {
	uint8_t *data;
	uint8_t *y;
	uint8_t *u;
	uint8_t *v;
	int width;
	int height;
	int64_t pts;
}image;


#endif // !_IMAGE_H_

