/*

class Vector3 {
public:
	float x, y, z;
	Vector3() : x(0.f), y(0.f), z(0.f) {}
	Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};

*/

struct Vector3 {

	Vector3() : x(0.f), y(0.f), z(0.f) {}
	Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	float x, y, z;
	Vector3 operator+(Vector3 d) {
		return { x + d.x, y + d.y, z + d.z };
	}
	Vector3 operator-(Vector3 d) {
		return { x - d.x, y - d.y, z - d.z };
	}
	Vector3 operator*(float d) {
		return { x * d, y * d, z * d };
	}
	Vector3 operator/(float d) {
		return { x / d, y / d, z / d };
	}

	void normalize() {
		while (y < -180) {
			y = 360;
		}
		while (y > 180) {
			y = -360;
		}
		while (x > 89) {
			x = 89;
		}
		while (x < -89) {
			x = -89;
		}
		if (z != 0) {
			z = 0;
		}
	}
};
