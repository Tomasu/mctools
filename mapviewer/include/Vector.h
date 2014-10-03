#ifndef VECTOR_H_GUARD
#define VECTOR_H_GUARD

class Vector3D
{
	public:
		//default constructor
		Vector3D(float X = 0, float Y = 0, float Z = 0) : x(X), y(Y), z(Z) { }
		~Vector3D() { }

		//calculate and return the magnitude of this vector
		float magnitude()
		{
			return sqrtf(x * x + y * y + z * z);
		}

		bool operator==(const Vector3D &v) const
		{
			return (x == v.x && y == v.y && z == v.z);
		}
		
		//multiply this vector by a scalar
		Vector3D operator*(float num) const
		{
			return Vector3D(x * num, y * num, z * num);
		}

		//pass in a vector, pass in a scalar, return the product
		friend Vector3D operator*(float num, Vector3D const &vec)
		{
			return Vector3D(vec.x * num, vec.y * num, vec.z * num);
		}

		//add two vectors
		Vector3D operator+(const Vector3D &vec) const
		{
			return Vector3D(x + vec.x, y + vec.y, z + vec.z);
		}
		
		Vector3D &operator+=(const Vector3D &vec)
		{
			x += vec.x;
			y += vec.y;
			z += vec.z;
			return *this;
		}

		//subtract two vectors
		Vector3D operator-(const Vector3D &vec) const
		{
			return Vector3D(x - vec.x, y - vec.y, z - vec.z);
		}

		//normalize this vector
		void normalize()
		{
			float magnitude = sqrtf(x * x + y * y + z * z);
			x /= magnitude;
			y /= magnitude;
			z /= magnitude;
		}
		
		//calculate and return dot product
		float dot(const Vector3D &vec) const
		{
			return x * vec.x + y * vec.y + z * vec.z;
		}

		//calculate and return cross product
		Vector3D cross(const Vector3D &vec) const
		{
			return Vector3D(
				y * vec.z - z * vec.y,
				z * vec.x - x * vec.z,
				x * vec.y - y * vec.x
			);
		}
		
		void translate(float X, float Y, float Z)
		{
			x += X;
			y += Y;
			z += Z;
		}
		
	public:
		float x, y, z;
};

namespace std {
	template <>
	struct hash < Vector3D >
	{
		public:
			size_t operator()(const Vector3D &v ) const
			{
				size_t h = std::hash<float>()(v.x) ^ std::hash<float>()(v.y) ^ std::hash<float>()(v.z);
				return  h ;
			}
	};
}

#endif /* VECTOR_H_GUARD */
