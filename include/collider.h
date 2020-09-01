#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>
#include <limits>
#include <complex>
#include <algorithm>
#include <variant>

struct AABB
{
	glm::dvec2 min;
	glm::dvec2 max;
};

struct aRect // axis aligned rectangle
{
	glm::dvec2 halfExtents; // (x, y) -> (half width, half height)

	aRect() = default;
	aRect(glm::dvec2 size) : halfExtents(size) {}

	void makeAABB(glm::dvec2 offset, AABB& outputAAB) {
		outputAAB = { offset - halfExtents, offset + halfExtents };
	}
};

struct Circle
{
	double radius;

	void makeAABB(glm::dvec2 offset, AABB &outputAABB) {
		outputAABB = { offset - radius, offset + radius };
	}
};

struct Contact {
	glm::dvec2 pos;
	glm::dvec2 normal;
	double depth;
};

using vShape_t = std::variant<aRect, Circle>;

class Collider
{
public:
	AABB AABB;

	glm::dvec2 pos = glm::vec3(0.);

	vShape_t shape;

	Collider() = default; 
	Collider(vShape_t shape) : shape(shape) {}

	void updatePos(glm::dvec2 pos) {

		std::visit([pos, this](auto& e) { e.makeAABB(pos, this->AABB); }, shape);

		this->pos = pos;
	}
};
// check collision between two AABB
inline bool collideAABB_AABB(AABB b1, AABB b2) {
	return b1.min.x < b2.max.x&& b2.min.x < b1.max.x&&
		b1.min.y < b2.max.y&& b2.min.y < b1.max.y;
}

// check if point is contained within AABB
inline bool collideAABB_Point(AABB b, glm::vec2 p) {
	return p.x < b.max.x&& p.x > b.min.x &&
		p.y < b.max.y&& p.y > b.min.y;
}
inline bool collideaRect_aRect(Collider& c1, Collider& c2, Contact& contact_out)
{
	const aRect& r1 = *std::get_if<aRect>(&c1.shape);
	const aRect& r2 = *std::get_if<aRect>(&c2.shape);

	auto AB = c2.pos - c1.pos;


	auto penetration = r1.halfExtents + r2.halfExtents;
	penetration.x -= fabs(AB.x);
	penetration.y -= fabs(AB.y);

	// Collision right on the diagonal
	//if (abs(penetration.x - penetration.y) < 0.1) [[unlikely]] {
	//	contact_out.normal = glm::normalize(glm::vec2(std::copysign(penetration.x, AB.x), std::copysign(penetration.y, AB.y)));
	//	contact_out.depth = glm::length(penetration);
	//	return true;
	//}

	if (abs(penetration.x - penetration.y) < 0.05) [[unlikely]] {
		return false;
	}


	if (penetration.x < penetration.y) {
		// we have collided from the sides
		if (AB.x > 0) { // B is to the right of A
			contact_out.normal = glm::vec2(1., 0.);
			contact_out.depth = penetration.x;
			//
		}
		else {
			contact_out.normal = glm::vec2(-1., 0.);
			contact_out.depth = penetration.x;
		}
	}
	else {
		// we have collided vertically
		if (AB.y > 0) { // B is above A
			contact_out.normal = glm::vec2(0., 1.);
			contact_out.depth = penetration.y;
			//
		}
		else {
			contact_out.normal = glm::vec2(0., -1.);
			contact_out.depth = penetration.y;
		}
	}

	return true; // assumed, since AABB would rule out any non-collisions.
}
/*
inline bool collideaRect_Circle(Collider& c1, Collider& c2, glm::vec2& normal_out)
{
	aRect &r = *std::get_if<aRect>(&c1.shape);
	Circle &s = *std::get_if<Circle>(&c1.shape);

	// assumes centre of circle is outside of rectangle 
	glm::vec2 C; // Point on the surface of the rectangle that is closest to the circle 
	C.x = std::min(std::max(c1.pos.x - r.halfSize.x, c2.pos.x), c1.pos.x = r.halfSize.x);
	C.y = std::min(std::max(c1.pos.y - r.halfSize.y, c2.pos.y), c1.pos.y = r.halfSize.y);

	auto CC = c2.pos - C;
	if (CC.x * CC.x + CC.y * CC.y > s.radius * s.radius) return false;
	
	normal_out = glm::normalize(CC) * (s.radius - glm::length(CC));

	return true;
}
inline bool collideCircle_Circle(Collider& c1, Collider& c2, glm::vec2& normal_out)
{
	Circle &s1 = *std::get_if<Circle>(&c1.shape), s2= *std::get_if<Circle>(&c1.shape);
	auto AB = c2.pos - c1.pos;
	auto sqrDist = AB.x * AB.x + AB.y * AB.y;
	if (sqrDist > (s1.radius + s2.radius) * (s1.radius + s2.radius)) return false;

	normal_out = AB / sqrt(sqrDist) * (sqrt(sqrDist) - (s1.radius + s2.radius)); // subtract sum of radii from distance between centres to get penetration distance
	return true;
}
*/
inline bool collide(Collider& c1, Collider& c2, Contact& contact_out) {
	if (!collideAABB_AABB(c1.AABB, c2.AABB)) return false;
	if (std::holds_alternative<aRect>(c1.shape) ) { 
		if (std::holds_alternative<aRect>(c2.shape)) { // two rect
			return collideaRect_aRect(c1, c2, contact_out);
		}
		else if (std::holds_alternative<Circle>(c2.shape)){
			//return collideaRect_Circle(c1, c2, contact_out);
		}
	}

	if (std::holds_alternative<Circle>(c1.shape) && std::holds_alternative<Circle>(c2.shape)) {
		if (std::holds_alternative<aRect>(c2.shape)) { // two rect
			//return collideaRect_Circle(c2, c1, contact_out); // BROKEN
		}
		else if (std::holds_alternative<Circle>(c2.shape)) {
			//return collideCircle_Circle(c1, c2, contact_out);
		}
	}

	return false; // error
	// Resort to GJK algorithm
}



/* Support functions for the GJK collision algorithm for complex hull */
#ifdef PROBABLY_NOT_DEFINED_YET
#define GJK_MAXITER 20

inline size_t getLargestDot(const std::vector<glm::vec3>& vertices, glm::vec3 dir) {
	size_t bigIndex = 0;
	float max = std::numeric_limits<float>::min();
	for (auto i = 0; i < vertices.size(); ++i) {
		float product = glm::dot(vertices[i], dir);
		if (product > max) {
			bigIndex = i;
			max = product;
		}
	}
	return bigIndex;
}

inline glm::vec3 gjkSupport(const Collider& c1, const Collider& c2, glm::vec3 dir) {
	return c1.support(dir) - c2.support(-dir);
}

struct simplex {
	glm::vec3 a, b, c, d;
	int dim;
};

inline void simp3(simplex& simp, glm::vec3& D) {
	/*           /
	 *          C   origin could be here [1]
	 *          | \
	 *          |   \   /
	 * (origin  |     A    origin cant be here, since A is furthest point in this direction
	 * cannot   |   /   \
	 * be here) | /
	 *          B   origin could be here [4]
	 *           \
	 * origin can also be above or below triangle, for a total of 5 cases
	 */

	auto AB = simp.b - simp.a;
	auto AC = simp.b - simp.a;
	auto ABC = glm::cross(AB, AC); // into page

	auto AO = -simp.a;

	if (glm::dot(glm::cross(AB, ABC), AO) > 0) { //[1]
		simp.c = simp.a;
		simp.dim = 2;
		D = glm::cross(glm::cross(AB, AO), AB);
		return;
	}
	if (glm::dot(glm::cross(ABC, AC), AO) > 0) { //[4]
		simp.b = simp.a;
		simp.dim = 2;
		D = glm::cross(glm::cross(AC, AO), AC);
		return;
	}

	simp.dim = 3;
	if (glm::dot(ABC, AO) > 0) { // above
		simp.d = simp.c;
		simp.c = simp.b;
		simp.b = simp.a;
		
		D = ABC;
		return;
	}
	//below
	simp.d = simp.b;
	simp.b = simp.a;
	//simp.dim = 3;
	D = -ABC;
	return;

	/* OLD:
	if (glm::dot(glm::cross(ABC, AC), AO) > 0) {
		if (glm::dot(AC, AO) > 0) { // [1]
			simp.b = simp.a;
			// simp.c = simp.c
			simp.dim = 2;
			D = glm::cross(AC, glm::cross(AO, AC));
			return;
		} else {
			// skip to [*]
		}
	} else {
		if (glm::dot(glm::cross(AB, ABC), AO) > 0) {
			// skip to [*]
		} else { // above or below triangle
			if (glm::dot(ABC, AO) > 0) { // [2]
				simp.d = simp.c;
				simp.c = simp.b;
				simp.b = simp.a;
				//simp.dim = 3;
				D = ABC;
				return;
			} else { // [3]
				simp.d = simp.b;
				//simp.c = simp.c;
				simp.b = simp.a;
				//simp.dim = 3;
				D = -ABC;
				return;
			}
		}
	}

	// [*]
	if (glm::dot(AB, AO) > 0) { // [4]
		simp.c = simp.a;
		simp.dim = 2;
		D = glm::cross(glm::cross(AB, AO), AB);
	} else { //[5]
		simp.b = simp.a;
		simp.dim = 1;
		D = AO;
	}

	return;*/
}

inline bool simp4(simplex& simp, glm::vec3& D) { // returns if point inside tetraherdon
	// This is really just the triangle case, repeated for each side

	simp.dim = 3; 
	glm::vec3 AO = -simp.a;
	
	glm::vec3 ABC = glm::cross(simp.b - simp.a, simp.c - simp.a);
	if (glm::dot(ABC, D) > 0) { // point D discarded
		simp.d = simp.c;
		simp.c = simp.b;
		simp.b = simp.a;
		D = ABC;
		return false;
	}

	glm::vec3 ACD = glm::cross(simp.c - simp.a, simp.d - simp.a);
	if (glm::dot(ACD, D) > 0) { // point B discarded
		//simp.d = simp.d;
		//simp.c = simp.c;
		simp.b = simp.a;
		D = ACD;
		return false;
	}

	glm::vec3 ADB = glm::cross(simp.d - simp.a, simp.b - simp.a);
	if (glm::dot(ADB, D) > 0) { // point C discarded
		simp.c = simp.d;
		simp.d = simp.b;
		simp.b = simp.a;
		D = ADB;
		return false;
	}

	return true; // origin is enclosed
}

inline bool collideConvex_Convex(Collider c1, Collider c2) {
	if (!collideAABB_AABB(c1.boundingBox, c2.boundingBox)) return false;
	simplex simp;

	glm::vec3 D = glm::vec3(0., 0., 1.);

	simp.c = gjkSupport(c1, c2, D);
	D = glm::normalize(simp.c);
	simp.b = gjkSupport(c1, c2, D);

	simp.dim = 2; // we know we have two points

	if (glm::dot(simp.b, D) < 0) { return false; }

	D = cross(cross(simp.c - simp.b, -simp.b), simp.c - simp.b); // maybe TODO: check for |D| = 0 case. (Not sure if neccesary)

	for (int i = 0; i < GJK_MAXITER; ++i)
	{
		simp.a = gjkSupport(c1, c2, D); // a is always the most recent point. 
		simp.dim++;

		if (glm::dot(simp.a, D) < 0) { return false; } 

		if (simp.dim == 3) { // triangle case
			simp3(simp, D);
		}
		else if (simp4(simp, D)) { // tetrahedron case
			//if (mtv) *mtv = EPA(a, b, c, d, coll1, coll2);
			return true;
		}
	}
}
#endif