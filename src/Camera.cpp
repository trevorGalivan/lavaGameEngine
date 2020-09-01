#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include <numbers>

const double pi = 3.1415926535897;

Camera::Camera() : position(0., 0., 0), horiAngle(0.), vertAngle(0.) {}
Camera::Camera(glm::dvec3 position_, double horiAngle_, double vertAngle_): position(position_), horiAngle(horiAngle_), vertAngle(vertAngle_) {}
Camera::Camera(glm::dvec3 position_, glm::dvec3 target): position(position_)
{
	lookAt(target);
}


void Camera::setAngles(double horizontal, double vertical) // Will wrap horizontal angle to [0, 2*pi), and clamp vertical angle to [-pi, pi]
{
	horiAngle = horizontal - floor(horizontal / (2*pi))*2*pi; // Unlike fmod, this will NEVER return a negative number
	vertAngle = fmax(fmin(vertical, pi), -1.*pi);
}

void Camera::rotate(double dHorizontal, double dVertical)  // Will wrap horizontal angle to [0, 2*pi), and clamp vertical angle to [-pi, pi]
{
	horiAngle += dHorizontal;
	horiAngle = horiAngle - floor(horiAngle / (2. * pi)) * 2 * pi;
	vertAngle = fmax(fmin(vertAngle + dVertical, pi * 0.5), -0.5 * pi);
}

// These could be optimized by calculating all directions at once and reusing calculated sin and cosine values, but these are only calculated once per frame anyway
glm::dvec3 Camera::getLookDir() // Gets a unit vector pointing in the current camera direction
{
	double y = sin(vertAngle);
	double x = cos(horiAngle) * cos(vertAngle);
	double z = -sin(horiAngle) * cos(vertAngle);
	return glm::dvec3(x, y, z);
}
glm::dvec3 Camera::getLookR()   // Gets unit vector orthogonal to LookDir, with y component equal to zero. Points "right" from camera
{
	double x =  sin(horiAngle);
	double z =  cos(horiAngle);
	return glm::dvec3(x, 0., z);
}
glm::dvec3 Camera::getLookU()   // Gets unit vector orthogonal to both LookDir and LookR. Points "Upwards" out of camera
{
	double y = cos(vertAngle);
	double x = cos(horiAngle) * -1 * sin(vertAngle);
	double z = -sin(horiAngle) * -1 * sin(vertAngle);
	return glm::dvec3(x, y, z);
}

glm::mat4x4 Camera::getWorldMatrix()
{
	glm::mat4x4 ret(glm::vec4(1, 0, 0, 0), glm::vec4(0, 1, 0, 0), glm::vec4(0, 0, 1, 0), glm::vec4(0, 0, 0, 1));
	ret = glm::rotate(ret, float(vertAngle), glm::vec3(-1.f, 0.f, 0.f)); 
	ret = glm::rotate(ret, float(-pi/2 + horiAngle), glm::vec3(0.f, -1.f, 0.f));

	ret = glm::translate(ret, -1.f * glm::vec3(position));

	return ret;
}

glm::mat4x4 Camera::getProjectionMatrix()
{
	return glm::perspective(float(horizontalFOV / aspectRatio), float(aspectRatio), 0.05f, 10000.f); // TODO: fix this (or maybe world matrix?) so that vertex winding doesnt get flipped
}

void Camera::setPos(glm::dvec3 position_)
{
	position = position_;
}
glm::dvec3 Camera::getPos()
{
	return position;
}
void Camera::translate(glm::dvec3 dPosition) // Translates camera position in world coordinates
{
	position += dPosition;
}
void Camera::fly(glm::dvec3 dPosition) // Translates camera position in screen coordinates
{
	position += dPosition.x * getLookDir() + dPosition.y * getLookU() + dPosition.z * getLookR();
}
void Camera::walk(glm::dvec3 dPosition) // Movement similar to fps. x and z coords move horizontally according to look direction, y translates straight up and down
{
	glm::dvec3 forward = getLookDir();
	forward.y = 0;
	forward = glm::normalize(forward);
	position += dPosition.x * getLookR() + dPosition.y * glm::dvec3(0., 1., 0.) + dPosition.z * forward;
}

void Camera::lookAt(glm::dvec3 target)
{
	glm::dvec3 direction = (target - position);
	
	vertAngle = atan2(direction.y, glm::dvec2(direction.x, direction.z).length());
	
	horiAngle = atan2(-direction.z, direction.x);
}