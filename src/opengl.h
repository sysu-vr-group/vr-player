
#ifndef _OPENGL_H_
#define _OPENGL_H_


// OpenGL
#include "headers.h"
#include "hdr_rs.hpp"
#include "image.h"


#ifdef _DEBUG
#define LOG( ... )				printf( __VA_ARGS__ )
#else
#define LOG( ... )              
#endif

typedef struct _cam_view
{
	glm::vec3 camera_pos;
	glm::vec3 camera_front;
	glm::vec3 camera_up;

	GLfloat pitch;
	GLfloat yaw;
}cam_view;


typedef struct context
{
	//
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	GLuint tbo;
    GLuint num_indices;
    GLuint num_vertices;
	//
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;

	GLint	model_location;
	GLint	view_location;
	GLint	proj_location;
#if 0
	glm::vec3 camera_pos;
	glm::vec3 camera_front;
	glm::vec3 camera_up;
#endif

	glm::vec3 sphere_pos;

	cam_view camera;

	//
	GLuint textures[3];
	//
	GLfloat fov;
	//
	GLuint program;

	//
	image *image;

	// GLFWwindow
	GLFWwindow *window;
	// window width and height
	int width;
	int height;
	char *title;
	void *user_data;
}gl_ctx;

gl_ctx *opengl_create_context();
int opengl_init(int width, int height, char *title, gl_ctx *ctx);
int opengl_deinit(gl_ctx *ctx);

//
int opengl_render(image *img, gl_ctx *ctx);

int opengl_euler_angle(float pitch, float yaw, gl_ctx *ctx);
//
//int load_yuv_image(image *image);



// GLFW
int window_is_close(gl_ctx *ctx);
int window_updata_fps(gl_ctx *ctx);


#endif // !_OPENGL_H_
