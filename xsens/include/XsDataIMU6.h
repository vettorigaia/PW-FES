#ifndef __XSENS_DATA_IMU6_H
    #define __XSENS_DATA_IMU6_H

#include <xstypes.h>

struct Vector3
{
	float x;
	float y;
	float z;

	Vector3() { }

	Vector3(const float& a, const float& b, const float& c)
		: x(a), y(b), z(c)
	{ }

};

class XsDataIMU6
{
public:
	Vector3 axel;
	Vector3 gyro;
	uint16_t counter;

	XsDataIMU6() { }

	XsDataIMU6(const uint16_t& c, const Vector3& a, const Vector3& b)
		: counter(c), axel(a), gyro(b)
	{ }

	XsDataIMU6(const uint16_t& c, const XsVector& a, const XsVector& b) {

        counter = c;
        
		axel.x = a.value(0);
		axel.y = a.value(1);
		axel.z = a.value(2);

		gyro.x = b.value(0);
		gyro.y = b.value(1);
		gyro.z = b.value(2);

	}

	std::string printAxel() const {
		std::string a = std::to_string(axel.x) + "," + std::to_string(axel.y) + "," + std::to_string(axel.z);
		return a;
	}

	std::string printGyro() const {
		std::string a = std::to_string(gyro.x) + "," + std::to_string(gyro.y) + "," + std::to_string(gyro.z);
		return a;
	}

};

#endif
