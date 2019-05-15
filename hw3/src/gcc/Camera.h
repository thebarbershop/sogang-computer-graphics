/*********************************  START: camera *********************************/
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

void set_ViewMatrix_for_world_viewer(void)
{
	ViewMatrix = glm::mat4(camera_wv.uaxis.x, camera_wv.vaxis.x, camera_wv.naxis.x, 0.0f,
						   camera_wv.uaxis.y, camera_wv.vaxis.y, camera_wv.naxis.y, 0.0f,
						   camera_wv.uaxis.z, camera_wv.vaxis.z, camera_wv.naxis.z, 0.0f,
						   0.0f, 0.0f, 0.0f, 1.0f);
	ViewMatrix = glm::translate(ViewMatrix, -camera_wv.pos);
}

void set_ViewMatrix_for_driver(void)
{
	glm::mat4 Matrix_CAMERA_driver_inverse;

	Matrix_CAMERA_driver_inverse = ModelMatrix_CAR_BODY * ModelMatrix_CAR_BODY_to_DRIVER;

	ViewMatrix = glm::affineInverse(Matrix_CAMERA_driver_inverse);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

/*
void set_ViewMatrix_for_driver2(void) { // This version does not use the glm::affineInverse(*) function.
	glm::mat4 Matrix_CAMERA;

	Matrix_CAMERA = glm::rotate(glm::mat4(1.0f), -rotation_angle_car, glm::vec3(0.0f, 1.0f, 0.0f));
	Matrix_CAMERA = glm::translate(Matrix_CAMERA, glm::vec3(20.0f, 4.89f, 0.0f));
	Matrix_CAMERA = glm::rotate(Matrix_CAMERA, 90.0f*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	Matrix_CAMERA = glm::translate(Matrix_CAMERA, glm::vec3(-3.0f, 0.5f, 2.5f));
	Matrix_CAMERA = glm::rotate(Matrix_CAMERA, TO_RADIAN*90.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	ViewMatrix = glm::affineInverse(Matrix_CAMERA);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}
*/

void initialize_camera(void)
{
	camera_type = CAMERA_WORLD_VIEWER;

	//ViewMatrix = glm::lookAt(glm::vec3(0.0f, 10.0f, 75.0f), glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	ViewMatrix = glm::lookAt(glm::vec3(75.0f, 75.0f, 100.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	camera_wv.uaxis = glm::vec3(ViewMatrix[0].x, ViewMatrix[1].x, ViewMatrix[2].x);
	camera_wv.vaxis = glm::vec3(ViewMatrix[0].y, ViewMatrix[1].y, ViewMatrix[2].y);
	camera_wv.naxis = glm::vec3(ViewMatrix[0].z, ViewMatrix[1].z, ViewMatrix[2].z);
	camera_wv.pos = -(ViewMatrix[3].x * camera_wv.uaxis + ViewMatrix[3].y * camera_wv.vaxis + ViewMatrix[3].z * camera_wv.naxis);

	camera_wv.move = 0;
	camera_wv.fovy = 30.0f, camera_wv.aspect_ratio = 1.0f;
	camera_wv.near_c = 5.0f;
	camera_wv.far_c = 10000.0f;

	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	// the transformation that moves the driver's camera frame from car body's MC to driver seat
	ModelMatrix_CAR_BODY_to_DRIVER = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.5f, 2.5f));
	ModelMatrix_CAR_BODY_to_DRIVER = glm::rotate(ModelMatrix_CAR_BODY_to_DRIVER,
												 TO_RADIAN * 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
}

#define CAM_TSPEED 0.05f
void renew_cam_position(int del)
{
	camera_wv.pos += CAM_TSPEED * del * (-camera_wv.naxis);
}

#define CAM_RSPEED 0.1f
void renew_cam_orientation_rotation_around_v_axis(int angle)
{
	glm::mat3 RotationMatrix;

	RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0), CAM_RSPEED * TO_RADIAN * angle, camera_wv.vaxis));
	camera_wv.uaxis = RotationMatrix * camera_wv.uaxis;
	camera_wv.naxis = RotationMatrix * camera_wv.naxis;
}
/*********************************  END: camera *********************************/