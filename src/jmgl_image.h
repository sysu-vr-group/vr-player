

#ifndef _JMGL_IMAGE_H_
#define _JMGL_IMAGE_H_

#include <stdint.h>

typedef struct jmgl_image {
	uint8_t *data;
	uint8_t *y;
	uint8_t *u;
	uint8_t *v;
	int width;
	int height;
	int64_t pts;
}jmgl_image;


#endif // !_JMGL_IMAGE_H_

