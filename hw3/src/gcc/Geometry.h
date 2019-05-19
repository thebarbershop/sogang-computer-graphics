GLenum polygonFace = GL_FRONT_AND_BACK;

namespace color
{
const GLfloat black[3] = {0.0f, 0.0f, 0.0f};
const GLfloat white[3] = {1.0f, 1.0f, 1.0f};
const GLfloat green[3] = {0.0f, 1.0f, 0.0f};
const GLfloat forest_green[3] = {0x30 / 256.0f, 0x81 / 256.0f, 0x28 / 256.0f};
const GLfloat dark_orange[3] = {0xFF / 256.0f, 0x8C / 256.0f, 0x00 / 256.0f};
const GLfloat aquamarine[3] = {0.498f, 1.000f, 0.831f};
const GLfloat dark_turquoise[3] = {0.000f, 0.808f, 0.820f};
const GLfloat light_steel_blue[3] = {0.690f, 0.769f, 0.871f};
const GLfloat fire_brick[3] = {0xB2 / 256.0f, 0x22 / 256.0f, 0x22 / 256.0f};
const GLfloat lavender[3] = {0xE6 / 256.0f, 0xE6 / 256.0f, 0xFA / 256.0f};
const GLfloat tan[3] = {0xD2 / 256.0f, 0xB4 / 256.0f, 0x8C / 256.0f};
const GLfloat rosy_brown[3] = {0xBC / 256.0f, 0x8F / 256.0f, 0x8F / 256.0f};
} // namespace color

// wheel numbers
const int WHEEL_FRONT_LEFT = 0b00;
const int WHEEL_REAR_LEFT = 0b01;
const int WHEEL_FRONT_RIGHT = 0b10;
const int WHEEL_REAR_RIGHT = 0b11;

/*********************************  START: geometry *********************************/
#define BUFFER_OFFSET(offset) ((GLvoid *)(offset))

#define LOC_VERTEX 0
#define LOC_NORMAL 1
#define LOC_TEXCOORD 2

GLuint axes_VBO, axes_VAO;
GLfloat axes_vertices[6][3] = {
	{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
GLfloat axes_color[3][3] = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};

void prepare_axes(void)
{ // Draw coordinate axes.
	// Initialize vertex buffer object.
	glGenBuffers(1, &axes_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes_vertices), &axes_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &axes_VAO);
	glBindVertexArray(axes_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_axes(void)
{
	glBindVertexArray(axes_VAO);
	glUniform3fv(loc_primitive_color, 1, axes_color[0]);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[1]);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[2]);
	glDrawArrays(GL_LINES, 4, 2);
	glBindVertexArray(0);
}

void free_axes(void)
{
	glDeleteVertexArrays(1, &axes_VAO);
	glDeleteBuffers(1, &axes_VBO);
}

GLuint path_VBO, path_VAO;
GLfloat *path_vertices;
int path_n_vertices;

int read_path_file(GLfloat **object, const char *filename)
{
	int i, n_vertices;
	float *flt_ptr;
	FILE *fp;

	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot open the path file %s ...", filename);
		return -1;
	}

	if (!fscanf(fp, "%d", &n_vertices))
	{
		fprintf(stderr, "Cannot read value of n_vertices");
		return -1;
	}
	*object = (float *)malloc(n_vertices * 3 * sizeof(float));
	if (*object == NULL)
	{
		fprintf(stderr, "Cannot allocate memory for the path file %s ...", filename);
		return -1;
	}

	flt_ptr = *object;
	for (i = 0; i < n_vertices; ++i)
	{
		if (fscanf(fp, "%f %f %f", flt_ptr, flt_ptr + 1, flt_ptr + 2) != 3)
		{
			fprintf(stderr, "Cannot read coordinate of vertex %d", i);
			return -1;
		}
		flt_ptr += 3;
	}
	fclose(fp);

	return n_vertices;
}

#define N_GEOMETRY_OBJECTS 6
#define GEOM_OBJ_ID_CAR_BODY 0
#define GEOM_OBJ_ID_CAR_WHEEL 1
#define GEOM_OBJ_ID_CAR_NUT 2
#define GEOM_OBJ_ID_COW 3
#define GEOM_OBJ_ID_TEAPOT 4
#define GEOM_OBJ_ID_BOX 5

GLuint geom_obj_VBO[N_GEOMETRY_OBJECTS];
GLuint geom_obj_VAO[N_GEOMETRY_OBJECTS];

int geom_obj_n_triangles[N_GEOMETRY_OBJECTS];
GLfloat *geom_obj_vertices[N_GEOMETRY_OBJECTS];

// codes for the 'general' triangular-mesh object
typedef enum _GEOM_OBJ_TYPE
{
	GEOM_OBJ_TYPE_V = 0,
	GEOM_OBJ_TYPE_VN,
	GEOM_OBJ_TYPE_VNT
} GEOM_OBJ_TYPE;
// GEOM_OBJ_TYPE_V: (x, y, z)
// GEOM_OBJ_TYPE_VN: (x, y, z, nx, ny, nz)
// GEOM_OBJ_TYPE_VNT: (x, y, z, nx, ny, nz, s, t)
int GEOM_OBJ_ELEMENTS_PER_VERTEX[3] = {3, 6, 8};

int read_geometry_file(GLfloat **object, const char *filename, GEOM_OBJ_TYPE geom_obj_type)
{
	int i, n_triangles;
	float *flt_ptr;
	FILE *fp;

	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot open the geometry file %s ...", filename);
		return -1;
	}

	if (!fscanf(fp, "%d", &n_triangles))
	{
		fprintf(stderr, "Cannot read value of n_triangles from %s\n", filename);
		return -1;
	}
	*object = (float *)malloc(3 * n_triangles * GEOM_OBJ_ELEMENTS_PER_VERTEX[geom_obj_type] * sizeof(float));
	if (*object == NULL)
	{
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	flt_ptr = *object;
	for (i = 0; i < 3 * n_triangles * GEOM_OBJ_ELEMENTS_PER_VERTEX[geom_obj_type]; ++i)
	{
		if (!fscanf(fp, "%f", flt_ptr++))
		{
			fprintf(stderr, "Cannot read value %d from %s\n", i, filename);
			return -1;
		}
	}
	fclose(fp);

	return n_triangles;
}

void prepare_geom_obj(int geom_obj_ID, const char *filename, GEOM_OBJ_TYPE geom_obj_type)
{
	int n_bytes_per_vertex;

	n_bytes_per_vertex = GEOM_OBJ_ELEMENTS_PER_VERTEX[geom_obj_type] * sizeof(float);
	geom_obj_n_triangles[geom_obj_ID] = read_geometry_file(&geom_obj_vertices[geom_obj_ID], filename, geom_obj_type);

	// Initialize vertex array object.
	glGenVertexArrays(1, &geom_obj_VAO[geom_obj_ID]);
	glBindVertexArray(geom_obj_VAO[geom_obj_ID]);
	glGenBuffers(1, &geom_obj_VBO[geom_obj_ID]);
	glBindBuffer(GL_ARRAY_BUFFER, geom_obj_VBO[geom_obj_ID]);
	glBufferData(GL_ARRAY_BUFFER, 3 * geom_obj_n_triangles[geom_obj_ID] * n_bytes_per_vertex,
				 geom_obj_vertices[geom_obj_ID], GL_STATIC_DRAW);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	if (geom_obj_type >= GEOM_OBJ_TYPE_VN)
	{
		glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
	if (geom_obj_type >= GEOM_OBJ_TYPE_VNT)
	{
		glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	free(geom_obj_vertices[geom_obj_ID]);
}

void draw_geom_obj(int geom_obj_ID)
{
	glBindVertexArray(geom_obj_VAO[geom_obj_ID]);
	glDrawArrays(GL_TRIANGLES, 0, 3 * geom_obj_n_triangles[geom_obj_ID]);
	glBindVertexArray(0);
}

void free_geom_obj(int geom_obj_ID)
{
	glDeleteVertexArrays(1, &geom_obj_VAO[geom_obj_ID]);
	glDeleteBuffers(1, &geom_obj_VBO[geom_obj_ID]);
}

/* START Custom Code */
const GLfloat floor_size = 50.0f;
GLuint floor_VBO, floor_VAO;
const GLfloat floor_vertices[4][3] = {
	{floor_size, floor_size, 0.0f}, {-floor_size, floor_size, 0.0f}, {-floor_size, -floor_size, 0.0f}, {floor_size, -floor_size, 0.0f}};
const GLfloat *floor_color = color::forest_green;
const GLfloat *grid_color = color::black;
const int n_grid = 8;

void prepare_floor(void)
{ // Draw grid floor.

	// Calculate coordinates for grids
	GLfloat grid_vertices[2 * (n_grid + 1)][2][3];
	const GLfloat grid_width = floor_size / (GLfloat)n_grid;
	for (int i = 0; i < (n_grid + 1); ++i)
	{
		grid_vertices[i][0][0] = -floor_size + grid_width * (GLfloat)(i * 2);
		grid_vertices[i][0][1] = floor_size;
		grid_vertices[i][0][2] = 0.0f;
		grid_vertices[i][1][0] = -floor_size + grid_width * (GLfloat)(i * 2);
		grid_vertices[i][1][1] = -floor_size;
		grid_vertices[i][1][2] = 0.0f;
	}
	for (int i = n_grid + 1; i < 2 * (n_grid + 1); ++i)
	{
		grid_vertices[i][0][0] = floor_size;
		grid_vertices[i][0][1] = -floor_size + grid_width * (GLfloat)((i - n_grid - 1) * 2);
		grid_vertices[i][0][2] = 0.0f;
		grid_vertices[i][1][0] = -floor_size;
		grid_vertices[i][1][1] = -floor_size + grid_width * (GLfloat)((i - n_grid - 1) * 2);
		grid_vertices[i][1][2] = 0.0f;
	}

	GLsizeiptr buffer_size = sizeof(floor_vertices) + sizeof(grid_vertices);
	// Initialize vertex buffer object.
	glGenBuffers(1, &floor_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, floor_VBO);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_vertices), floor_vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_vertices), sizeof(grid_vertices), grid_vertices);

	// Initialize vertex array object.
	glGenVertexArrays(1, &floor_VAO);
	glBindVertexArray(floor_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, floor_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_floor(void)
{
	unsigned int current_size = 0;

	glBindVertexArray(floor_VAO);
	glUniform3fv(loc_primitive_color, 1, floor_color);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	current_size += 4;

	glUniform3fv(loc_primitive_color, 1, grid_color);
	glDrawArrays(GL_LINES, current_size, 2 * (n_grid + 1));
	current_size += 2 * (n_grid + 1);
	glDrawArrays(GL_LINES, current_size, 2 * (n_grid + 1));
	current_size += 2 * (n_grid + 1);

	glBindVertexArray(0);
}

void free_floor(void)
{
	glDeleteVertexArrays(1, &floor_VAO);
	glDeleteBuffers(1, &floor_VBO);
}

int read_geometry(GLfloat **object, int bytes_per_primitive, char *filename)
{
	int n_triangles;
	FILE *fp;

	fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot open the object file %s ...", filename);
		return -1;
	}
	if (!fread(&n_triangles, sizeof(int), 1, fp))
	{
		fprintf(stderr, "Cannot read value of n_triangles from %s\n", filename);
		return -1;
	}

	*object = (float *)malloc(n_triangles * bytes_per_primitive);
	if (*object == NULL)
	{
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	if (fread(*object, bytes_per_primitive, n_triangles, fp) < (unsigned)n_triangles)
	{
		fprintf(stderr, "Cannot read objects from %s\n", filename);
		return -1;
	}
	fclose(fp);

	return n_triangles;
}

// tiger object
int cur_frame_tiger = 0;
#define N_TIGER_FRAMES 12
GLuint tiger_VBO, tiger_VAO;
int tiger_n_triangles[N_TIGER_FRAMES];
int tiger_vertex_offset[N_TIGER_FRAMES];
GLfloat *tiger_vertices[N_TIGER_FRAMES];
const GLfloat *tiger_color = color::dark_orange;

void prepare_tiger(void)
{ // vertices enumerated clockwise
	int i, n_bytes_per_vertex, n_bytes_per_triangle, tiger_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_TIGER_FRAMES; ++i)
	{
		sprintf(filename, "Data/dynamic_objects/tiger/Tiger_%d%d_triangles_vnt.geom", i / 10, i % 10);
		tiger_n_triangles[i] = read_geometry(&tiger_vertices[i], n_bytes_per_triangle, filename);

		// assume all geometry files are effective
		tiger_n_total_triangles += tiger_n_triangles[i];

		if (i == 0)
			tiger_vertex_offset[i] = 0;
		else
			tiger_vertex_offset[i] = tiger_vertex_offset[i - 1] + 3 * tiger_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &tiger_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glBufferData(GL_ARRAY_BUFFER, tiger_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_TIGER_FRAMES; ++i)
		glBufferSubData(GL_ARRAY_BUFFER, tiger_vertex_offset[i] * n_bytes_per_vertex,
						tiger_n_triangles[i] * n_bytes_per_triangle, tiger_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_TIGER_FRAMES; ++i)
		free(tiger_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &tiger_VAO);
	glBindVertexArray(tiger_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void draw_tiger(void)
{
	glBindVertexArray(tiger_VAO);
	glUniform3fv(loc_primitive_color, 1, tiger_color);
	glDrawArrays(GL_TRIANGLES, tiger_vertex_offset[cur_frame_tiger], 3 * tiger_n_triangles[cur_frame_tiger]);
	glBindVertexArray(0);
}

void free_tiger(void)
{
	glDeleteVertexArrays(1, &tiger_VAO);
	glDeleteBuffers(1, &tiger_VBO);
}

// ironman object
GLuint ironman_VBO, ironman_VAO;
int ironman_n_triangles;
GLfloat *ironman_vertices;
const GLfloat *ironman_color = color::tan;

void prepare_ironman(void)
{
	int n_bytes_per_vertex, n_bytes_per_triangle, ironman_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/ironman_vnt.geom");
	ironman_n_triangles = read_geometry(&ironman_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	ironman_n_total_triangles += ironman_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &ironman_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, ironman_VBO);
	glBufferData(GL_ARRAY_BUFFER, ironman_n_total_triangles * 3 * n_bytes_per_vertex, ironman_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(ironman_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &ironman_VAO);
	glBindVertexArray(ironman_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, ironman_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_ironman(void)
{
	glFrontFace(GL_CW);

	glBindVertexArray(ironman_VAO);
	glUniform3fv(loc_primitive_color, 1, ironman_color);
	glDrawArrays(GL_TRIANGLES, 0, 3 * ironman_n_triangles);
	glBindVertexArray(0);
}

void free_ironman(void)
{
	glDeleteVertexArrays(1, &ironman_VAO);
	glDeleteBuffers(1, &ironman_VBO);
}

// bus object
GLuint bus_VBO, bus_VAO;
int bus_n_triangles;
GLfloat *bus_vertices;
const GLfloat *bus_color = color::fire_brick;

void prepare_bus(void)
{
	int n_bytes_per_vertex, n_bytes_per_triangle, bus_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/bus_vnt.geom");
	bus_n_triangles = read_geometry(&bus_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	bus_n_total_triangles += bus_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &bus_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, bus_VBO);
	glBufferData(GL_ARRAY_BUFFER, bus_n_total_triangles * 3 * n_bytes_per_vertex, bus_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(bus_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &bus_VAO);
	glBindVertexArray(bus_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, bus_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_bus(void)
{
	glFrontFace(GL_CW);

	glBindVertexArray(bus_VAO);
	glUniform3fv(loc_primitive_color, 1, bus_color);
	glDrawArrays(GL_TRIANGLES, 0, 3 * bus_n_triangles);
	glBindVertexArray(0);
}

void free_bus(void)
{
	glDeleteVertexArrays(1, &bus_VAO);
	glDeleteBuffers(1, &bus_VBO);
}

// spider object
#define N_SPIDER_FRAMES 16
GLuint spider_VBO, spider_VAO;
int spider_n_triangles[N_SPIDER_FRAMES];
int spider_vertex_offset[N_SPIDER_FRAMES];
GLfloat *spider_vertices[N_SPIDER_FRAMES];
int cur_frame_spider;
const GLfloat *spider_color = color::rosy_brown;

void prepare_spider(void)
{
	int i, n_bytes_per_vertex, n_bytes_per_triangle, spider_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_SPIDER_FRAMES; i++)
	{
		sprintf(filename, "Data/dynamic_objects/spider/spider_vnt_%d%d.geom", i / 10, i % 10);
		spider_n_triangles[i] = read_geometry(&spider_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		spider_n_total_triangles += spider_n_triangles[i];

		if (i == 0)
			spider_vertex_offset[i] = 0;
		else
			spider_vertex_offset[i] = spider_vertex_offset[i - 1] + 3 * spider_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &spider_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, spider_VBO);
	glBufferData(GL_ARRAY_BUFFER, spider_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_SPIDER_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, spider_vertex_offset[i] * n_bytes_per_vertex,
						spider_n_triangles[i] * n_bytes_per_triangle, spider_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_SPIDER_FRAMES; i++)
		free(spider_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &spider_VAO);
	glBindVertexArray(spider_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, spider_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_spider(void)
{
	glFrontFace(GL_CW);

	glBindVertexArray(spider_VAO);
	glUniform3fv(loc_primitive_color, 1, spider_color);
	glDrawArrays(GL_TRIANGLES, spider_vertex_offset[cur_frame_spider], 3 * spider_n_triangles[cur_frame_spider]);
	glBindVertexArray(0);
}

void free_spider(void)
{
	glDeleteVertexArrays(1, &spider_VAO);
	glDeleteBuffers(1, &spider_VBO);
}

/* END Custom Code */
/*********************************  END: geometry *********************************/