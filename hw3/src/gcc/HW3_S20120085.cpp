#define _CRT_SECURE_NO_WARNINGS

#define TO_RADIAN 0.01745329252f
#define TO_DEGREE 57.295779513f

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cfloat>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram;									  // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables
GLint loc_ModelViewProjectionMatrix_TXPS, loc_ModelViewMatrix_TXPS, loc_ModelViewMatrixInvTrans_TXPS;

// #include glm/*.hpp only if necessary
// #include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
#include <glm/gtc/matrix_inverse.hpp>   //inverse, affineInverse, etc.
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/fast_trigonometry.hpp>
glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ViewProjectionMatrix, ViewMatrix, ProjectionMatrix;
glm::mat4 ModelViewMatrix;
glm::mat3 ModelViewMatrixInvTrans;

glm::mat4 ModelMatrix_CAR_BODY, ModelMatrix_CAR_WHEEL, ModelMatrix_CAR_NUT, ModelMatrix_CAR_DRIVER;
glm::mat4 ModelMatrix_CAR_BODY_to_DRIVER; // computed only once in initialize_camera()

#include "custommath.hpp"
#include "Camera.h"
#include "Geometry.h"
#ifndef INT_MAX
#define INT_MAX INT32_MAX
#endif
#ifndef UINT_MAX
#define UINT_MAX UINT32_MAX
#endif

void timer_scene(int);
unsigned int timestamp_scene = 0; // the global clock in the scene
int flag_animation = 1;

// for car animation
const float speed_car = 1.0f;
float rotation_angle_car;
glm::vec2 steering_angle_wheel; // .x: steering angle of left wheel, .y: steering angle of right wheel
float rotation_angle_wheel;
glm::vec3 position_car;

// for tiger animation
const float speed_tiger = 4.0f;
float rotation_angle_tiger = 0.0f;
glm::vec3 position_tiger;

#define rad 1.7f
#define ww 1.0f
void draw_wheel_and_nut()
{
	int i;

	glUniform3fv(loc_primitive_color, 1, color::black);
	draw_geom_obj(GEOM_OBJ_ID_CAR_WHEEL); // draw wheel

	for (i = 0; i < 5; i++)
	{
		ModelMatrix_CAR_NUT = glm::rotate(ModelMatrix_CAR_WHEEL, TO_RADIAN * 72.0f * i, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix_CAR_NUT = glm::translate(ModelMatrix_CAR_NUT, glm::vec3(rad - 0.5f, 0.0f, ww));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix_CAR_NUT;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

		glUniform3fv(loc_primitive_color, 1, color::light_steel_blue);
		draw_geom_obj(GEOM_OBJ_ID_CAR_NUT); // draw i-th nut
	}
}

const glm::vec3 car_wheel_position[4] = {
	glm::vec3(-3.9f, -3.5f, 4.5f),
	glm::vec3(3.9f, -3.5f, 4.5f),
	glm::vec3(-3.9f, -3.5f, -4.5f),
	glm::vec3(3.9f, -3.5f, -4.5f)};
void draw_car_dummy(void)
{
	glUniform3fv(loc_primitive_color, 1, color::aquamarine);
	draw_geom_obj(GEOM_OBJ_ID_CAR_BODY); // draw body

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

	for (int i = 0; i < 4; i++)
	{
		ModelMatrix_CAR_WHEEL = glm::translate(ModelMatrix_CAR_BODY, car_wheel_position[i]);
		if (i == WHEEL_FRONT_LEFT)
		{
			ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, steering_angle_wheel.x, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		if (i == WHEEL_FRONT_RIGHT)
		{
			ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, steering_angle_wheel.y, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		ModelMatrix_CAR_WHEEL = glm::rotate(ModelMatrix_CAR_WHEEL, rotation_angle_wheel, glm::vec3(0.0f, 0.0f, 1.0f));
		if (i == WHEEL_FRONT_RIGHT || i == WHEEL_REAR_RIGHT)
		{
			ModelMatrix_CAR_WHEEL = glm::scale(ModelMatrix_CAR_WHEEL, glm::vec3(1.0f, 1.0f, -1.0f));
		}
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix_CAR_WHEEL;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_wheel_and_nut(); // draw wheel 0~3
	}
}

/*********************************  START: callbacks *********************************/

void display(void)
{
	glm::mat4 ModelMatrix_big_cow, ModelMatrix_small_cow;
	glm::mat4 ModelMatrix_big_box, ModelMatrix_small_box;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw Grid Floor
	ModelViewProjectionMatrix = ViewProjectionMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(3.0f);
	draw_floor();
	glLineWidth(1.0f);

	// Draw Axes
	ModelViewProjectionMatrix = glm::scale(ViewProjectionMatrix, glm::vec3(5.0f, 5.0f, 5.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(5.0f);
	draw_axes();
	glLineWidth(1.0f);

	// Draw Car
	ModelMatrix_CAR_BODY = glm::translate(glm::mat4(1.0f), position_car);
	ModelMatrix_CAR_BODY = glm::rotate(ModelMatrix_CAR_BODY, rotation_angle_car, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix_CAR_BODY = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(0.0f, 4.89f, 0.0f));
	ModelMatrix_CAR_BODY = glm::rotate(ModelMatrix_CAR_BODY, 90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));

	if (camera_type == CAMERA_DRIVER)
		set_ViewMatrix_for_driver();

	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix_CAR_BODY;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_car_dummy();

	// Draw Tiger
	ModelViewMatrix = glm::translate(ViewMatrix, position_tiger);
	ModelViewMatrix = glm::rotate(ModelViewMatrix, rotation_angle_tiger, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(0.05f, 0.05f, 0.05f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_tiger();

	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a': // toggle the animation effect.
		flag_animation = 1 - flag_animation;
		if (flag_animation)
		{
			glutTimerFunc(100, timer_scene, 0);
			fprintf(stderr, "Animation mode ON.\n");
		}
		else
			fprintf(stderr, "Animation mode OFF.\n");
		break;
	case 'f': // Fill polygon
		polygonMode = 1;
		setPolygonMode();
		glutPostRedisplay();
		fprintf(stderr, "Fill Polygon.\n");
		break;
	case 'l': // Draw only skeleton
		polygonMode = 0;
		setPolygonMode();
		glutPostRedisplay();
		fprintf(stderr, "Draw Only Lines.\n");
		break;
	case 'd': // Driver cam
		camera_type = CAMERA_DRIVER;
		glutPostRedisplay();
		fprintf(stderr, "Driver Cam.\n");
		break;
	case 'w': // World cam
		camera_type = CAMERA_WORLD_VIEWER;
		set_ViewMatrix_for_world_viewer();
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();
		fprintf(stderr, "World Cam.\n");
		break;
	case 'n': // Next frame
		flag_animation = 0;
		timer_scene(0);
		glutPostRedisplay();
		fprintf(stderr, "Next frame.\n");
		break;
	case 'p': // Previous frame
		flag_animation = 0;
		timestamp_scene -= 2;
		timer_scene(0);
		glutPostRedisplay();
		fprintf(stderr, "Previous frame.\n");
		break;
	case 'r': // Reset World Cam position
		initialize_camera();
		set_ViewMatrix_for_world_viewer();
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();
		fprintf(stderr, "Reset World Cam position.\n");
		break;
	case 27:				 // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	}
}

int is_shift_down;
int prevx;

void motion(int x, int y)
{
	if (!camera_wv.move | (camera_type != CAMERA_WORLD_VIEWER))
		return;

	if (is_shift_down)
	{
		renew_cam_position(prevx - x);
	}

	else
	{
		renew_cam_orientation_rotation_around_v_axis(prevx - x);
	}

	prevx = x;

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
			is_shift_down = glutGetModifiers() & GLUT_ACTIVE_SHIFT;
			camera_wv.move = 1;
			prevx = x;
		}
		else if (state == GLUT_UP)
		{
			camera_wv.move = 0;
		}
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

void timer_scene(int value)
{

	timestamp_scene = (timestamp_scene + 1) % UINT_MAX;

	// Calculate position and frame of car
	static glm::vec3 previous_position_car;
	static glm::vec3 next_position_car;
	previous_position_car = position_car;
	position_car = next_position_car;
	next_position_car = getButterflyCurve((timestamp_scene + 1) * speed_car);
	glm::vec3 direction_vec_car = next_position_car - previous_position_car;
	rotation_angle_car = std::atan2(direction_vec_car.x, direction_vec_car.z);
	glm::vec3 circumcenter = getCircumcenter(previous_position_car, position_car, next_position_car);
	int orientation = getOrientation(previous_position_car, position_car, next_position_car);
	if (orientation == 0) // Car is going straight
	{
		steering_angle_wheel = glm::vec2(0.0f, 0.0f);
	}
	else // Car is steering
	{
		steering_angle_wheel = 1.0f * orientation * getSteeringAngle(circumcenter);
	}

	// Rotate the wheel according to moved distance.
	rotation_angle_wheel += glm::distance(previous_position_car, position_car) / 1.7f;
	if (rotation_angle_wheel > 2 * M_PI)
	{
		rotation_angle_wheel -= 2 * M_PI;
	}

	// Calculate position and frame of tiger
	//static float previous_vec_tan_tiger;
	cur_frame_tiger = timestamp_scene % N_TIGER_FRAMES;
	//position_tiger = getHeartCurve(timestamp_scene * speed_tiger);
	//glm::vec3 direction_vec_tiger = getHeartCurve((timestamp_scene + 1) * speed_tiger) - position_tiger;
	//float atan2 = std::atan2(direction_vec_tiger.x, direction_vec_tiger.z);
	//rotation_angle_tiger = 0.5f * (atan2 + previous_vec_tan_tiger);
	//previous_vec_tan_tiger = atan2;

	glutPostRedisplay();
	if (flag_animation)
		glutTimerFunc(100, timer_scene, 0);
}

void cleanup(void)
{
	free_axes();
	free_path();
	free_floor();
	free_tiger();

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
	prepare_tiger();
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

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 2
int main(int argc, char *argv[])
{
	char program_name[64] = "HW3_20120085";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used: 'f', l', 'd', 'w', 'o', 'ESC'",
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
