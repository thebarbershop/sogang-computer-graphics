#define _CRT_SECURE_NO_WARNINGS

#define TO_RADIAN 0.01745329252f
#define TO_DEGREE 57.295779513f

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram;									  // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// #include glm/*.hpp only if necessary
// #include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
#include <glm/gtc/matrix_inverse.hpp>   //inverse, affineInverse, etc.
glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ViewProjectionMatrix, ViewMatrix, ProjectionMatrix;

glm::mat4 ModelMatrix_CAR_BODY, ModelMatrix_CAR_WHEEL, ModelMatrix_CAR_NUT, ModelMatrix_CAR_DRIVER;
glm::mat4 ModelMatrix_CAR_BODY_to_DRIVER; // computed only once in initialize_camera()

#include "Camera.h"
#include "Geometry.h"
#ifndef INT_MAX
#define INT_MAX INT32_MAX
#endif

float rotation_angle_car = 0.0f;

void draw_objects_in_world(void)
{
	// Removed
}

#define rad 1.7f
#define ww 1.0f
void draw_wheel_and_nut()
{
	// angle is used in Hierarchical_Car_Correct later
	int i;

	glUniform3f(loc_primitive_color, 0.000f, 0.808f, 0.820f); // color name: DarkTurquoise
	draw_geom_obj(GEOM_OBJ_ID_CAR_WHEEL);					  // draw wheel

	for (i = 0; i < 5; i++)
	{
		ModelMatrix_CAR_NUT = glm::rotate(ModelMatrix_CAR_WHEEL, TO_RADIAN * 72.0f * i, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix_CAR_NUT = glm::translate(ModelMatrix_CAR_NUT, glm::vec3(rad - 0.5f, 0.0f, ww));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix_CAR_NUT;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

		glUniform3f(loc_primitive_color, 0.690f, 0.769f, 0.871f); // color name: LightSteelBlue
		draw_geom_obj(GEOM_OBJ_ID_CAR_NUT);						  // draw i-th nut
	}
}

void draw_car_dummy(void)
{
	glUniform3f(loc_primitive_color, 0.498f, 1.000f, 0.831f); // color name: Aquamarine
	draw_geom_obj(GEOM_OBJ_ID_CAR_BODY);					  // draw body

	glLineWidth(2.0f);
	draw_axes(); // draw MC axes of body
	glLineWidth(1.0f);

	ModelMatrix_CAR_DRIVER = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(-3.0f, 0.5f, 2.5f));
	ModelMatrix_CAR_DRIVER = glm::rotate(ModelMatrix_CAR_DRIVER, TO_RADIAN * 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix_CAR_DRIVER;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(5.0f);
	draw_axes(); // draw camera frame at driver seat
	glLineWidth(1.0f);

	ModelMatrix_CAR_WHEEL = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(-3.9f, -3.5f, 4.5f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix_CAR_WHEEL;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_wheel_and_nut(); // draw wheel 0

	ModelMatrix_CAR_WHEEL = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(3.9f, -3.5f, 4.5f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix_CAR_WHEEL;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_wheel_and_nut(); // draw wheel 1

	ModelMatrix_CAR_WHEEL = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(-3.9f, -3.5f, -4.5f));
	ModelMatrix_CAR_WHEEL = glm::scale(ModelMatrix_CAR_WHEEL, glm::vec3(1.0f, 1.0f, -1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix_CAR_WHEEL;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_wheel_and_nut(); // draw wheel 2

	ModelMatrix_CAR_WHEEL = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(3.9f, -3.5f, -4.5f));
	ModelMatrix_CAR_WHEEL = glm::scale(ModelMatrix_CAR_WHEEL, glm::vec3(1.0f, 1.0f, -1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix_CAR_WHEEL;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_wheel_and_nut(); // draw wheel 3
}

/*********************************  START: callbacks *********************************/
int flag_draw_world_objects = 1;

void display(void)
{
	glm::mat4 ModelMatrix_big_cow, ModelMatrix_small_cow;
	glm::mat4 ModelMatrix_big_box, ModelMatrix_small_box;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ModelViewProjectionMatrix = ViewProjectionMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_floor();

	ModelMatrix_CAR_BODY = glm::rotate(glm::mat4(1.0f), -rotation_angle_car, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix_CAR_BODY = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(20.0f, 4.89f, 0.0f));
	ModelMatrix_CAR_BODY = glm::rotate(ModelMatrix_CAR_BODY, 90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));

	if (camera_type == CAMERA_DRIVER)
		set_ViewMatrix_for_driver();

	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix_CAR_BODY;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_car_dummy();

	ModelViewProjectionMatrix = glm::scale(ViewProjectionMatrix, glm::vec3(5.0f, 5.0f, 5.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	draw_axes();
	glLineWidth(1.0f);

	// ModelViewProjectionMatrix = ViewProjectionMatrix;
	// glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	// draw_path();

	if (flag_draw_world_objects)
		draw_objects_in_world();

	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'f': // Fill polygon
		polygonMode = 1;
		setPolygonMode();
		glutPostRedisplay();
		break;
	case 'l': // Draw only skeleton
		polygonMode = 0;
		setPolygonMode();
		glutPostRedisplay();
		break;
	case 'd': // Driver cam
		camera_type = CAMERA_DRIVER;
		glutPostRedisplay();
		break;
	case 'w': // World cam
		camera_type = CAMERA_WORLD_VIEWER;
		set_ViewMatrix_for_world_viewer();
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();
		break;
	case 'o': // World objects
		flag_draw_world_objects = 1 - flag_draw_world_objects;
		glutPostRedisplay();
		break;
	case 27:				 // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	}
}

int prevx, prevy;

void motion(int x, int y)
{
	if (!camera_wv.move | (camera_type != CAMERA_WORLD_VIEWER))
		return;

	renew_cam_position(prevy - y);
	renew_cam_orientation_rotation_around_v_axis(prevx - x);

	prevx = x;
	prevy = y;

	set_ViewMatrix_for_world_viewer();
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
	if ((button == GLUT_LEFT_BUTTON))
	{
		if (state == GLUT_DOWN)
		{
			camera_wv.move = 1;
			prevx = x;
			prevy = y;
		}
		else if (state == GLUT_UP)
			camera_wv.move = 0;
	}
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	camera_wv.aspect_ratio = (float)width / height;

	ProjectionMatrix = glm::perspective(TO_RADIAN * camera_wv.fovy, camera_wv.aspect_ratio, camera_wv.near_c, camera_wv.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	glutPostRedisplay();
}

void timer_scene(int timestamp_scene)
{
	rotation_angle_car = (timestamp_scene % 360) * TO_RADIAN;
	glutPostRedisplay();
	glutTimerFunc(100, timer_scene, (timestamp_scene + 1) % INT_MAX);
}

void cleanup(void)
{
	free_axes();
	free_path();
	free_floor();

	free_geom_obj(GEOM_OBJ_ID_CAR_BODY);
	free_geom_obj(GEOM_OBJ_ID_CAR_WHEEL);
	free_geom_obj(GEOM_OBJ_ID_CAR_NUT);
	free_geom_obj(GEOM_OBJ_ID_CAR_BODY);
	free_geom_obj(GEOM_OBJ_ID_COW);
	free_geom_obj(GEOM_OBJ_ID_TEAPOT);
	free_geom_obj(GEOM_OBJ_ID_BOX);
}
/*********************************  END: callbacks *********************************/

void register_callbacks(void)
{
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);
	glutTimerFunc(100, timer_scene, 0);
	glutCloseFunc(cleanup);
}

void prepare_shader_program(void)
{
	ShaderInfo shader_info[3] = {
		{GL_VERTEX_SHADER, "Shaders/simple.vert"},
		{GL_FRAGMENT_SHADER, "Shaders/simple.frag"},
		{GL_NONE, NULL}};

	h_ShaderProgram = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram, "u_primitive_color");
}

void initialize_OpenGL(void)
{
	setPolygonMode();
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	ViewMatrix = glm::mat4(1.0f);
	ProjectionMatrix = glm::mat4(1.0f);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	ModelMatrix_CAR_BODY = glm::mat4(1.0f);
	ModelMatrix_CAR_WHEEL = glm::mat4(1.0f);
	ModelMatrix_CAR_NUT = glm::mat4(1.0f);
}

void prepare_scene(void)
{
	prepare_axes();
	prepare_path();
	prepare_floor();
	prepare_geom_obj(GEOM_OBJ_ID_CAR_BODY, "Data/car_body_triangles_v.txt", GEOM_OBJ_TYPE_V);
	prepare_geom_obj(GEOM_OBJ_ID_CAR_WHEEL, "Data/car_wheel_triangles_v.txt", GEOM_OBJ_TYPE_V);
	prepare_geom_obj(GEOM_OBJ_ID_CAR_NUT, "Data/car_nut_triangles_v.txt", GEOM_OBJ_TYPE_V);
	prepare_geom_obj(GEOM_OBJ_ID_COW, "Data/cow_triangles_v.txt", GEOM_OBJ_TYPE_V);
	prepare_geom_obj(GEOM_OBJ_ID_TEAPOT, "Data/teapot_triangles_v.txt", GEOM_OBJ_TYPE_V);
	prepare_geom_obj(GEOM_OBJ_ID_BOX, "Data/box_triangles_v.txt", GEOM_OBJ_TYPE_V);
}

void initialize_renderer(void)
{
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
	initialize_camera();
}

void initialize_glew(void)
{
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void print_message(const char *m)
{
	fprintf(stdout, "%s\n\n", m);
}

void greetings(const char *program_name, char messages[][256], int n_message_lines)
{
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 2
int main(int argc, char *argv[])
{
	char program_name[64] = "Sogang CSE4170 4.7.1.Hier_Car_Dummy_Driver_GLSL";
	char messages[N_MESSAGE_LINES][256] = {"    - Keys used: 'f', l', 'd', 'w', 'o', 'ESC'",
										   "    - Mouse used: L-Click and move"};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(1200, 800);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutMainLoop();
}
