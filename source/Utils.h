#pragma once
#include <array>
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1
			// d = ray.direction

			const Vector3 cameraToSphere{ ray.origin - sphere.origin };	
			const float b{ Vector3::Dot(2 * ray.direction, cameraToSphere) };
			const float c{ Vector3::Dot(cameraToSphere, cameraToSphere) - Square(sphere.radius) };

			const float discriminant{ Square(b) - 4 * c };

			float t{};

			// no hit if discriminant is 0
			hitRecord.didHit = discriminant > 0;

			if (!hitRecord.didHit)
				return false;

			t = (-b - sqrtf(discriminant)) * 0.5f;

			// check if inside of min and max
			hitRecord.didHit = t > ray.min && t < ray.max;

			if (!hitRecord.didHit)
				return false;


			if (t < ray.min)
				t = (-b + sqrtf(discriminant)) * 0.5f;

			hitRecord.origin = ray.origin + t * ray.direction;
			hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
			hitRecord.t = t;
			hitRecord.materialIndex = sphere.materialIndex;

			return true;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1

			// calculate dot between ray and normal
			const float unitProjection{ Vector3::Dot(ray.direction, plane.normal) };

			// if ray is coming from behind, ignore
			if (unitProjection >= 0)
				return false;

			// calculate the distance to the intersection
			const float height{ Vector3::Dot((plane.origin - ray.origin) , plane.normal) };

			const float t{ height / unitProjection };

			// check distance in range
			hitRecord.didHit = t > ray.min && t < ray.max;
			if (!hitRecord.didHit)
				return false;

			hitRecord.origin = ray.origin + t * ray.direction;
			hitRecord.normal = plane.normal;
			hitRecord.materialIndex = plane.materialIndex;
			hitRecord.t = t;

			return true;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			// Get intersection point with plane
			float normalViewDot{ Vector3::Dot(triangle.normal, ray.direction) };

			// in case of shadows inverse the dot
			if (ignoreHitRecord)
				normalViewDot *= -1;

			switch (triangle.cullMode)
			{
			case TriangleCullMode::FrontFaceCulling:
				if ( normalViewDot <= 0)
					return false;
				break;
			case TriangleCullMode::BackFaceCulling:
				if (normalViewDot >= 0)
					return false;
				break;
			case TriangleCullMode::NoCulling:
				if (abs(normalViewDot) < FLT_EPSILON)
					return false;
				break;
			}

			const Vector3 L{ triangle.v0 - ray.origin };
			const float t{ Vector3::Dot(L, triangle.normal) / Vector3::Dot(ray.direction, triangle.normal) };

			if (t < ray.min || t > ray.max)
				return false;

			const Vector3 intersection{ ray.origin + ray.direction * t };

			// Check if intersection point is in the triangle
			std::array vertexVec{ triangle.v0, triangle.v1, triangle.v2 };
			const int nrVertices{ int(vertexVec.size()) };

			for (int index{}; index < nrVertices; ++index)
			{
				const Vector3 e{ vertexVec[(index + 1) % nrVertices] - vertexVec[index] };
				const Vector3 p{ intersection - vertexVec[index] };
				if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) < 0)
					return false;
			}

			hitRecord.origin = intersection;
			hitRecord.didHit = true;
			hitRecord.normal = triangle.normal;
			hitRecord.materialIndex = triangle.materialIndex;
			hitRecord.t = t;
			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			
			return false;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			//todo W3
			return { light.origin - origin };
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3
			switch (light.type)
			{
			case LightType::Point:
				return { light.color * (light.intensity / (light.origin - target).SqrMagnitude()) };
			case LightType::Directional:
				return { light.color * light.intensity };
			}
			return{};
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}