/*********************************  START: camera *********************************/
#include <algorithm>

typedef struct _Camera
{
	glm::vec3 pos;
	glm::vec3 uaxis, vaxis, naxis;

	float fovy, aspect_ratio, near_c, far_c;
	int move;
} Camera;

Camera camera_wv;
Camera camera_sub;

enum _CameraType
{
	CAMERA_WORLD_VIEWER,
	CAMERA_DRIVER
} camera_type;

const float CAM_FOVY_COEFF = 0.05f;

void set_ViewProjectionMatrix_for_world_viewer(void)
{
	ViewMatrix = glm::mat4(camera_wv.uaxis.x, camera_wv.vaxis.x, camera_wv.naxis.x, 0.0f,
						   camera_wv.uaxis.y, camera_wv.vaxis.y, camera_wv.naxis.y, 0.0f,
						   camera_wv.uaxis.z, camera_wv.vaxis.z, camera_wv.naxis.z, 0.0f,
						   0.0f, 0.0f, 0.0f, 1.0f);
	ViewMatrix = glm::translate(ViewMatrix, -camera_wv.pos);
	ProjectionMatrix = glm::perspective(TO_RADIAN * camera_wv.fovy, camera_wv.aspect_ratio, camera_wv.near_c, camera_wv.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void set_ViewProjectionMatrix_for_driver(void)
{
	glm::mat4 Matrix_CAMERA_driver_inverse;

	Matrix_CAMERA_driver_inverse = ModelMatrix_CAR_BODY * ModelMatrix_CAR_BODY_to_DRIVER;

	ViewMatrix = glm::affineInverse(Matrix_CAMERA_driver_inverse);
	ProjectionMatrix = glm::perspective(TO_RADIAN * 30.0f, camera_wv.aspect_ratio, camera_wv.near_c, camera_wv.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void set_ViewProjectionMatrix_for_sub(void)
{
	ViewMatrix = glm::mat4(camera_sub.uaxis.x, camera_sub.vaxis.x, camera_sub.naxis.x, 0.0f,
						   camera_sub.uaxis.y, camera_sub.vaxis.y, camera_sub.naxis.y, 0.0f,
						   camera_sub.uaxis.z, camera_sub.vaxis.z, camera_sub.naxis.z, 0.0f,
						   0.0f, 0.0f, 0.0f, 1.0f);
	ViewMatrix = glm::translate(ViewMatrix, -camera_sub.pos);

	ProjectionMatrix = glm::perspective(TO_RADIAN * camera_sub.fovy, camera_sub.aspect_ratio, camera_sub.near_c, camera_sub.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void initialize_world_camera(void)
{
	camera_type = CAMERA_WORLD_VIEWER;
	ViewMatrix = glm::lookAt(glm::vec3(75.0f, 75.0f, 100.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	camera_wv.uaxis = glm::vec3(ViewMatrix[0].x, ViewMatrix[1].x, ViewMatrix[2].x);
	camera_wv.vaxis = glm::vec3(ViewMatrix[0].y, ViewMatrix[1].y, ViewMatrix[2].y);
	camera_wv.naxis = glm::vec3(ViewMatrix[0].z, ViewMatrix[1].z, ViewMatrix[2].z);
	camera_wv.pos = -(ViewMatrix[3].x * camera_wv.uaxis + ViewMatrix[3].y * camera_wv.vaxis + ViewMatrix[3].z * camera_wv.naxis);

	camera_wv.move = 0;
	camera_wv.fovy = 30.0f;
	camera_wv.aspect_ratio = (float)glutGet(GLUT_WINDOW_WIDTH) / (float)glutGet(GLUT_WINDOW_HEIGHT);
	camera_wv.near_c = 5.0f;
	camera_wv.far_c = 10000.0f;
}

void initialize_sub_camera(void)
{
	//	GLfloat sqrt2 = glm::sqrt(2);
	ViewMatrix = glm::lookAt(glm::vec3(0.0f, 100.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	camera_sub.uaxis = glm::vec3(ViewMatrix[0].x, ViewMatrix[1].x, ViewMatrix[2].x);
	camera_sub.vaxis = glm::vec3(ViewMatrix[0].y, ViewMatrix[1].y, ViewMatrix[2].y);
	camera_sub.naxis = glm::vec3(ViewMatrix[0].z, ViewMatrix[1].z, ViewMatrix[2].z);
	camera_sub.pos = -(ViewMatrix[3].x * camera_sub.uaxis + ViewMatrix[3].y * camera_sub.vaxis + ViewMatrix[3].z * camera_sub.naxis);
	camera_sub.move = 0;
	camera_sub.fovy = 30.0f;
	camera_sub.aspect_ratio = (float)glutGet(GLUT_WINDOW_WIDTH) / (float)glutGet(GLUT_WINDOW_HEIGHT);
	camera_sub.near_c = 5.0f;
	camera_sub.far_c = 10000.0f;
}

void initialize_camera(void)
{
	initialize_world_camera();
	initialize_sub_camera();
	set_ViewProjectionMatrix_for_world_viewer();

	// the transformation that moves the driver's camera frame from car body's MC to driver seat
	ModelMatrix_CAR_BODY_to_DRIVER = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.5f, 2.5f));
	ModelMatrix_CAR_BODY_to_DRIVER = glm::rotate(ModelMatrix_CAR_BODY_to_DRIVER,
												 TO_RADIAN * 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
}

void renew_cam_wv_fovy(const float del)
{
	camera_wv.fovy = std::max(camera_wv.fovy + del * CAM_FOVY_COEFF, EPSILON);
	camera_wv.fovy = std::min(camera_wv.fovy, 180.0f);
}

/*********************************  END: camera *********************************/