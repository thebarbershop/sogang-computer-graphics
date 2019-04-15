#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram;									  // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, ortho, etc.
glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;

#define TO_RADIAN 0.01745329252f
#define TO_DEGREE 57.295779513f
#define BUFFER_OFFSET(offset) ((GLvoid *)(offset))
#define LOC_VERTEX 0

//// Function headers ////
void prepare_scene(void);
void cleanup(void);
//////////////////////////

//// CUSTUM CONSTANTS ////
const GLfloat BACKGROUND_COLOR[] = {173 / 255.0f, 255 / 255.0f, 47 / 255.0f}; // R,G,B value of background color (0-255)
const unsigned int MIN_SPEED = 5;
const unsigned int MAX_SPEED = 20;
const unsigned int INITIAL_WIDTH = 1280;
const unsigned int INITIAL_HEIGHT = 800;
const unsigned int REFRESH_RATE = (unsigned int)(1000 / 30); // == 30 fps
const GLfloat SWORD_DIRECTION_MIN = 60 * TO_RADIAN;
const GLfloat SWORD_DIRECTION_MAX = 120 * TO_RADIAN;
const GLfloat SWORD_SPEED_MIN = 7.5f;
const GLfloat SWORD_SPEED_MAX = 12.5f;
const float epsilon = 0.0005f;
//////////////////////////

//// PRE-DEFINED VARIABLES ////
int win_width = 0,
	win_height = 0;
///////////////////////////////

//// CUSTUM VARIABLES ////
unsigned int car_speed = 10; // car moves 15 per tick
GLfloat sword_speed = 10;
bool pause = false;
int n_heart = 3;
bool boom_flag = false;
bool gameover_flag = false;

// random number generators
std::random_device rd;
std::mt19937 gen(rd());
//////////////////////////

//// CUSTUM FUNCTIONS ////
GLfloat current_angle(void)
{
	return std::atan(1.0f * win_height / win_width);
}

glm::vec2 move(const glm::vec2 vec, const GLfloat angle, const GLfloat displacement)
{
	// move the vector by given angle and displacement
	GLfloat x1 = vec.x + displacement * std::cos(angle);
	GLfloat y1 = vec.y + displacement * std::sin(angle);
	return glm::vec2(x1, y1);
}

bool fequals(const float a, const float b)
{
	return (std::abs(a - b) < epsilon);
}
////

//// DESIGN ROAD ////
const unsigned int ROAD_MAIN = 0;
const unsigned int ROAD_LINE_TOP = 1;
const unsigned int ROAD_LINE_BOTTOM = 2;

GLfloat road_main_shape[4][2] = {
	{-1.0f, 0.3f},
	{1.0f, 0.3f},
	{1.0f, -0.3f},
	{-1.0f, -0.3f},
};
GLfloat road_line_top_shape[4][2] = {
	{-1.0f, 0.3f},
	{1.0f, 0.3f},
	{1.0f, 0.25f},
	{-1.0f, 0.25f},
};
GLfloat road_line_bottom_shape[4][2] = {
	{-1.0f, -0.3f},
	{1.0f, -0.3f},
	{1.0f, -0.25f},
	{-1.0f, -0.25f},
};

GLfloat road_color[3][3] = {
	{0x70 / 255.0f, 0x80 / 255.0f, 0x90 / 255.0f},
	{0xFF / 255.0f, 0xFA / 255.0f, 0xFA / 255.0f},
	{0xFF / 255.0f, 0xFA / 255.0f, 0xFA / 255.0f},
};

GLuint VBO_road, VAO_road;

void prepare_road()
{
	GLsizeiptr buffer_size = sizeof(road_main_shape) + sizeof(road_line_top_shape) + sizeof(road_line_bottom_shape);

	glGenBuffers(1, &VBO_road);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_road);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(road_main_shape), road_main_shape);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(road_main_shape), sizeof(road_line_top_shape), road_line_top_shape);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(road_main_shape) + sizeof(road_line_top_shape), sizeof(road_line_bottom_shape), road_line_bottom_shape);

	glGenVertexArrays(1, &VAO_road);
	glBindVertexArray(VAO_road);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_road);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_road()
{
	glBindVertexArray(VAO_road);

	glUniform3fv(loc_primitive_color, 1, road_color[ROAD_MAIN]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, road_color[ROAD_LINE_TOP]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, road_color[ROAD_LINE_BOTTOM]);
	glDrawArrays(GL_TRIANGLE_FAN, 4 + 4, 4);

	glBindVertexArray(0);
}
//// DESIGN ROAD END ////

//// DESIGN HOUSE ////
#define HOUSE_ROOF 0
#define HOUSE_BODY 1
#define HOUSE_CHIMNEY 2
#define HOUSE_DOOR 3
#define HOUSE_WINDOW 4

GLfloat roof[3][2] = {{-12.0f, 0.0f}, {0.0f, 12.0f}, {12.0f, 0.0f}};
GLfloat house_body[4][2] = {{-12.0f, -14.0f}, {-12.0f, 0.0f}, {12.0f, 0.0f}, {12.0f, -14.0f}};
GLfloat chimney[4][2] = {{6.0f, 6.0f}, {6.0f, 14.0f}, {10.0f, 14.0f}, {10.0f, 2.0f}};
GLfloat door[4][2] = {{-8.0f, -14.0f}, {-8.0f, -8.0f}, {-4.0f, -8.0f}, {-4.0f, -14.0f}};
GLfloat window[4][2] = {{4.0f, -6.0f}, {4.0f, -2.0f}, {8.0f, -2.0f}, {8.0f, -6.0f}};

GLfloat house_color[5][3] = {
	{200 / 255.0f, 39 / 255.0f, 42 / 255.0f},
	{235 / 255.0f, 225 / 255.0f, 196 / 255.0f},
	{255 / 255.0f, 0 / 255.0f, 0 / 255.0f},
	{233 / 255.0f, 113 / 255.0f, 23 / 255.0f},
	{44 / 255.0f, 180 / 255.0f, 49 / 255.0f},
};

const size_t n_house = 4;
std::vector<glm::vec2> house_positions;
const std::vector<GLfloat> house_initial_positions = {
	0.5f,
	0.475f,
	0.45f,
	0.425f,
	0.4f,
	0.375f,
	-0.5f,
	-0.475f,
	-0.45f,
	-0.425f,
	-0.4f,
	-0.375f,
}; // initial positions, propotional to win_width and win_height

std::vector<glm::vec2> house_scales;
const glm::vec2 house_initial_scale = glm::vec2(3.0f, 3.0f);

GLuint VBO_house, VAO_house;
void prepare_house()
{
	GLsizeiptr buffer_size = sizeof(roof) + sizeof(house_body) + sizeof(chimney) + sizeof(door) + sizeof(window);
	std::uniform_real_distribution<> house_displacement_random(-0.5f * win_width, 0.5f * win_width);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_house);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_house);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(roof), roof);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof), sizeof(house_body), house_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof) + sizeof(house_body), sizeof(chimney), chimney);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof) + sizeof(house_body) + sizeof(chimney), sizeof(door), door);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof) + sizeof(house_body) + sizeof(chimney) + sizeof(door),
					sizeof(window), window);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_house);
	glBindVertexArray(VAO_house);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_house);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Initialize house position
	std::vector<GLfloat> tmp_house_positions(house_initial_positions);
	std::shuffle(tmp_house_positions.begin(), tmp_house_positions.end(), gen);
	for (size_t i = 0; i < n_house; ++i)
	{
		house_positions.push_back(glm::vec2(house_displacement_random(gen), tmp_house_positions[i] * win_height));
		house_scales.push_back(glm::vec2(house_initial_scale.x, house_initial_scale.y));
	}
}

void draw_house()
{
	glBindVertexArray(VAO_house);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_ROOF]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 3);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 3, 4);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_CHIMNEY]);
	glDrawArrays(GL_TRIANGLE_FAN, 7, 4);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_DOOR]);
	glDrawArrays(GL_TRIANGLE_FAN, 11, 4);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_WINDOW]);
	glDrawArrays(GL_TRIANGLE_FAN, 15, 4);

	glBindVertexArray(0);
}
//// DESIGN HOUSE END ////

//// DESIGN CAR2 ////
//draw car2
#define CAR2_BODY 0
#define CAR2_FRONT_WINDOW 1
#define CAR2_BACK_WINDOW 2
#define CAR2_FRONT_WHEEL 3
#define CAR2_BACK_WHEEL 4
#define CAR2_LIGHT1 5
#define CAR2_LIGHT2 6

GLfloat car2_body[8][2] = {{-18.0f, -7.0f}, {-18.0f, 0.0f}, {-13.0f, 0.0f}, {-10.0f, 8.0f}, {10.0f, 8.0f}, {13.0f, 0.0f}, {18.0f, 0.0f}, {18.0f, -7.0f}};
GLfloat car2_front_window[4][2] = {{-10.0f, 0.0f}, {-8.0f, 6.0f}, {-2.0f, 6.0f}, {-2.0f, 0.0f}};
GLfloat car2_back_window[4][2] = {{0.0f, 0.0f}, {0.0f, 6.0f}, {8.0f, 6.0f}, {10.0f, 0.0f}};
GLfloat car2_front_wheel[8][2] = {{-11.0f, -11.0f}, {-13.0f, -8.0f}, {-13.0f, -7.0f}, {-11.0f, -4.0f}, {-7.0f, -4.0f}, {-5.0f, -7.0f}, {-5.0f, -8.0f}, {-7.0f, -11.0f}};
GLfloat car2_back_wheel[8][2] = {{7.0f, -11.0f}, {5.0f, -8.0f}, {5.0f, -7.0f}, {7.0f, -4.0f}, {11.0f, -4.0f}, {13.0f, -7.0f}, {13.0f, -8.0f}, {11.0f, -11.0f}};
GLfloat car2_light1[3][2] = {{-18.0f, -1.0f}, {-17.0f, -2.0f}, {-18.0f, -3.0f}};
GLfloat car2_light2[3][2] = {{-18.0f, -4.0f}, {-17.0f, -5.0f}, {-18.0f, -6.0f}};

GLfloat car2_color[7][3] = {
	{0xF0 / 255.0f, 0xFF / 255.0f, 0xFF / 255.0f},
	{235 / 255.0f, 219 / 255.0f, 208 / 255.0f},
	{235 / 255.0f, 219 / 255.0f, 208 / 255.0f},
	{0 / 255.0f, 0 / 255.0f, 0 / 255.0f},
	{0 / 255.0f, 0 / 255.0f, 0 / 255.0f},
	{249 / 255.0f, 244 / 255.0f, 0 / 255.0f},
	{249 / 255.0f, 244 / 255.0f, 0 / 255.0f}};

glm::vec2 car2_position;
GLfloat car2_displacement = -.4f;
glm::vec2 car2_scale = glm::vec2(4.0f, 4.0f);
const unsigned int CAR2_TIMER_MAX = 30;
unsigned int car2_timer;
int car2_coeff;

GLuint VBO_car2, VAO_car2;
void prepare_car2()
{
	GLsizeiptr buffer_size = sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window) + sizeof(car2_front_wheel) + sizeof(car2_back_wheel) + sizeof(car2_light1) + sizeof(car2_light2);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_car2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_car2);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(car2_body), car2_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body), sizeof(car2_front_window), car2_front_window);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window), sizeof(car2_back_window), car2_back_window);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window), sizeof(car2_front_wheel), car2_front_wheel);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window) + sizeof(car2_front_wheel),
					sizeof(car2_back_wheel), car2_back_wheel);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window) + sizeof(car2_front_wheel) + sizeof(car2_back_wheel), sizeof(car2_light1), car2_light1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window) + sizeof(car2_front_wheel) + sizeof(car2_back_wheel) + sizeof(car2_light1), sizeof(car2_light2), car2_light2);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_car2);
	glBindVertexArray(VAO_car2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_car2);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Initialize car object position
	car2_position = move(car2_position, 0, car2_displacement * win_width);
	car2_timer = CAR2_TIMER_MAX;
	car2_coeff = 1;
}

void draw_car2()
{
	glBindVertexArray(VAO_car2);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 8);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_FRONT_WINDOW]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_BACK_WINDOW]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_FRONT_WHEEL]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 8);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_BACK_WHEEL]);
	glDrawArrays(GL_TRIANGLE_FAN, 24, 8);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_LIGHT1]);
	glDrawArrays(GL_TRIANGLE_FAN, 32, 3);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_LIGHT2]);
	glDrawArrays(GL_TRIANGLE_FAN, 35, 3);

	glBindVertexArray(0);
}
//// DESIGN CAR2 END ////

//// DESIGN SWORD ////
const unsigned int SWORD_BODY = 0;
const unsigned int SWORD_BODY2 = 1;
const unsigned int SWORD_HEAD = 2;
const unsigned int SWORD_HEAD2 = 3;
const unsigned int SWORD_IN = 4;
const unsigned int SWORD_DOWN = 5;
const unsigned int SWORD_BODY_IN = 6;

GLfloat sword_body[4][2] = {{-6.0f, 0.0f}, {-6.0f, -4.0f}, {6.0f, -4.0f}, {6.0f, 0.0f}};
GLfloat sword_body2[4][2] = {{-2.0f, -4.0f}, {-2.0f, -6.0f}, {2.0f, -6.0f}, {2.0f, -4.0f}};
GLfloat sword_head[4][2] = {{-2.0f, 0.0f}, {-2.0f, 16.0f}, {2.0f, 16.0f}, {2.0f, 0.0f}};
GLfloat sword_head2[3][2] = {{-2.0f, 16.0f}, {0.0f, 19.46f}, {2.0f, 16.0f}};
GLfloat sword_in[4][2] = {{-0.3f, 0.7f}, {-0.3f, 15.3f}, {0.3f, 15.3f}, {0.3f, 0.7f}};
GLfloat sword_down[4][2] = {{-2.0f, -6.0f}, {2.0f, -6.0f}, {4.0f, -8.0f}, {-4.0f, -8.0f}};
GLfloat sword_body_in[4][2] = {{0.0f, -1.0f}, {1.0f, -2.732f}, {0.0f, -4.464f}, {-1.0f, -2.732f}};

GLfloat sword_color[7][3] = {
	{139 / 255.0f, 69 / 255.0f, 19 / 255.0f},
	{139 / 255.0f, 69 / 255.0f, 19 / 255.0f},
	{155 / 255.0f, 155 / 255.0f, 155 / 255.0f},
	{155 / 255.0f, 155 / 255.0f, 155 / 255.0f},
	{0 / 255.0f, 0 / 255.0f, 0 / 255.0f},
	{139 / 255.0f, 69 / 255.0f, 19 / 255.0f},
	{255 / 255.0f, 0 / 255.0f, 0 / 255.0f}};

GLuint VBO_sword, VAO_sword;

glm::vec2 sword_scale(5.0f, 5.0f);
GLfloat sword_direction;
glm::vec2 sword_position;

void prepare_sword()
{
	GLsizeiptr buffer_size = sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2) + sizeof(sword_in) + sizeof(sword_down) + sizeof(sword_body_in);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_sword);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_sword);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sword_body), sword_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body), sizeof(sword_body2), sword_body2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2), sizeof(sword_head), sword_head);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head), sizeof(sword_head2), sword_head2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2), sizeof(sword_in), sword_in);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2) + sizeof(sword_in), sizeof(sword_down), sword_down);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2) + sizeof(sword_in) + sizeof(sword_down), sizeof(sword_body_in), sword_body_in);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_sword);
	glBindVertexArray(VAO_sword);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_sword);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Initialize sword position out of screen
	// Actual position is randomly assigned by sword_timer
	sword_position = glm::vec2(win_width, win_height);
	sword_direction = 90 * TO_RADIAN;
}

void draw_sword()
{
	glBindVertexArray(VAO_sword);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_BODY2]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_HEAD]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_HEAD2]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 3);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_IN]);
	glDrawArrays(GL_TRIANGLE_FAN, 15, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_DOWN]);
	glDrawArrays(GL_TRIANGLE_FAN, 19, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_BODY_IN]);
	glDrawArrays(GL_TRIANGLE_FAN, 23, 4);

	glBindVertexArray(0);
}
//// DESIGN SWORD END ////

//// DESIGN CAKE ////
const unsigned int CAKE_FIRE = 0;
const unsigned int CAKE_CANDLE = 1;
const unsigned int CAKE_BODY = 2;
const unsigned int CAKE_BOTTOM = 3;
const unsigned int CAKE_DECORATE = 4;

GLfloat cake_fire[4][2] = {{-0.5f, 14.0f}, {-0.5f, 13.0f}, {0.5f, 13.0f}, {0.5f, 14.0f}};
GLfloat cake_candle[4][2] = {{-1.0f, 8.0f}, {-1.0f, 13.0f}, {1.0f, 13.0f}, {1.0f, 8.0f}};
GLfloat cake_body[4][2] = {{8.0f, 5.0f}, {-8.0f, 5.0f}, {-8.0f, 8.0f}, {8.0f, 8.0f}};
GLfloat cake_bottom[4][2] = {{-10.0f, 1.0f}, {-10.0f, 5.0f}, {10.0f, 5.0f}, {10.0f, 1.0f}};
GLfloat cake_decorate[4][2] = {{-10.0f, 0.0f}, {-10.0f, 1.0f}, {10.0f, 1.0f}, {10.0f, 0.0f}};

GLfloat cake_color[5][3] = {
	{255 / 255.0f, 0 / 255.0f, 0 / 255.0f},
	{255 / 255.0f, 204 / 255.0f, 0 / 255.0f},
	{255 / 255.0f, 102 / 255.0f, 255 / 255.0f},
	{255 / 255.0f, 102 / 255.0f, 255 / 255.0f},
	{102 / 255.0f, 51 / 255.0f, 0 / 255.0f}};

GLuint VBO_cake, VAO_cake;

glm::vec2 cake_position;
const glm::vec2 cake_scale(2.5f, 2.5f);

void prepare_cake()
{
	int size = sizeof(cake_fire);
	GLsizeiptr buffer_size = sizeof(cake_fire) * 5;

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_cake);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cake);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, size, cake_fire);
	glBufferSubData(GL_ARRAY_BUFFER, size, size, cake_candle);
	glBufferSubData(GL_ARRAY_BUFFER, size * 2, size, cake_body);
	glBufferSubData(GL_ARRAY_BUFFER, size * 3, size, cake_bottom);
	glBufferSubData(GL_ARRAY_BUFFER, size * 4, size, cake_decorate);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_cake);
	glBindVertexArray(VAO_cake);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cake);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Initialize cake position
	cake_position = glm::vec2(0.45f * win_width, 0.45f * win_height);
}

void draw_cake()
{
	glBindVertexArray(VAO_cake);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_FIRE]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_CANDLE]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_BOTTOM]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_DECORATE]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);

	glBindVertexArray(0);
}
//// DESIGN CAKE END ////

//// DESIGN CAR ////
const unsigned int CAR_BODY = 0;
const unsigned int CAR_FRAME = 1;
const unsigned int CAR_WINDOW = 2;
const unsigned int CAR_LEFT_LIGHT = 3;
const unsigned int CAR_RIGHT_LIGHT = 4;
const unsigned int CAR_LEFT_WHEEL = 5;
const unsigned int CAR_RIGHT_WHEEL = 6;

GLfloat car_body[4][2] = {{-16.0f, -8.0f}, {-16.0f, 0.0f}, {16.0f, 0.0f}, {16.0f, -8.0f}};
GLfloat car_frame[4][2] = {{-10.0f, 0.0f}, {-10.0f, 10.0f}, {10.0f, 10.0f}, {10.0f, 0.0f}};
GLfloat car_window[4][2] = {{-8.0f, 0.0f}, {-8.0f, 8.0f}, {8.0f, 8.0f}, {8.0f, 0.0f}};
GLfloat car_left_light[4][2] = {{-9.0f, -6.0f}, {-10.0f, -5.0f}, {-9.0f, -4.0f}, {-8.0f, -5.0f}};
GLfloat car_right_light[4][2] = {{9.0f, -6.0f}, {8.0f, -5.0f}, {9.0f, -4.0f}, {10.0f, -5.0f}};
GLfloat car_left_wheel[4][2] = {{-10.0f, -12.0f}, {-10.0f, -8.0f}, {-6.0f, -8.0f}, {-6.0f, -12.0f}};
GLfloat car_right_wheel[4][2] = {{6.0f, -12.0f}, {6.0f, -8.0f}, {10.0f, -8.0f}, {10.0f, -12.0f}};

GLfloat car_color[7][3] = {
	{0 / 255.0f, 149 / 255.0f, 159 / 255.0f},
	{0 / 255.0f, 149 / 255.0f, 159 / 255.0f},
	{216 / 255.0f, 208 / 255.0f, 174 / 255.0f},
	{249 / 255.0f, 244 / 255.0f, 0 / 255.0f},
	{249 / 255.0f, 244 / 255.0f, 0 / 255.0f},
	{21 / 255.0f, 30 / 255.0f, 26 / 255.0f},
	{21 / 255.0f, 30 / 255.0f, 26 / 255.0f}};

GLuint VBO_car, VAO_car;

glm::vec2 car_scale(10.0f, 10.0f);
GLfloat car_angle;

void prepare_car()
{
	GLsizeiptr buffer_size = sizeof(car_body) + sizeof(car_frame) + sizeof(car_window) + sizeof(car_left_light) + sizeof(car_right_light) + sizeof(car_left_wheel) + sizeof(car_right_wheel);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_car);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_car);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(car_body), car_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car_body), sizeof(car_frame), car_frame);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car_body) + sizeof(car_frame), sizeof(car_window), car_window);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car_body) + sizeof(car_frame) + sizeof(car_window), sizeof(car_left_light), car_left_light);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car_body) + sizeof(car_frame) + sizeof(car_window) + sizeof(car_left_light),
					sizeof(car_right_light), car_right_light);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car_body) + sizeof(car_frame) + sizeof(car_window) + sizeof(car_left_light) + sizeof(car_right_light), sizeof(car_left_wheel), car_left_wheel);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car_body) + sizeof(car_frame) + sizeof(car_window) + sizeof(car_left_light) + sizeof(car_right_light) + sizeof(car_left_wheel), sizeof(car_right_wheel), car_right_wheel);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_car);
	glBindVertexArray(VAO_car);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_car);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_car()
{
	glBindVertexArray(VAO_car);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_FRAME]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_WINDOW]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_LEFT_LIGHT]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_RIGHT_LIGHT]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_LEFT_WHEEL]);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

	glUniform3fv(loc_primitive_color, 1, car_color[CAR_RIGHT_WHEEL]);
	glDrawArrays(GL_TRIANGLE_FAN, 24, 4);

	glBindVertexArray(0);
}
//// DESIGN CAR END ////

//// DESIGN BOOM ////
const unsigned int BOOM_OUTER = 0;
const unsigned int BOOM_INNER = 1;

GLfloat boom_outer_shape[18][2] = {
	{0.0f, 0.0f},
	{2.22f, 6.58f},
	{0.0f, 3.0f},
	{-4.17f, 7.34f},
	{-2.21f, 1.84f},
	{-8.66f, 2.42f},
	{-2.89f, -0.82f},
	{-8.01f, -6.47f},
	{-1.58f, -2.55f},
	{-0.62f, -9.07f},
	{0.55f, -2.95f},
	{5.8f, -7.66f},
	{2.39f, -1.81f},
	{9.38f, -0.45f},
	{2.99f, 0.27f},
	{6.41f, 4.87f},
	{2.02f, 2.21f},
	{2.22f, 6.58f},
};
GLfloat boom_inner_shape[18][2] = {
	{0.0f, 0.0f},
	{0.965f, 3.68f},
	{-0.035f, 1.5f},
	{-2.81f, 3.91f},
	{-1.08f, 1.04f},
	{-4.5f, 2.06f},
	{-1.5f, -0.035f},
	{-4.64f, -2.83f},
	{-1.04f, -1.08f},
	{1.4f, -4.21f},
	{0.19f, -1.37f},
	{4.78f, -2.84f},
	{1.08f, -1.03f},
	{5.0f, 0.92f},
	{1.5f, 0.035f},
	{2.53f, 3.31f},
	{1.04f, 1.08f},
	{0.965f, 3.68f},
};

GLfloat boom_color[2][3] = {
	{0xDC / 255.0f, 0x14 / 255.0f, 0x3C / 255.0f},
	{0x8B / 255.0f, 0x00 / 255.0f, 0x00 / 255.0f},
};

glm::vec2 boom_scale;
unsigned int boom_counter;

const GLfloat BOOM_SCALE_MIN = 8.0f;
const GLfloat BOOM_SCALE_MAX = 16.0f;
const unsigned int BOOM_COUNTER_MAX = 5;
int boom_coeff;
const GLfloat boom_tick = (BOOM_SCALE_MAX - BOOM_SCALE_MIN) / (BOOM_COUNTER_MAX - 1);

GLuint VBO_boom, VAO_boom;

void prepare_boom()
{
	GLsizeiptr buffer_size = sizeof(boom_outer_shape) + sizeof(boom_inner_shape);

	glGenBuffers(1, &VBO_boom);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_boom);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boom_outer_shape), boom_outer_shape);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(boom_outer_shape), sizeof(boom_inner_shape), boom_inner_shape);

	glGenVertexArrays(1, &VAO_boom);
	glBindVertexArray(VAO_boom);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_boom);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Initialize boom counter
	boom_counter = BOOM_COUNTER_MAX;
	boom_scale = glm::vec2(BOOM_SCALE_MIN, BOOM_SCALE_MIN);
	boom_coeff = 1;
}

void draw_boom()
{
	glBindVertexArray(VAO_boom);

	glUniform3fv(loc_primitive_color, 1, boom_color[BOOM_OUTER]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 18);

	glUniform3fv(loc_primitive_color, 1, boom_color[BOOM_INNER]);
	glDrawArrays(GL_TRIANGLE_FAN, 18, 18);

	glBindVertexArray(0);
}
//// DESIGN BOOM END ////

//// DESIGN AIRPLANE ////
const unsigned int AIRPLANE_BIG_WING = 0;
const unsigned int AIRPLANE_SMALL_WING = 1;
const unsigned int AIRPLANE_BODY = 2;
const unsigned int AIRPLANE_BACK = 3;
const unsigned int AIRPLANE_SIDEWINDER1 = 4;
const unsigned int AIRPLANE_SIDEWINDER2 = 5;
const unsigned int AIRPLANE_CENTER = 6;
GLfloat big_wing[6][2] = {{0.0f, 0.0f}, {-20.0f, 15.0f}, {-20.0f, 20.0f}, {0.0f, 23.0f}, {20.0f, 20.0f}, {20.0f, 15.0f}};
GLfloat small_wing[6][2] = {{0.0f, -18.0f}, {-11.0f, -12.0f}, {-12.0f, -7.0f}, {0.0f, -10.0f}, {12.0f, -7.0f}, {11.0f, -12.0f}};
GLfloat body[5][2] = {{0.0f, -25.0f}, {-6.0f, 0.0f}, {-6.0f, 22.0f}, {6.0f, 22.0f}, {6.0f, 0.0f}};
GLfloat back[5][2] = {{0.0f, 25.0f}, {-7.0f, 24.0f}, {-7.0f, 21.0f}, {7.0f, 21.0f}, {7.0f, 24.0f}};
GLfloat sidewinder1[5][2] = {{-20.0f, 10.0f}, {-18.0f, 3.0f}, {-16.0f, 10.0f}, {-18.0f, 20.0f}, {-20.0f, 20.0f}};
GLfloat sidewinder2[5][2] = {{20.0f, 10.0f}, {18.0f, 3.0f}, {16.0f, 10.0f}, {18.0f, 20.0f}, {20.0f, 20.0f}};
GLfloat center[1][2] = {{0.0f, 0.0f}};
GLfloat airplane_color[7][3] = {
	{150 / 255.0f, 129 / 255.0f, 183 / 255.0f}, // big_wing
	{245 / 255.0f, 211 / 255.0f, 0 / 255.0f},   // small_wing
	{111 / 255.0f, 85 / 255.0f, 157 / 255.0f},  // body
	{150 / 255.0f, 129 / 255.0f, 183 / 255.0f}, // back
	{245 / 255.0f, 211 / 255.0f, 0 / 255.0f},   // sidewinder1
	{245 / 255.0f, 211 / 255.0f, 0 / 255.0f},   // sidewinder2
	{255 / 255.0f, 0 / 255.0f, 0 / 255.0f}		// center
};

GLuint VBO_airplane, VAO_airplane;

glm::vec2 airplane_position;
glm::vec2 airplane_scale(1.5f, 1.5f);
GLfloat airplane_direction;
GLfloat airplane_speed;
unsigned int airplane_counter;

const unsigned int AIRPLANE_COUNTER_MAX = 10;
const GLfloat AIRPLANE_SCALE_MIN = 1.0f;
const GLfloat AIRPLANE_SCALE_MAX = 6.0f;

void prepare_airplane()
{
	GLsizeiptr buffer_size = sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back) + sizeof(sidewinder1) + sizeof(sidewinder2) + sizeof(center);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_airplane);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_airplane);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(big_wing), big_wing);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing), sizeof(small_wing), small_wing);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing), sizeof(body), body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body), sizeof(back), back);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back),
					sizeof(sidewinder1), sidewinder1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back) + sizeof(sidewinder1), sizeof(sidewinder2), sidewinder2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back) + sizeof(sidewinder1) + sizeof(sidewinder2), sizeof(center), center);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_airplane);
	glBindVertexArray(VAO_airplane);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_airplane);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Initialize airplane position and angle
	std::uniform_real_distribution<GLfloat> airplane_initial_position_random(-1.0f / 3, 1.0f / 6);
	GLfloat x = airplane_initial_position_random(gen) * win_width;
	GLfloat y = airplane_initial_position_random(gen) * win_height;
	airplane_position = glm::vec2(x, y);

	std::uniform_real_distribution<GLfloat> airplane_initial_angle_random(0.0f, 360 * TO_RADIAN);
	airplane_direction = airplane_initial_angle_random(gen);
	airplane_counter = AIRPLANE_COUNTER_MAX;
	airplane_speed = 5.0f;
}

void draw_airplane()
{ // Draw airplane in its MC.
	glBindVertexArray(VAO_airplane);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_BIG_WING]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_SMALL_WING]);
	glDrawArrays(GL_TRIANGLE_FAN, 6, 6);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_BACK]);
	glDrawArrays(GL_TRIANGLE_FAN, 17, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_SIDEWINDER1]);
	glDrawArrays(GL_TRIANGLE_FAN, 22, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_SIDEWINDER2]);
	glDrawArrays(GL_TRIANGLE_FAN, 27, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_CENTER]);
	glPointSize(5.0);
	glDrawArrays(GL_POINTS, 32, 1);
	glPointSize(1.0);
	glBindVertexArray(0);
}
//// DESIGN AIRPLANE END ////

void display(void)
{
	glm::mat4 ModelMatrix;

	glClear(GL_COLOR_BUFFER_BIT);

	// draw road object
	ModelMatrix = glm::mat4(1.0f);
	ModelMatrix = glm::rotate(ModelMatrix, current_angle(), glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(win_width, win_height, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_road();

	// draw house objects
	for (size_t i = 0; i < n_house; i++)
	{
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::rotate(ModelMatrix, current_angle(), glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(house_positions[i].x, house_positions[i].y, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(house_scales[i].x, house_scales[i].y, 1.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_house();
	}

	// draw car2 object
	ModelMatrix = glm::mat4(1.0f);
	ModelMatrix = glm::rotate(ModelMatrix, current_angle(), glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(car2_position.x, car2_position.y, 0.0f));
	if (boom_flag || gameover_flag)
	{
		ModelMatrix = glm::rotate(ModelMatrix, 1.5f * current_angle(), glm::vec3(0.0f, 0.0f, 1.0f));
	}
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(car2_scale.x, car2_scale.y, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_car2();

	// draw boom object
	if (boom_flag || gameover_flag)
	{
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::rotate(ModelMatrix, current_angle(), glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(car2_position.x, car2_position.y, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(boom_scale.x, boom_scale.y, 1.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_boom();
	}

	// draw sword object
	ModelMatrix = glm::mat4(1.0f);
	ModelMatrix = glm::rotate(ModelMatrix, current_angle(), glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(sword_position.x, sword_position.y, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, sword_direction - 90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(sword_scale.x, sword_scale.y, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_sword();

	// draw cake objects
	for (int i = 0; i < n_heart; ++i)
	{
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(cake_position.x - i * 16 * 1.5 * cake_scale.x, cake_position.y, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(cake_scale.x, cake_scale.y, 1.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_cake();
	}

	// draw airplane object
	ModelMatrix = glm::mat4(1.0f);
	ModelMatrix = glm::rotate(ModelMatrix, current_angle(), glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(airplane_position.x, airplane_position.y, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, airplane_direction, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(airplane_scale.x, airplane_scale.y, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_airplane();

	// draw gameover scene
	if (gameover_flag)
	{
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::rotate(ModelMatrix, car_angle, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(car_scale.x, car_scale.y, 1.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_car();
	}

	glFlush();
}

void timer(int value)
{
	// check for gameover
	if (gameover_flag)
	{
		car_speed = 0;
		car_scale = 1.01f * car_scale;
		car_angle = car_angle + 10 * TO_RADIAN;
		if (car_angle > 360 * TO_RADIAN)
		{
			car_angle -= 360 * TO_RADIAN;
			// stop rotating if gameover scene is big enough
			if (car_scale.x >= 100)
			{
				car_angle = 0;
				glutPostRedisplay();
				pause = true;
				return;
			}
		}
	}

	// check for pause
	if (pause)
	{
		glutTimerFunc(REFRESH_RATE, timer, 0);
		return;
	}

	// check for boom
	if (glm::length(car2_position - sword_position) < 100.0f)
	{
		if (!boom_flag)
		{
			boom_flag = true;
			--n_heart;
			if (n_heart == 0)
			{
				gameover_flag = true;
			}
		}
	}
	else
	{
		boom_flag = false;
		boom_counter = BOOM_COUNTER_MAX;
		boom_scale = glm::vec2(BOOM_SCALE_MIN, BOOM_SCALE_MIN);
	}

	// re-scale boom
	if (boom_flag)
	{
		--boom_counter;
		boom_scale += glm::vec2(boom_coeff * boom_tick, boom_coeff * boom_tick);
		if (!boom_counter)
		{
			boom_counter = BOOM_COUNTER_MAX;
			boom_coeff *= -1;
		}
	}

	// Move car2
	car2_timer -= 1;
	if (!car2_timer)
	{
		car2_timer = CAR2_TIMER_MAX;
		car2_coeff *= -1;
	}
	car2_position.x += car2_coeff * (int)car_speed / 10.0f;

	// Move house
	if (!gameover_flag)
	{
		for (size_t i = 0; i < n_house; ++i)
		{
			house_scales[i] = 1.001f * house_scales[i];
			house_positions[i].x -= car_speed;
			if (house_positions[i].x < -0.6f * win_width)
			{
				std::uniform_int_distribution<size_t> house_initial_random(0, house_initial_positions.size() - 1);
				house_positions[i] = glm::vec2(0.6f * win_width, house_initial_positions[house_initial_random(gen)] * win_height);
				house_scales[i] = house_initial_scale;
			}
		}
	}

	// Move sword
	std::uniform_real_distribution<GLfloat> sword_x_random(-0.1f * win_width, 0.1f * win_width);
	std::uniform_real_distribution<GLfloat> sword_direction_random(-15 * TO_RADIAN, 15 * TO_RADIAN);
	std::uniform_real_distribution<GLfloat> sword_speed_random(-0.2f, 0.5f);

	// Randomly control sword direction
	sword_direction += sword_direction_random(gen);
	if (sword_direction < SWORD_DIRECTION_MIN)
	{
		sword_direction = SWORD_DIRECTION_MIN;
	}
	else if (sword_direction > SWORD_DIRECTION_MAX)
	{
		sword_direction = SWORD_DIRECTION_MAX;
	}

	// Randomly control sword speed
	sword_speed += sword_speed_random(gen);
	if (sword_speed < SWORD_SPEED_MIN)
	{
		sword_speed = SWORD_SPEED_MIN;
	}
	else if (sword_speed > SWORD_SPEED_MAX)
	{
		sword_speed = SWORD_SPEED_MAX;
	}

	sword_position = move(sword_position, sword_direction, sword_speed);
	sword_position.x -= car_speed;
	if (!(-0.6f * win_width < sword_position.x && sword_position.x < win_width * 0.6f) || !(-win_height * 0.6f < sword_position.y && sword_position.y < win_height * 0.6f))
	{
		GLfloat new_x = sword_x_random(gen);
		sword_position = glm::vec2(new_x, -0.5f * win_height);
	}

	// Move airplane
	// airplane_position.x -= car_speed;
	if (airplane_speed < 50.0f)
	{
		airplane_speed *= 1.002f;
	}
	airplane_position = move(airplane_position, airplane_direction - 90 * TO_RADIAN, airplane_speed);
	std::uniform_real_distribution<GLfloat> airplane_scale_random(0.998f, 1.005f);
	airplane_scale *= airplane_scale_random(gen);
	if (airplane_scale.x < AIRPLANE_SCALE_MIN)
	{
		airplane_scale = glm::vec2(AIRPLANE_SCALE_MIN, AIRPLANE_SCALE_MIN);
	}
	if (airplane_scale.x > AIRPLANE_SCALE_MAX)
	{
		airplane_scale = glm::vec2(AIRPLANE_SCALE_MAX, AIRPLANE_SCALE_MAX);
	}
	airplane_direction += 5.0f * TO_RADIAN;

	glutPostRedisplay();
	glutTimerFunc(REFRESH_RATE, timer, 0);
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:				 // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	case 'p':
	case 'P':
		if (!gameover_flag)
		{
			pause = !pause;
		}
		break;
	}
}

void special(int key, int x, int y)
{
	if (gameover_flag)
	{
		return;
	}
	switch (key)
	{
	case GLUT_KEY_LEFT:
		--car_speed;
		if (car_speed < MIN_SPEED)
		{
			car_speed = MIN_SPEED;
		}
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		++car_speed;
		if (car_speed > MAX_SPEED)
		{
			car_speed = MAX_SPEED;
		}
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		car2_position.y -= 0.01f * win_height;
		glutPostRedisplay();
		break;
	case GLUT_KEY_UP:
		car2_position.y += 0.01f * win_height;
		glutPostRedisplay();
		break;
	}
}

void cleanup(void)
{
	glDeleteVertexArrays(1, &VAO_road);
	glDeleteBuffers(1, &VBO_road);

	glDeleteVertexArrays(1, &VAO_house);
	glDeleteBuffers(1, &VBO_house);
}

void register_callbacks(void)
{
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutTimerFunc(0, timer, 0);
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
	glEnable(GL_MULTISAMPLE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClearColor(BACKGROUND_COLOR[0], BACKGROUND_COLOR[1], BACKGROUND_COLOR[2], 1.0f);
	ViewMatrix = glm::mat4(1.0f);

	glViewport(0, 0, win_width, win_height);
	ProjectionMatrix = glm::ortho(-win_width / 2.0, win_width / 2.0,
								  -win_height / 2.0, win_height / 2.0, -1000.0, 1000.0);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void prepare_scene(void)
{
	prepare_road();
	prepare_house();
	prepare_car2();
	prepare_sword();
	prepare_cake();
	prepare_car();
	prepare_boom();
	prepare_airplane();
}

void initialize_renderer(void)
{
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
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

void greetings(char *program_name, char messages[][256], int n_message_lines)
{
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  %s\n\n", program_name);
	fprintf(stdout, " CSE4170 Assignment 1\n");
	fprintf(stdout, "    20120085 EOM, Taegyung\n");
	fprintf(stdout, "      April 17, 2019\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 4
int main(int argc, char *argv[])
{
	char program_name[64] = "Fury Road";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used: 'ESC', 'p', four arrows",
		"    - Press 'p' to pause.",
		"    - Press Up/Down arrow to move car",
		"    - Press Left/Right arrow to control speed.",
	};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_MULTISAMPLE);
	win_width = INITIAL_WIDTH;
	win_height = INITIAL_HEIGHT;
	glutInitWindowSize(win_width, win_height);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
	return 0;
}
