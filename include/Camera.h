#pragma once
#include <glm/glm.hpp>

class Camera
{

public:
	glm::dvec3 position;

	double horiAngle; // In radians, measured from +'ve x axis, towards negative z axis
	double vertAngle; // in radians, measured up from the horizon. Range (-pi, pi).
	double aspectRatio = 1.;
	double horizontalFOV = 1.570796327;

	Camera();
	Camera(glm::dvec3 position, double horiAngle, double vertAngle);
	Camera(glm::dvec3 position, glm::dvec3 target);

	void setAngles(double horizontal, double vertical); // Will wrap horizontal angle to [0, 2*pi), and clamp vertical angle to (-pi, pi)
	void rotate(double dHorizontal, double dVertical);  // Will wrap horizontal angle to [0, 2*pi), and clamp vertical angle to (-pi, pi)
	glm::dvec3 getLookDir(); // Gets a unit vector pointing in the current camera direction
	glm::dvec3 getLookR();   // Gets unit vector orthogonal to LookDir, with y component equal to zero. Points "right" from camera
	glm::dvec3 getLookU();   // Gets unit vector orthogonal to both LookDir and LookR. Points "Upwards" out of camera

	glm::mat4x4 getProjectionMatrix();
	glm::mat4x4 getWorldMatrix();
	void setPos(glm::dvec3 position);
	glm::dvec3 getPos();
	void translate(glm::dvec3 dPosition); // Translates camera position in world coordinates
	void fly(glm::dvec3 dPosition); // Translates camera position in screen coordinates
	void walk(glm::dvec3 dPosition); // Movement similar to fps. x and y coords move horizontally according to look direction, y translates straight up and down
	void lookAt(glm::dvec3 target);
};

