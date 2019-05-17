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
enum _CameraType
{
	CAMERA_WORLD_VIEWER,
	CAMERA_DRIVER
} camera_type;

const float CAM_TSPEED = 0.05f;
const float CAM_FOVY_COEFF = 0.05f;
const float CAM_RSPEED = 0.1f;

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

void initialize_world_camera(void)
{
	camera_type = CAMERA_WORLD_VIEWER;
	ViewMatrix = glm::lookAt(glm::vec3(75.0f, 75.0f, 100.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	camera_wv.uaxis = glm::vec3(ViewMatrix[0].x, ViewMatrix[1].x, ViewMatrix[2].x);
	camera_wv.vaxis = glm::vec3(ViewMatrix[0].y, ViewMatrix[1].y, ViewMatrix[2].y);
	camera_wv.naxis = glm::vec3(ViewMatrix[0].z, ViewMatrix[1].z, ViewMatrix[2].z);
	camera_wv.pos = -(ViewMatrix[3].x * camera_wv.uaxis + ViewMatrix[3].y * camera_wv.vaxis + ViewMatrix[3].z * camera_wv.naxis);

	camera_wv.move = 0;
	camera_wv.fovy = 30.0f, camera_wv.aspect_ratio = 1.0f;
	camera_wv.near_c = 5.0f;
	camera_wv.far_c = 10000.0f;
	set_ViewProjectionMatrix_for_world_viewer();
}

void initialize_camera(void)
{
	initialize_world_camera();

	// the transformation that moves the driver's camera frame from car body's MC to driver seat
	ModelMatrix_CAR_BODY_to_DRIVER = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.5f, 2.5f));
	ModelMatrix_CAR_BODY_to_DRIVER = glm::rotate(ModelMatrix_CAR_BODY_to_DRIVER,
												 TO_RADIAN * 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
}

void renew_cam_fovy(const float del)
{
	camera_wv.fovy = std::max(camera_wv.fovy + del * CAM_FOVY_COEFF, 0.0f);
	camera_wv.fovy = std::min(camera_wv.fovy, 180.0f);
}

/*********************************  END: camera *********************************/