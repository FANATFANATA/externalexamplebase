#pragma once

#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>

#if defined(__ARM_NEON__)
#include <arm_neon.h>
#endif

struct Vector4 {
    float x, y, z, w;

    Vector4() : x(0), y(0), z(0), w(0) {}
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};

struct Quaternion {
    float x, y, z, w;

    Quaternion() : x(0), y(0), z(0), w(1) {}
    Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};

struct Vector3 {
    union {
        struct { float x, y, z; };
        float data[3];
    };

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    float& operator[](int i) { return data[i]; }
    const float& operator[](int i) const { return data[i]; }

    Vector3& operator+=(const Vector3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vector3& operator-=(const Vector3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    Vector3& operator*=(const Vector3& o) { x *= o.x; y *= o.y; z *= o.z; return *this; }
    Vector3& operator/=(const Vector3& o) { x /= o.x; y /= o.y; z /= o.z; return *this; }
    Vector3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    Vector3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }

    Vector3 operator+(const Vector3& o) const { return Vector3(x + o.x, y + o.y, z + o.z); }
    Vector3 operator-(const Vector3& o) const { return Vector3(x - o.x, y - o.y, z - o.z); }
    Vector3 operator*(const Vector3& o) const { return Vector3(x * o.x, y * o.y, z * o.z); }
    Vector3 operator/(const Vector3& o) const { return Vector3(x / o.x, y / o.y, z / o.z); }
    Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }
    Vector3 operator/(float s) const { return Vector3(x / s, y / s, z / s); }

    bool operator==(const Vector3& o) const { return x == o.x && y == o.y && z == o.z; }
    bool operator!=(const Vector3& o) const { return x != o.x || y != o.y || z != o.z; }

    static float Angle(const Vector3& a, const Vector3& b) {
        float d = Dot(a, b) / (Magnitude(a) * Magnitude(b));
        return acosf(d < -1.f ? -1.f : (d > 1.f ? 1.f : d));
    }
    static float Dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
    static Vector3 Cross(const Vector3& a, const Vector3& b) {
        return Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
    }
    static float Distance(const Vector3& a, const Vector3& b) {
        float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }
    static float Magnitude(const Vector3& v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }
    static float SqrMagnitude(const Vector3& v) { return v.x * v.x + v.y * v.y + v.z * v.z; }
    static Vector3 Normalize(const Vector3& v) {
        float m = Magnitude(v);
        return m > 0.0001f ? v / m : Vector3(0, 0, 0);
    }
    static Vector3 Lerp(const Vector3& a, const Vector3& b, float t) {
        t = t < 0.f ? 0.f : (t > 1.f ? 1.f : t);
        return a + (b - a) * t;
    }
    static Vector3 LerpUnclamped(const Vector3& a, const Vector3& b, float t) {
        return a + (b - a) * t;
    }
    static Vector3 Max(const Vector3& a, const Vector3& b) {
        return Vector3(a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y, a.z > b.z ? a.z : b.z);
    }
    static Vector3 Min(const Vector3& a, const Vector3& b) {
        return Vector3(a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y, a.z < b.z ? a.z : b.z);
    }
    static Vector3 Scale(const Vector3& a, const Vector3& b) {
        return Vector3(a.x * b.x, a.y * b.y, a.z * b.z);
    }
    static Vector3 MoveTowards(const Vector3& current, const Vector3& target, float maxDistanceDelta) {
        float dx = target.x - current.x, dy = target.y - current.y, dz = target.z - current.z;
        float sqdist = dx * dx + dy * dy + dz * dz;
        if (sqdist == 0 || (maxDistanceDelta >= 0 && sqdist <= maxDistanceDelta * maxDistanceDelta))
            return target;
        float dist = sqrtf(sqdist);
        return Vector3(current.x + dx / dist * maxDistanceDelta, current.y + dy / dist * maxDistanceDelta, current.z + dz / dist * maxDistanceDelta);
    }
    static Vector3 ClampMagnitude(const Vector3& v, float maxLength) {
        float sqm = SqrMagnitude(v);
        if (sqm <= maxLength * maxLength) return v;
        float m = sqrtf(sqm);
        return v / m * maxLength;
    }
    static Vector3 Orthogonal(const Vector3& v) {
        return fabsf(v.x) < fabsf(v.y) ? Vector3(1, 0, 0) : Vector3(0, 1, 0);
    }
};
