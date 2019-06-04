
#ifndef _JMGL_OPENGL_H_
#define _JMGL_OPENGL_H_


// OpenGL
#include "jmgl_headers.h"

#include "jmgl_image.h"


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


typedef struct jmgl_context
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
	jmgl_image *image;

	// GLFWwindow
	GLFWwindow *window;
	// window width and height
	int width;
	int height;
	char *title;
	void *user_data;

}gl_ctx;

gl_ctx *jmgl_opengl_create_context();
int jmgl_opengl_init(int width, int height, char *title, gl_ctx *ctx);
int jmgl_opengl_deinit(gl_ctx *ctx);

//
int jmgl_opengl_render(jmgl_image *img, gl_ctx *ctx);

int jmgl_opengl_euler_angle(float pitch, float yaw, gl_ctx *ctx);
//
//int jmgl_load_yuv_image(jmgl_image *image);



// GLFW
int jmgl_window_is_close(gl_ctx *ctx);
int jmgl_window_updata_fps(gl_ctx *ctx);


#endif // !_JMGL_OPENGL_H_
