/*********************************  START: camera *********************************/
#include <algorithm>

typedef struct _Camera
{
	glm::vec3 pos;
	glm::vec3 uaxis, vaxis, naxis;

	float fovy, aspect_ratio, near_c, far_c;
	int move;

	float FOVY_COEFF, MANIPULATE_COEFF, ROTATION_COEFF;
} Camera;

Camera camera_wv;
Camera camera_sub;

enum _CameraType
{
	CAMERA_WORLD_VIEWER,
	CAMERA_DRIVER
} camera_type;

void set_axes_from_ViewMatrix(Camera &camera)
{
	camera.uaxis = glm::vec3(ViewMatrix[0].x, ViewMatrix[1].x, ViewMatrix[2].x);
	camera.vaxis = glm::vec3(ViewMatrix[0].y, ViewMatrix[1].y, ViewMatrix[2].y);
	camera.naxis = glm::vec3(ViewMatrix[0].z, ViewMatrix[1].z, ViewMatrix[2].z);
	camera.pos = -(ViewMatrix[3].x * camera.uaxis + ViewMatrix[3].y * camera.vaxis + ViewMatrix[3].z * camera.naxis);
}

void set_ViewMatrix(const Camera &camera)
{
	ViewMatrix = glm::mat4(camera.uaxis.x, camera.vaxis.x, camera.naxis.x, 0.0f,
						   camera.uaxis.y, camera.vaxis.y, camera.naxis.y, 0.0f,
						   camera.uaxis.z, camera.vaxis.z, camera.naxis.z, 0.0f,
						   0.0f, 0.0f, 0.0f, 1.0f);
	ViewMatrix = glm::translate(ViewMatrix, -camera.pos);
}

void set_ViewProjectionMatrix(const Camera &camera)
{
	set_ViewMatrix(camera);
	ProjectionMatrix = glm::perspective(glm::radians(camera.fovy), camera.aspect_ratio, camera.near_c, camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}
void set_ViewProjectionMatrix_for_driver(void)
{
	glm::mat4 Matrix_CAMERA_driver_inverse;

	Matrix_CAMERA_driver_inverse = ModelMatrix_CAR_BODY * ModelMatrix_CAR_BODY_to_DRIVER;

	ViewMatrix = glm::affineInverse(Matrix_CAMERA_driver_inverse);
	ProjectionMatrix = glm::perspective(glm::radians(30.0f), camera_wv.aspect_ratio, camera_wv.near_c, camera_wv.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void initialize_world_camera(void)
{
	camera_type = CAMERA_WORLD_VIEWER;
	ViewMatrix = glm::lookAt(glm::vec3(75.0f, 75.0f, 100.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	set_axes_from_ViewMatrix(camera_wv);

	camera_wv.move = 0;
	camera_wv.fovy = 30.0f;
	camera_wv.aspect_ratio = (float)glutGet(GLUT_WINDOW_WIDTH) / (float)glutGet(GLUT_WINDOW_HEIGHT);
	camera_wv.near_c = 5.0f;
	camera_wv.far_c = 10000.0f;

	camera_wv.FOVY_COEFF = 0.05f;
	camera_wv.MANIPULATE_COEFF = 0.1f;
}

void initialize_sub_camera(void)
{
	ViewMatrix = glm::lookAt(glm::vec3(0.0f, 200.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	set_axes_from_ViewMatrix(camera_sub);

	camera_sub.move = 0;
	camera_sub.fovy = 30.0f;
	camera_sub.aspect_ratio = (float)glutGet(GLUT_WINDOW_WIDTH) / (float)glutGet(GLUT_WINDOW_HEIGHT);
	camera_sub.near_c = 5.0f;
	camera_sub.far_c = 10000.0f;

	camera_sub.FOVY_COEFF = 0.05f;
	camera_sub.MANIPULATE_COEFF = 2.0f;
	camera_sub.ROTATION_COEFF = 0.01f;
}

void initialize_camera(void)
{
	initialize_world_camera();
	initialize_sub_camera();
	set_ViewProjectionMatrix(camera_wv);

	// the transformation that moves the driver's camera frame from car body's MC to driver seat
	ModelMatrix_CAR_BODY_to_DRIVER = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.5f, 2.5f));
	ModelMatrix_CAR_BODY_to_DRIVER = glm::rotate(ModelMatrix_CAR_BODY_to_DRIVER,
												 glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

void renew_camera_fovy(Camera &camera, const float del)
{
	camera.fovy = std::max(camera.fovy + del * camera.FOVY_COEFF, EPSILON);
	camera.fovy = std::min(camera.fovy, 180.0f);
}

void manipulate_world_camera(int key)
{
	// Manipulate world camera position
	glm::vec4 tpos = glm::vec4(camera_wv.pos.x, camera_wv.pos.y, camera_wv.pos.z, 1.0f);
	GLfloat fy = camera_wv.pos.y;
	int flag = 1;

	switch (key)
	{
	case GLUT_KEY_UP:
		tpos = glm::rotate(glm::mat4(1.0f), camera_wv.MANIPULATE_COEFF, glm::vec3(-tpos.z, 0.0f, tpos.x)) * tpos;
		flag = tpos.y > fy; // Camera must move higher
		fprintf(stderr, "World camera climbing up.\n");
		break;
	case GLUT_KEY_DOWN:
		tpos = glm::rotate(glm::mat4(1.0f), camera_wv.MANIPULATE_COEFF, glm::vec3(tpos.z, 0.0f, -tpos.x)) * tpos;
		flag = (tpos.y < fy) && (tpos.y > 0.0f); // Camera must move lower but should not go underground
		fprintf(stderr, "World camera climbing down.\n");
		break;
	case GLUT_KEY_LEFT:
		tpos = glm::rotate(glm::mat4(1.0f), camera_wv.MANIPULATE_COEFF, glm::vec3(0.0f, -1.0f, 0.0f)) * tpos;
		fprintf(stderr, "World camera rotating left.\n");
		break;
	case GLUT_KEY_RIGHT:
		tpos = glm::rotate(glm::mat4(1.0f), camera_wv.MANIPULATE_COEFF, glm::vec3(0.0f, 1.0f, 0.0f)) * tpos;
		fprintf(stderr, "World camera rotating right.\n");
		break;
	default:
		return;
	}
	if (flag)
	{
		camera_wv.pos.y = tpos.y;
		camera_wv.pos.x = tpos.x;
		camera_wv.pos.z = tpos.z;
	}

	ViewMatrix = glm::lookAt(camera_wv.pos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	set_axes_from_ViewMatrix(camera_wv);
}

void manipulate_sub_camera(int key)
{
	// Manipulate sub camera position
	glm::vec4 tpos = glm::vec4(camera_sub.pos.x, camera_sub.pos.y, camera_sub.pos.z, 1.0f);
	int flag = 1;

	switch (key)
	{
	case GLUT_KEY_UP:
		tpos = glm::translate(glm::mat4(1.0f), camera_sub.vaxis * camera_sub.MANIPULATE_COEFF) * tpos;
		fprintf(stderr, "Sub camera moving forward.\n");
		break;
	case GLUT_KEY_DOWN:
		tpos = glm::translate(glm::mat4(1.0f), -camera_sub.vaxis * camera_sub.MANIPULATE_COEFF) * tpos;
		flag = (tpos.y > EPSILON);
		fprintf(stderr, "Sub camera moving backward.\n");
		break;
	case GLUT_KEY_LEFT:
		tpos = glm::translate(glm::mat4(1.0f), -camera_sub.uaxis * camera_sub.MANIPULATE_COEFF) * tpos;
		fprintf(stderr, "Sub camera moving left.\n");
		break;
	case GLUT_KEY_RIGHT:
		tpos = glm::translate(glm::mat4(1.0f), camera_sub.uaxis * camera_sub.MANIPULATE_COEFF) * tpos;
		fprintf(stderr, "Sub camera moving right.\n");
		break;
	case 'z':
		tpos = glm::translate(glm::mat4(1.0f), camera_sub.naxis * camera_sub.MANIPULATE_COEFF) * tpos;
		fprintf(stderr, "Sub camera moving up.\n");
		break;
	case 'x':
		tpos = glm::translate(glm::mat4(1.0f), -camera_sub.naxis * camera_sub.MANIPULATE_COEFF) * tpos;
		fprintf(stderr, "Sub camera moving down.\n");
		break;
	case 'c':
		set_ViewMatrix(camera_sub);
		ViewMatrix = glm::rotate(ViewMatrix, camera_sub.ROTATION_COEFF, camera_sub.vaxis);
		set_axes_from_ViewMatrix(camera_sub);
		fprintf(stderr, "Sub camera rotating counter-clockwise.\n");
		break;
	case 'v':
		set_ViewMatrix(camera_sub);
		ViewMatrix = glm::rotate(ViewMatrix, -camera_sub.ROTATION_COEFF, camera_sub.vaxis);
		set_axes_from_ViewMatrix(camera_sub);
		fprintf(stderr, "Sub camera rotating clockwise.\n");
		break;
	case 'a':
		renew_camera_fovy(camera_sub, -1.0f);
		fprintf(stderr, "Sub camera zoom in.\n");
		break;
	case 's':
		renew_camera_fovy(camera_sub, 1.0f);
		fprintf(stderr, "Sub camera zoom out.\n");
		break;
	case 'r':
		initialize_sub_camera();
		fprintf(stderr, "Sub camera position reset\n");
		return;
	default:
		return;
	}
	if (flag)
	{
		camera_sub.pos.x = tpos.x;
		camera_sub.pos.y = tpos.y;
		camera_sub.pos.z = tpos.z;
	}
}

/*********************************  END: camera *********************************/