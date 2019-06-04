
#include "jmgl_opengl.h"

#include "jmgl_image.h"

#define FFMPEG_DEC		0
#define NV_INTEL_DEC	0
#define VIDEO_DEC		1

#if FFMPEG_DEC
#include "jmgl_ff_player.h"
#endif

#if NV_INTEL_DEC
#include "jmgl_nv_intel_dec.h"
#endif

#if VIDEO_DEC
#include "jmgl_video_dec.h"
#endif



#include <stdio.h>
#include <Windows.h>
#include <iostream>




int main(int argc, char **argv)
{
	int dec_type = 0;
	char *input = *(++argv);

	std::cout << "Simple Panorama player." << std::endl;
	std::cout <<  "\tBase on OpenGL, NV CUDA decode, Intel Media SDK and FFmpeg." << std::endl;
	std::cout << "Usage: jmgl_pano input_file" << std::endl;
	std::cout <<  "dec_type: decode by 0:auto, 1:nv, 2:intel or 3:ffmpeg" << std::endl;


	/***********************************************************/
	gl_ctx *gl = jmgl_opengl_create_context();
#if FFMPEG_DEC
	player_ctx *player = jmgl_player_create_context();
#elif NV_INTEL_DEC
	dec_ctx *ctx = jmgl_dec_create_context();
#elif VIDEO_DEC
	dec_ctx *ctx = jmgl_video_dec_create_context();
#endif


#if FFMPEG_DEC
	jmgl_player_init(input, player);
	//jmgl_player_init(argv[1], player);
	//jmgl_player_init("rtmp://localhost/live/test live=1", player);
	//jmgl_player_init("F:\\EarthMap_2500x1250.jpg", player);
#elif NV_INTEL_DEC
	jmgl_dec_init(input, dec_type, ctx);
#elif VIDEO_DEC
	jmgl_video_dec_init(input, dec_type, ctx);

#endif

	jmgl_opengl_init(1280, 720, "JMGL_PANO", gl);

	jmgl_image	* img = NULL;

#if FFMPEG_DEC
	img = player->img;
#elif NV_INTEL_DEC
	img = ctx->img;
#elif VIDEO_DEC
	img = ctx->img;
#endif

	while (!jmgl_window_is_close(gl))
	{
		jmgl_opengl_render(img, gl);
	}



	jmgl_opengl_deinit(gl);
#if FFMPEG_DEC
	jmgl_player_deinit(player);
#elif NV_INTEL_DEC
	jmgl_dec_deinit(ctx);
#elif VIDEO_DEC
	jmgl_video_dec_deinit(ctx);
#endif
	return 0;
}