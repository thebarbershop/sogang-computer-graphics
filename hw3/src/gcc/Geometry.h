GLenum polygonFace = GL_FRONT_AND_BACK;
int polygonMode = 0;

namespace color
{
const GLfloat black[3] = {0.0f, 0.0f, 0.0f};
const GLfloat white[3] = {1.0f, 1.0f, 1.0f};
const GLfloat green[3] = {0.0f, 1.0f, 0.0f};
const GLfloat forest_green[3] = {0x30 / 256.0f, 0x81 / 256.0f, 0x28 / 256.0f};
} // namespace color

/*********************************  START: geometry *********************************/
#define BUFFER_OFFSET(offset) ((GLvoid *)(offset))

#define LOC_VERTEX 0
#define LOC_NORMAL 1
#define LOC_TEXCOORD 2

GLuint axes_VBO, axes_VAO;
GLfloat axes_vertices[6][3] = {
	{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
GLfloat axes_color[3][3] = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};

void setPolygonMode()
{
	GLenum GLmode = polygonMode ? GL_FILL : GL_LINE;
	glPolygonMode(polygonFace, GLmode);
}

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
	for (i = 0; i < n_vertices; i++)
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

void prepare_path(void)
{ // Draw path.
	//	return;
	path_n_vertices = read_path_file(&path_vertices, "Data/path.txt");
	// Initialize vertex array object.
	glGenVertexArrays(1, &path_VAO);
	glBindVertexArray(path_VAO);
	glGenBuffers(1, &path_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, path_VBO);
	glBufferData(GL_ARRAY_BUFFER, path_n_vertices * 3 * sizeof(float), path_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_path(void)
{
	glBindVertexArray(path_VAO);
	glUniform3f(loc_primitive_color, 1.000f, 1.000f, 0.000f); // color name: Yellow
	glDrawArrays(GL_LINE_STRIP, 0, path_n_vertices);
}

void free_path(void)
{
	glDeleteVertexArrays(1, &path_VAO);
	glDeleteBuffers(1, &path_VBO);
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
		fprintf(stderr, "Cannot read value of n_triangles");
		return -1;
	}
	*object = (float *)malloc(3 * n_triangles * GEOM_OBJ_ELEMENTS_PER_VERTEX[geom_obj_type] * sizeof(float));
	if (*object == NULL)
	{
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	flt_ptr = *object;
	for (i = 0; i < 3 * n_triangles * GEOM_OBJ_ELEMENTS_PER_VERTEX[geom_obj_type]; i++)
	{
		if (!fscanf(fp, "%f", flt_ptr++))
		{
			fprintf(stderr, "Cannot read value %d", i);
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
const GLfloat floor_size = 30.0f;
GLuint floor_VBO, floor_VAO;
const GLfloat floor_vertices[4][3] = {
	{floor_size, 0.0f, floor_size}, {-floor_size, 0.0f, floor_size}, {-floor_size, 0.0f, -floor_size}, {floor_size, 0.0f, -floor_size}};
const GLfloat *floor_color = color::forest_green;
const GLfloat *grid_color = color::black;
const int n_grid = 8;

void prepare_floor(void)
{ // Draw grid floor.

	// Calculate coordinates for grids
	GLfloat grid_vertices[2 * (n_grid + 1)][2][3];
	const GLfloat grid_width = floor_size / (GLfloat)n_grid;
	for (int i = 0; i < (n_grid + 1); i++)
	{
		grid_vertices[i][0][0] = -floor_size + grid_width * (GLfloat)(i * 2);
		grid_vertices[i][0][1] = 0.0f;
		grid_vertices[i][0][2] = floor_size;
		grid_vertices[i][1][0] = -floor_size + grid_width * (GLfloat)(i * 2);
		grid_vertices[i][1][1] = 0.0f;
		grid_vertices[i][1][2] = -floor_size;
	}
	for (int i = n_grid + 1; i < 2 * (n_grid + 1); i++)
	{
		grid_vertices[i][0][0] = floor_size;
		grid_vertices[i][0][1] = 0.0f;
		grid_vertices[i][0][2] = -floor_size + grid_width * (GLfloat)((i - n_grid - 1) * 2);
		grid_vertices[i][1][0] = -floor_size;
		grid_vertices[i][1][1] = 0.0f;
		grid_vertices[i][1][2] = -floor_size + grid_width * (GLfloat)((i - n_grid - 1) * 2);
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
	int orig_polygonMode = polygonMode;
	polygonMode = 1;
	setPolygonMode();

	unsigned int current_size = 0;

	glBindVertexArray(floor_VAO);
	glUniform3fv(loc_primitive_color, 1, floor_color);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	current_size += 4;

	glLineWidth(2.0f);
	glUniform3fv(loc_primitive_color, 1, grid_color);
	glDrawArrays(GL_LINES, current_size, 2 * (n_grid + 1));
	current_size += 2 * (n_grid + 1);
	glDrawArrays(GL_LINES, current_size, 2 * (n_grid + 1));
	current_size += 2 * (n_grid + 1);
	glLineWidth(1.0f);

	glBindVertexArray(0);
	polygonMode = orig_polygonMode;
	setPolygonMode();
}

void free_floor(void)
{
	glDeleteVertexArrays(1, &floor_VAO);
	glDeleteBuffers(1, &floor_VBO);
}
/* END Custom Code */
/*********************************  END: geometry *********************************/