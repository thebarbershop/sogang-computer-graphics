#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cfloat>
#include <random>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram;									  // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
#include <glm/gtc/matrix_inverse.hpp>   //inverse, affineInverse, etc.
glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ViewProjectionMatrix, ViewMatrix, ProjectionMatrix;
glm::mat4 ModelViewMatrix;

glm::mat4 ModelMatrix_CAR_BODY, ModelMatrix_CAR_WHEEL, ModelMatrix_CAR_NUT, ModelMatrix_CAR_DRIVER, ModelMatrix_SPIDER;
glm::mat4 ModelMatrix_CAR_BODY_to_DRIVER; // computed only once in initialize_camera()

glm::mat4 ModelMatrix_TIGER, ModelMatrix_TIGER_to_EYE;

#include "custommath.hpp"
#include "Camera.h"
#include "Geometry.h"
#ifndef INT_MAX
#define INT_MAX INT32_MAX
#endif
#ifndef UINT_MAX
#define UINT_MAX UINT32_MAX
#endif

typedef struct
{
	glm::vec3 pos;
	float angle;
} ObjectState;

std::vector<ObjectState> path_states;
std::vector<ObjectState> teapot_states;

void timer_scene(int);
const unsigned char ESC = 27;
unsigned int timestamp_scene; // the global clock in the scene
int flag_animation;
int flag_polygon_mode;
int flag_sub;
int flag_driver;
int flag_eye;

// for car animation
const float speed_car = 1.0f;
float rotation_angle_car;
glm::vec2 steering_angle_wheel; // .x: steering angle of left wheel, .y: steering angle of right wheel
float rotation_angle_wheel;
glm::vec3 position_car;

// for tiger animation
const float speed_tiger = 3.0f;
GLfloat rotation_angle_tiger;
GLfloat rotation_speed_tiger;
glm::vec3 position_tiger;
glm::vec3 goal_tiger;
glm::vec3 direction_tiger;
int flag_rotating_tiger;

// for ironman placement
glm::vec3 position_ironman;
GLfloat rotation_angle_ironman;

// for dragon animation
const float speed_dragon = 5.0f;
GLfloat rotation_angle_dragon;
GLfloat goal_distance_dragon;
glm::vec3 position_dragon;
glm::vec3 direction_dragon;
int flag_animation_dragon;
int flag_dragon;

// random number generators
std::random_device rd;
std::mt19937 gen(rd());

void prepare_animation(void)
{
	flag_animation = 1;

	position_tiger = glm::vec3(-0.8 * floor_size, -0.8 * floor_size, 0.0f);
	goal_tiger = glm::vec3(0.8f * floor_size, -0.8f * floor_size, 0.0f);
	direction_tiger = glm::normalize(goal_tiger - position_tiger);
	rotation_angle_tiger = glm::atan(direction_tiger.y, direction_tiger.x);
	rotation_speed_tiger = M_PIf32 / 6.0f;

	position_ironman = glm::vec3(0.5 * floor_size, 0.5 * floor_size, 0.0f);
	rotation_angle_ironman = glm::atan(position_ironman.y, position_ironman.x);

	flag_dragon = 1;
	std::uniform_real_distribution<GLfloat> position_dragon_random(-0.6f * floor_size, 0.6f * floor_size);
	position_dragon.x = position_dragon_random(gen);
	position_dragon.y = position_dragon_random(gen);
	std::uniform_real_distribution<GLfloat> goal_distance_dragon_random(20.0f * floor_size, 30.0f * floor_size);
	goal_distance_dragon = goal_distance_dragon_random(gen);
	std::uniform_real_distribution<GLfloat> rotation_angle_dragon_random(0, 2.0f * M_PIf32);
	rotation_angle_dragon = rotation_angle_dragon_random(gen);
	direction_dragon = glm::normalize(glm::vec3(1.0f, glm::tan(rotation_angle_dragon), 0.0f));
}

void setPolygonMode(const int mode)
{
	GLenum GLmode = mode ? GL_FILL : GL_LINE;
	glPolygonMode(polygonFace, GLmode);
}

const float rad = 1.7f;
const float ww = 1.0f;
void draw_wheel_and_nut(void)
{
	int i;

	glUniform3fv(loc_primitive_color, 1, color::black);
	draw_geom_obj(GEOM_OBJ_ID_CAR_WHEEL); // draw wheel

	for (i = 0; i < 5; ++i)
	{
		ModelMatrix_CAR_NUT = glm::rotate(ModelMatrix_CAR_WHEEL, glm::radians(72.0f) * i, glm::vec3(0.0f, 0.0f, 1.0f));
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

void draw_car_and_spider(void)
{
	glUniform3fv(loc_primitive_color, 1, color::aquamarine);
	draw_geom_obj(GEOM_OBJ_ID_CAR_BODY); // draw body

	glLineWidth(2.0f);
	draw_axes(); // draw MC axes of body
	glLineWidth(1.0f);

	ModelMatrix_CAR_DRIVER = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(-3.0f, 0.5f, 2.5f));
	ModelMatrix_CAR_DRIVER = glm::rotate(ModelMatrix_CAR_DRIVER, M_PI_2f32, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix_CAR_DRIVER;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(5.0f);
	draw_axes(); // draw camera frame at driver seat
	glLineWidth(1.0f);

	for (int i = 0; i < 4; ++i)
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

	ModelMatrix_SPIDER = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(0.0f, 3.2f, 0.0f));
	ModelMatrix_SPIDER = glm::rotate(ModelMatrix_SPIDER, M_PIf32, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix_SPIDER = glm::rotate(ModelMatrix_SPIDER, M_PI_2f32, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix_SPIDER;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_spider(); // draw spider
}

void draw_teapot(void)
{
	glUniform3fv(loc_primitive_color, 1, color::lavender);
	draw_geom_obj(GEOM_OBJ_ID_TEAPOT);
}

void draw_objects(void)
{
	glm::mat4 ModelMatrix_big_cow, ModelMatrix_small_cow;
	glm::mat4 ModelMatrix_big_box, ModelMatrix_small_box;

	// Draw Grid Floor
	ModelViewProjectionMatrix = ViewProjectionMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(3.0f);
	setPolygonMode(1);
	draw_floor();
	setPolygonMode(flag_polygon_mode);
	glLineWidth(1.0f);

	// Draw Axes
	ModelViewProjectionMatrix = glm::scale(ViewProjectionMatrix, glm::vec3(5.0f, 5.0f, 5.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(5.0f);
	draw_axes();
	glLineWidth(1.0f);

	// Draw Car
	ModelMatrix_CAR_BODY = glm::translate(glm::mat4(1.0f), position_car);
	ModelMatrix_CAR_BODY = glm::rotate(ModelMatrix_CAR_BODY, rotation_angle_car, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix_CAR_BODY = glm::translate(ModelMatrix_CAR_BODY, glm::vec3(0.0f, 0.0f, 4.89f));
	ModelMatrix_CAR_BODY = glm::rotate(ModelMatrix_CAR_BODY, -M_PIf32, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix_CAR_BODY = glm::rotate(ModelMatrix_CAR_BODY, M_PI_2f32, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix_CAR_BODY;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_car_and_spider();

	// Draw Tiger
	ModelMatrix_TIGER = glm::translate(glm::mat4(1.0f), position_tiger);
	ModelMatrix_TIGER = glm::rotate(ModelMatrix_TIGER, rotation_angle_tiger + M_PI_2f32, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix_TIGER = glm::scale(ModelMatrix_TIGER, glm::vec3(0.05f, 0.05f, 0.05f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix_TIGER;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_tiger();

	// Draw Ironman
	ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix, position_ironman + glm::vec3(0.0f, 0.0f, 0.5f));
	ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, rotation_angle_ironman, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_ironman();

	// Draw Bus
	for (auto &path_state : path_states)
	{
		ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix, path_state.pos);
		ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, path_state.angle + M_PI_2f32, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, M_PI_2f32, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_bus();
	}

	// Draw Teapot
	for (auto &teapot_state : teapot_states)
	{
		ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix, teapot_state.pos);
		ModelViewProjectionMatrix = glm::translate(ModelViewProjectionMatrix, glm::vec3(0.0f, 0.0f, 1.5f));
		ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, teapot_state.angle, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_teapot();
	}

	// Draw Dragon
	ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix, position_dragon);
	//	ModelViewProjectionMatrix = glm::translate(ModelViewProjectionMatrix, glm::vec3(0.0f, 0.0f, 1.5f));
	ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, rotation_angle_dragon, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_dragon();
}

/*********************************  START: callbacks *********************************/

void display(void)
{
	// Draw main window
	glViewport(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	glScissor(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	glEnable(GL_SCISSOR_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	set_ViewProjectionMatrix(camera_wv);
	draw_objects();

	int quarter_width = glutGet(GLUT_WINDOW_WIDTH) / 4;
	int quarter_height = glutGet(GLUT_WINDOW_HEIGHT) / 4;
	// Draw sub window
	if (flag_sub)
	{
		glViewport(0, 0, quarter_width, quarter_height);
		glScissor(0, 0, quarter_width, quarter_height);
		glEnable(GL_SCISSOR_TEST);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		set_ViewProjectionMatrix(camera_sub);
		draw_objects();
	}

	// Draw driver window
	if (flag_driver)
	{
		glViewport(0, 3 * quarter_height, quarter_width, quarter_height);
		glScissor(0, 3 * quarter_height, quarter_width, quarter_height);
		glEnable(GL_SCISSOR_TEST);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		set_ViewProjectionMatrix_for_driver();
		draw_objects();
	}

	// Draw eye window
	if (flag_eye)
	{
		glViewport(3 * quarter_width, 3 * quarter_height, quarter_width, quarter_height);
		glScissor(3 * quarter_width, 3 * quarter_height, quarter_width, quarter_height);
		glEnable(GL_SCISSOR_TEST);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		set_ViewProjectionMatrix_for_eye();
		draw_objects();
	}

	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
	if (flag_sub && (glutGetModifiers() & GLUT_ACTIVE_CTRL))
	{
		manipulate_sub_camera(key + 'a' - 1);
		return;
	}

	switch (key)
	{
	case '1': // Toggle sub window
		flag_sub = 1 - flag_sub;
		fprintf(stderr, "Sub camera %s\n", flag_sub ? "ON" : "OFF");
		glutPostRedisplay();
		break;
	case '2': // Toggle driver cam
		flag_driver = 1 - flag_driver;
		fprintf(stderr, "Driver camera %s.\n", flag_driver ? "ON" : "OFF");
		glutPostRedisplay();
		break;
	case '3': // Toggle eye cam
		flag_eye = 1 - flag_eye;
		fprintf(stderr, "Eye camera %s.\n", flag_eye ? "ON" : "OFF");
		glutPostRedisplay();
		break;
	case 'A':
	case 'a': // toggle the animation effect.
		flag_animation = 1 - flag_animation;
		if (flag_animation)
		{
			glutTimerFunc(100, timer_scene, 0);
		}
		fprintf(stderr, "Animation mode %s.\n", flag_animation ? "ON" : "OFF");
		break;
	case 'F':
	case 'f': // Toggle polygon fill mode
		flag_polygon_mode = 1 - flag_polygon_mode;
		setPolygonMode(flag_polygon_mode);
		glutPostRedisplay();
		fprintf(stderr, "Fill Polygon %s.\n", flag_polygon_mode ? "ON" : "OFF");
		break;
	case 'R':
	case 'r': // Reset World Cam position
		initialize_world_camera();
		glutPostRedisplay();
		fprintf(stderr, "Reset World Cam.\n");
		break;
	case ESC:
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	}
}

void special(int key, int x, int y)
{
	if (!(glutGetModifiers() & GLUT_ACTIVE_CTRL))
	{
		manipulate_world_camera(key);
	}

	if (flag_sub && (glutGetModifiers() & GLUT_ACTIVE_CTRL))
	{
		manipulate_sub_camera(key);
	}
	glutPostRedisplay();
}

int is_shift_down;
int prevx;

void motion(int x, int y)
{
	if (!camera_wv.move || !is_shift_down)
		return;

	renew_camera_fovy(camera_wv, prevx - x);
	prevx = x;
	set_ViewProjectionMatrix(camera_wv);

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
	// main window
	glViewport(0, 0, width, height);
	float aspect_ratio = (float)width / height;
	camera_wv.aspect_ratio = aspect_ratio;
	camera_sub.aspect_ratio = aspect_ratio;

	ProjectionMatrix = glm::perspective(glm::radians(camera_wv.fovy), camera_wv.aspect_ratio, camera_wv.near_c, camera_wv.far_c);
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
	next_position_car = glm::vec3(getButterflyCurve((timestamp_scene + 1) * speed_car));
	glm::vec3 direction_vec_car = next_position_car - previous_position_car;
	rotation_angle_car = std::atan2(direction_vec_car.y, direction_vec_car.x);
	glm::vec3 circumcenter = getCircumcenter(previous_position_car, position_car, next_position_car);
	int orientation = -getOrientation(previous_position_car, position_car, next_position_car);
	if (orientation == 0) // Car is going straight
	{
		steering_angle_wheel = glm::vec2(0.0f, 0.0f);
	}
	else // Car is steering
	{
		steering_angle_wheel = 1.0f * orientation * getSteeringAngle(circumcenter);
	}
	// Rotate the wheel according to moved distance. 1.7f is radius of wheel.
	rotation_angle_wheel += glm::distance(previous_position_car, position_car) / 1.7f;
	rotation_angle_wheel = normalizeAngle(rotation_angle_wheel);

	// Calculate position and frame of tiger
	static GLfloat direction_angle_tiger;
	static GLfloat cw_ccw_tiger = 1.0f; // if -1, counter clockwise turn; if 1, clockwise turn
	cur_frame_tiger = timestamp_scene % N_TIGER_FRAMES;
	if (flag_rotating_tiger)
	{
		rotation_angle_tiger = normalizeAngle(rotation_angle_tiger + cw_ccw_tiger * rotation_speed_tiger);
		if (glm::epsilonEqual(direction_angle_tiger, rotation_angle_tiger, rotation_speed_tiger - EPSILON))
		{
			flag_rotating_tiger = 0;
			rotation_angle_tiger = direction_angle_tiger;
		}
	}
	else
	{
		if (glm::distance(position_tiger, goal_tiger) < speed_tiger)
		{
			position_tiger = goal_tiger;
			glm::vec4 tmp_vec(goal_tiger, 1.0f);
			tmp_vec = glm::rotate(glm::mat4(1.0f), M_PI_2f32, glm::vec3(0.0f, 0.0f, 1.0f)) * tmp_vec;
			goal_tiger.x = tmp_vec.x;
			goal_tiger.y = tmp_vec.y;
			goal_tiger.z = tmp_vec.z;
			direction_tiger = glm::normalize(goal_tiger - position_tiger);
			direction_angle_tiger = normalizeAngle(glm::atan(direction_tiger.y, direction_tiger.x)); // angle between [0, 2*PI)
			cw_ccw_tiger = (normalizeAngle(direction_angle_tiger - rotation_angle_tiger) > M_PIf32 + EPSILON) ? -1 : 1;
			flag_rotating_tiger = 1;
		}
		else
		{
			position_tiger = position_tiger + speed_tiger * direction_tiger;
		}
	}
	glutPostRedisplay();
	if (flag_animation)
		glutTimerFunc(100, timer_scene, 0);
}

void cleanup(void)
{
	free_axes();
	free_floor();
	free_tiger();
	free_ironman();
	free_spider();
	free_dragon();
	free_bus();

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
	glutSpecialFunc(special);
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
	setPolygonMode(0);
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	ViewMatrix = glm::mat4(1.0f);
	ProjectionMatrix = glm::mat4(1.0f);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	ModelMatrix_CAR_BODY = glm::mat4(1.0f);
	ModelMatrix_CAR_WHEEL = glm::mat4(1.0f);
	ModelMatrix_CAR_NUT = glm::mat4(1.0f);
}

void initialize_object_states(void)
{
	ObjectState tmp_state;
	// Initialize path states
	int n_path_points = 240;
	float del = 12.0f * 180 / n_path_points;
	glm::vec3 prev_pos, next_pos;
	path_states.push_back({glm::vec3(0.0f, 0.0f, 0.0f), 0.0f});
	for (int i = 1; i < n_path_points; ++i)
	{
		prev_pos = path_states[i - 1].pos;
		tmp_state.pos = next_pos;
		next_pos = getButterflyCurve(del * i);

		glm::vec3 direction = next_pos - prev_pos;
		tmp_state.angle = std::atan2(direction.y, direction.x);
		path_states.push_back(tmp_state);
	}

	// Initialize teapot states
	for (int i = 0; i < 4; i++)
	{
		tmp_state.pos.x = ((i & 0b01) ? -1 : 1) * floor_size;
		tmp_state.pos.y = ((i & 0b10) ? -1 : 1) * floor_size;
		tmp_state.angle = std::atan2(-tmp_state.pos.y, -tmp_state.pos.x);
		teapot_states.push_back(tmp_state);
	}
}

void prepare_scene(void)
{
	prepare_animation();
	prepare_axes();
	prepare_floor();
	prepare_tiger();
	prepare_ironman();
	prepare_spider();
	prepare_dragon();
	prepare_bus();
	prepare_geom_obj(GEOM_OBJ_ID_CAR_BODY, "Data/car_body_triangles_v.txt", GEOM_OBJ_TYPE_V);
	prepare_geom_obj(GEOM_OBJ_ID_CAR_WHEEL, "Data/car_wheel_triangles_v.txt", GEOM_OBJ_TYPE_V);
	prepare_geom_obj(GEOM_OBJ_ID_CAR_NUT, "Data/car_nut_triangles_v.txt", GEOM_OBJ_TYPE_V);
	prepare_geom_obj(GEOM_OBJ_ID_COW, "Data/cow_triangles_v.txt", GEOM_OBJ_TYPE_V);
	prepare_geom_obj(GEOM_OBJ_ID_TEAPOT, "Data/teapot_triangles_v.txt", GEOM_OBJ_TYPE_V);
	prepare_geom_obj(GEOM_OBJ_ID_BOX, "Data/box_triangles_v.txt", GEOM_OBJ_TYPE_V);
	initialize_object_states();
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

	for (int i = 0; i < n_message_lines; ++i)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");
}

#define N_MESSAGE_LINES 2
int main(int argc, char *argv[])
{
	char program_name[64] = "HW3_20120085";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used: 'f', l', 'd', 'w', 'o', 'ESC'",
		"    - Mouse used: L-Click and move left/right"};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(1200, 800);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_glew();
	initialize_renderer();

	glutMainLoop();
}