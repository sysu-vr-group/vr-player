
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>


#include "opengl.h"

#include "shader.h"
//#include "vertex.h"

bool playing = true;

#define CHECK_STATUS(T,O,S) do { \
  GLint status; \
  glGet##T##iv(O, S, &status); \
  if (GL_FALSE == status) { \
    GLint len; \
    glGet##T##iv(O, GL_INFO_LOG_LENGTH, &len); \
    char *log = (char *)malloc(len * sizeof(char)); \
    glGet##T##InfoLog(O, len, NULL, log); \
    fprintf(stderr, "[GL: %d]: %d, %s\n", __LINE__, S, log); \
    free(log); \
  } \
} while (0)



// GLFW callback function
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void error_callback(int error, const char *msg);


bool keys[1024] = { 0 };

float lastX = 1280.0 / 2.0;
float lastY = 720.0 / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


static GLuint create_shader(GLuint type, const char *source);
static GLuint create_program(GLuint vert, GLuint frag);

static GLuint create_sphere(GLuint num_slices, GLfloat radius, GLfloat **vertices,
	GLfloat **texcoords, GLuint **indices, GLuint *num_vertices_out);

void release_sphere_resource(GLfloat **vertices, GLfloat **texcoords, GLuint **indices);


/*******************************************
 *		GLFW callback function
 ******************************************/
bool log(const char *msg, ...)
{
	va_list argptr;
	va_start(argptr, msg);
	vfprintf(stdout, msg, argptr);
	va_end(argptr);

	return true;
}

void error_callback(int error, const char *msg)
{
	log("%s\n", msg);
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	gl_ctx *gl = (gl_ctx *)glfwGetWindowUserPointer(window);

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		playing = !playing;
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
	return;
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
	//LOG("mouse_callback: xpos = %f, ypos = %f\n", xpos, ypos);
	static double pre_x = xpos, pre_y = ypos;

	gl_ctx *ctx = (gl_ctx *)glfwGetWindowUserPointer(window);

	double xoffset = 0.0f, yoffset = 0.0f;
	xoffset = xpos - pre_x;
	yoffset = ypos - pre_y;

	opengl_euler_angle(xoffset, yoffset, ctx);

	pre_x = xpos;
	pre_y = ypos;

	return;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	gl_ctx *ctx = (gl_ctx *)glfwGetWindowUserPointer(window);
	
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

}

void window_size_callback(GLFWwindow *window, int width, int height) {
	LOG("width %i height %i\n", width, height);
	gl_ctx *ctx = (gl_ctx *)glfwGetWindowUserPointer(window);
	/* update any perspective matrices used here */
	ctx->width = width;
	ctx->height = height;
	glViewport(0, 0, ctx->width, ctx->height);

}


/*******************************************
 *		
 ******************************************/

gl_ctx *opengl_create_context()
{
	gl_ctx *ctx = new gl_ctx;
	memset(ctx, 0, sizeof(gl_ctx));

	return ctx;
}

int opengl_init(int width, int height, char *title, gl_ctx *ctx)
{
	// Init GLFW
	ctx->width = width;
	ctx->height = height;
	int len = strlen(title) + 1;
	ctx->title = new char [len];
	memset(ctx->title, 0x0, len);
	strcpy(ctx->title, title);

	//
	log("start GLFW %s\n", glfwGetVersionString());
	glfwSetErrorCallback(error_callback);
	
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);//GL_FALSE
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	ctx->window = glfwCreateWindow(ctx->width, ctx->height, ctx->title, NULL, NULL);

	// callback
	glfwSetKeyCallback(ctx->window, key_callback);
	glfwSetCursorPosCallback(ctx->window, mouse_callback);
	glfwSetMouseButtonCallback(ctx->window, mouse_button_callback);
	glfwSetWindowSizeCallback(ctx->window, window_size_callback);
	// GLFW Option
	glfwSetInputMode(ctx->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetWindowUserPointer(ctx->window, ctx);

	glfwMakeContextCurrent(ctx->window);

	glfwWindowHint(GLFW_SAMPLES, 4);

	/***************************************/

	GLuint vert, frag, prog;
	
	// GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	//
	glViewport(0, 0, width, height);
	// Setup OpenGL options
	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	//

	// Shader
	vert = create_shader(GL_VERTEX_SHADER, vertext_id);
	frag = create_shader(GL_FRAGMENT_SHADER, fragment_yuv_id);
	prog = create_program(vert, frag);

	ctx->program = prog;

	// vertices
	GLfloat *sphere_vertices = NULL;
	GLfloat *texcoords = NULL;
	GLuint 	*indices = NULL;
	GLuint num_vertices = 0;
	GLuint num_indices = 0;

	num_indices = create_sphere(200, 1.0, &sphere_vertices, &texcoords, &indices, &num_vertices);

	glGenVertexArrays(1, &(ctx->vao));
	glGenBuffers(1, &(ctx->vbo));
	glGenBuffers(1, &(ctx->ebo));
	glGenBuffers(1, &(ctx->tbo));

	glBindVertexArray(ctx->vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * num_vertices, sphere_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * num_indices, indices, GL_STATIC_DRAW);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(test_indices), test_indices, GL_STATIC_DRAW);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	// TBO
	glBindBuffer(GL_ARRAY_BUFFER, ctx->tbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * num_vertices, texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);


	// Release sphere resource
	ctx->num_indices = num_indices;
	ctx->num_vertices = num_vertices;
	release_sphere_resource(&sphere_vertices, &texcoords, &indices);


#if 0
	//
	model = glm::mat4(1.0f);
	model = glm::translate(model, sphere_pos);
	glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -3.0f);
	glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::vec3 sphere_pos = glm::vec3(0.0f, 0.0f, 3.0f);
#endif
	//glEnableVertexAttribArray(0);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

#if 0	// RGB
	// textures
	glGenTextures(1, &(ctx->textures[0]));
	glBindTexture(GL_TEXTURE_2D, ctx->textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ctx->image.width, ctx->image.height, 0, GL_RGB, GL_UNSIGNED_BYTE, ctx->image.data);
	//glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	//
#else // YUV
	// textures
	glGenTextures(3, ctx->textures);
	for (int i = 0; i < 3; i++) {
		glBindTexture(GL_TEXTURE_2D, ctx->textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ctx->image.width, ctx->image.height, 0, GL_RGB, GL_UNSIGNED_BYTE, ctx->image.data);
		//glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

#endif

	// init value
	ctx->fov = 45.0f;

	ctx->sphere_pos = glm::vec3(0.0f, 0.0f, 0.0f);
#if 0
	ctx->camera_pos = glm::vec3(0.0f, 0.0f, 1.0f);

	glm::vec3 cameraDirection = glm::normalize(ctx->camera_pos - ctx->sphere_pos);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));


	ctx->camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
	ctx->camera_up = glm::cross(cameraDirection, cameraRight);
#endif

	// ========================================================

	ctx->camera.camera_pos = glm::vec3(0.0f, 0.0f, 1.0f);
	ctx->camera.camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
	ctx->camera.camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
	ctx->camera.pitch = 0.0f;
	ctx->camera.yaw = 0.0f;


	// ========================================================
	ctx->model = glm::mat4(1.0f);
	ctx->view = glm::mat4(1.0f);
	ctx->proj = glm::mat4(1.0f);
	ctx->model = glm::translate(ctx->model, ctx->sphere_pos);

	ctx->model_location = glGetUniformLocation(ctx->program, "model");
	ctx->view_location = glGetUniformLocation(ctx->program, "view");
	ctx->proj_location = glGetUniformLocation(ctx->program, "proj");



	return 0;
}

double elapsed_seconds = 0.0f;
//static double previous_seconds = 0.0f;
int opengl_render(image *img, gl_ctx *ctx)
{
	while (!playing) {

	}
	static double previous_seconds = glfwGetTime();
	double current_seconds = glfwGetTime();
	elapsed_seconds = current_seconds - previous_seconds;
	previous_seconds = current_seconds;

	// Render
	window_updata_fps(ctx);

	// Clear the color buffer
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	

#if 0	// RGB
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ctx->textures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ctx->image.width, ctx->image.height, 0, GL_RGB, GL_UNSIGNED_BYTE, img->data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(ctx->program, "ourTexture1"), 0);

#else	// YUV

	// textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ctx->textures[0]);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ctx->image.width, ctx->image.height, 0, GL_RGB, GL_UNSIGNED_BYTE, img->data);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, img->width, img->height, 0, GL_RED, GL_UNSIGNED_BYTE, img->y);
	glUniform1i(glGetUniformLocation(ctx->program, "tex_y"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ctx->textures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, img->width, img->height, 0, GL_RED, GL_UNSIGNED_BYTE, img->u);
	glUniform1i(glGetUniformLocation(ctx->program, "tex_u"), 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, ctx->textures[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, img->width, img->height, 0, GL_RED, GL_UNSIGNED_BYTE, img->v);
	glUniform1i(glGetUniformLocation(ctx->program, "tex_v"), 2);

#endif	
	//glGenerateMipmap(GL_TEXTURE_2D);
	
	
	
	//glBindTexture(GL_TEXTURE_2D, 0);

	//
	glUseProgram(ctx->program);


	
	//

	ctx->view = glm::lookAt(ctx->camera.camera_pos, ctx->camera.camera_pos + ctx->camera.camera_front, ctx->camera.camera_up);
	
	ctx->proj = glm::perspective(ctx->fov, (float)ctx->width / (float)ctx->height, 0.01f, 100.0f);

#if 0
	//	model = glm::rotate(model, 50.0f, glm::vec3(0.5f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(ctx->program, "model"), 1, GL_FALSE, glm::value_ptr(ctx->model));
	glUniformMatrix4fv(glGetUniformLocation(ctx->program, "view"), 1, GL_FALSE, glm::value_ptr(ctx->view));
	glUniformMatrix4fv(glGetUniformLocation(ctx->program, "proj"), 1, GL_FALSE, glm::value_ptr(ctx->proj));
#else

	glUniformMatrix4fv(ctx->model_location, 1, GL_FALSE, glm::value_ptr(ctx->model));
	glUniformMatrix4fv(ctx->view_location, 1, GL_FALSE, glm::value_ptr(ctx->view));
	glUniformMatrix4fv(ctx->proj_location, 1, GL_FALSE, glm::value_ptr(ctx->proj));

#endif

	// Draw 
	glBindVertexArray(ctx->vao);
	glDrawElements(GL_TRIANGLES, ctx->num_indices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// GLFW
	glfwSwapBuffers(ctx->window);
	glfwPollEvents();
	
	return 0;
}

int opengl_euler_angle(float pitch, float yaw, gl_ctx *ctx)
{
	glm::vec3 front;
	GLfloat radius = 1.0f;
	float camera_speed = elapsed_seconds * 20;

	pitch = camera_speed * pitch;
	yaw = camera_speed * yaw;

	ctx->camera.pitch += pitch;
	ctx->camera.yaw += yaw;
	//radius = 0.2f;

	GLfloat cam_pitch = ctx->camera.pitch;
	GLfloat cam_yaw = ctx->camera.yaw;

	if (cam_yaw > 89.0f)
		cam_yaw = 89.0f;
	if (cam_yaw < -89.0f)
		cam_yaw = -89.0f;


	front.x = cos(glm::radians(cam_pitch)) * cos(glm::radians(cam_yaw));
	front.y = sin(glm::radians(cam_yaw));
	front.z = sin(glm::radians(cam_pitch)) * cos(glm::radians(cam_yaw));


	ctx->camera.camera_pos = radius * glm::normalize(front);
	ctx->camera.camera_front = glm::normalize(ctx->sphere_pos - ctx->camera.camera_pos);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::normalize(glm::cross(up, ctx->camera.camera_front));

	ctx->camera.camera_up = glm::cross(ctx->camera.camera_front, right);

	return 0;
}

int opengl_deinit(gl_ctx *ctx)
{
	// GLFW
	glfwDestroyWindow(ctx->window);
	glfwTerminate();
	if(ctx->title) {
		delete [] ctx->title;
	}

	//
	glDeleteTextures(3, ctx->textures);

	glDeleteVertexArrays(1, &(ctx->vao));
	glDeleteBuffers(1, &(ctx->vbo));
	glDeleteBuffers(1, &(ctx->ebo));
	glDeleteBuffers(1, &(ctx->tbo));

	delete(ctx);

	return 0;
}


int opengl_view_port(int x, int y, int w, int h)
{
	glViewport(x, y, w, h);
	return 0;
}


// GLFW
int window_is_close(gl_ctx *ctx)
{
	return glfwWindowShouldClose(ctx->window);
}

int window_updata_fps(gl_ctx *ctx)
{
	static double previous_seconds = glfwGetTime();
	static unsigned int frame_count;
	double current_seconds = glfwGetTime();
	double elapsed_seconds = current_seconds - previous_seconds;
	if (elapsed_seconds > 0.25) {
		previous_seconds = current_seconds;
		double fps = (double)frame_count / elapsed_seconds;
		char tmp[128];
		sprintf(tmp, "%s @ fps: %.2f", ctx->title, fps);
		glfwSetWindowTitle(ctx->window, tmp);
		frame_count = 0;
	}
	frame_count++;

	return 0;
}
//
static GLuint create_shader(GLuint type, const char *source)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar **)&source, NULL);
	glCompileShader(shader);

	// Check Error
	CHECK_STATUS(Shader, shader, GL_COMPILE_STATUS);


	return shader;
}

static GLuint create_program(GLuint vert, GLuint frag)
{
	GLuint prog = glCreateProgram();
	glAttachShader(prog, vert);
	glAttachShader(prog, frag);
	glLinkProgram(prog);

	// Check Error
	CHECK_STATUS(Program, prog, GL_LINK_STATUS);


	glDetachShader(prog, vert);
	glDetachShader(prog, frag);

	//
	glDeleteShader(vert);
	glDeleteShader(frag);

	return prog;
}

//
#define PI (3.14159265f)
static GLuint create_sphere(GLuint num_slices, GLfloat radius, GLfloat **vertices,
	GLfloat **texcoords, GLuint **indices, GLuint *num_vertices_out)
{
	int i, j;
	int num_parallels = num_slices / 2;
	int num_vertices = (num_parallels + 1) * (num_slices + 1);
	int num_indices = num_parallels * num_slices * 6;
	float angle_step = (2.0f*PI) / ((float)num_slices);
	
	// create
	if (vertices != NULL) {
		*vertices = (GLfloat*)malloc(sizeof(GLfloat) * 3 * num_vertices);
	}

	if (texcoords != NULL) {
		*texcoords = (GLfloat*)malloc(sizeof(GLfloat) * 2 * num_vertices);
	}
	
	if (indices != NULL) {
		*indices = (GLuint*)malloc(sizeof(GLuint) * num_indices);
	}

	for (i = 0; i < num_parallels + 1; i++) {
		for (j = 0; j < num_slices + 1; j++) {
			int vertex = (i*(num_slices + 1) + j) * 3;
			if (vertices) {
				(*vertices)[vertex + 0] = radius*sinf(angle_step*(float)i)*sinf(angle_step*(float)j);
				(*vertices)[vertex + 1] = radius*cosf(angle_step*(float)i);
				(*vertices)[vertex + 2] = radius*sinf(angle_step*(float)i)*cosf(angle_step*(float)j);
			}

			if (texcoords) {
				int texIndex = (i * (num_slices + 1) + j) * 2;
				(*texcoords)[texIndex + 0] = (float)j / (float)num_slices;
				(*texcoords)[texIndex + 1] = 1.0f - ((float)i / (float)num_parallels);
			}
		}
	}

	if (indices != NULL) {
		GLuint *index_buf = *indices;
		for (i = 0; i < num_parallels; i++) {
			for (j = 0; j < num_slices; j++) {
				*index_buf++ = i * (num_slices + 1) + j;
				*index_buf++ = (i + 1) * (num_slices + 1) + j;
				*index_buf++ = (i + 1) * (num_slices + 1) + (j + 1);

				*index_buf++ = i * (num_slices + 1) + j;
				*index_buf++ = (i + 1) * (num_slices + 1) + (j + 1);
				*index_buf++ = i * (num_slices + 1) + (j + 1);
			}
		}
	}

	if (num_vertices_out) {
		*num_vertices_out = num_vertices;
	}

	return num_indices;
}

void release_sphere_resource(GLfloat **vertices, GLfloat **texcoords, GLuint **indices)
{
	if (*vertices) {
		free(*vertices);
		*vertices = NULL;
	}

	if (*texcoords) {
		free(*texcoords);
		*texcoords = NULL;
	}

	if (*indices) {
		free(*indices);
		*indices = NULL;
	}

}
