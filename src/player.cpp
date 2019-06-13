
#include "opengl.h"

#include "image.h"

#define FFMPEG_DEC		1

#if FFMPEG_DEC
#include "ff_player.h"
#endif



#include <stdio.h>
#include <Windows.h>
#include <iostream>



int main(int argc, char **argv)
{
	int dec_type = 0;
	char *input = *(++argv);

	std::cout << "Simple Panorama player." << std::endl;
	std::cout <<  "\tBase on OpenGL, FFmpeg decode." << std::endl;
	std::cout << "Usage: pano input_file" << std::endl;


	gl_ctx *gl = opengl_create_context();
#if FFMPEG_DEC
	player_ctx *player = player_create_context();
#endif


#if FFMPEG_DEC
	player_init(input, player);
	//player_init(argv[1], player);
	//player_init("rtmp://localhost/live/test live=1", player);
	//player_init("F:\\EarthMap_2500x1250.jpg", player);
#endif

	opengl_init(1280, 720, "PANO", gl);

	image	* img = NULL;

#if FFMPEG_DEC
	img = player->img;
#endif
	std::string str = player->url;
	int f = str.find("mp4"), pre_pts = -1;

	while (!window_is_close(gl))
	{
		if (f != -1) {
			if (img->pts == pre_pts) {
				continue;
			}
			else {
				pre_pts = img->pts;
				opengl_render(img, gl);
			}
		}
		else {
			opengl_render(img, gl);
		}

	}



	opengl_deinit(gl);
#if FFMPEG_DEC
	player_deinit(player);
#endif
	return 0;
}