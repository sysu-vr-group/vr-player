
#ifndef _FF_PLAYER_H_
#define _FF_PLAYER_H_
#include "pthread.h"
#include "queue"
// OpenGL
#include "image.h"

typedef struct player
{
	char		* url;
	void		* user_data;
	image	* img;
	//std::queue<image *> imgs;

	bool		is_exit;
	bool		is_pause;
	int64_t		cur_time;
	int64_t		duration;
	int64_t		base;
	pthread_t	play_thread;

}player_ctx;

player_ctx *player_create_context();
int player_init(char *url, player_ctx *ctx);
int player_deinit(player_ctx *ctx);

int player_set_exit(int is_exit, player_ctx *ctx);



#endif // !_FF_PLAYER_H_S
