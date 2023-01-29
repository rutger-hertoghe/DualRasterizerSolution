#pragma once
#include <fstream>
#include <vector>
#include "Math.h"
#include "Vector3.h"
#include "Structs.h"

namespace dae
{
	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vertex_PosTex>& vertices, std::vector<uint32_t>& indices, bool flipAxisAndWinding = true)
		{
#ifdef DISABLE_OBJ
			// >> Comment/Remove '#define DISABLE_OBJ'
			assert(false && "OBJ PARSER not enabled! Check the comments in Utils::ParseOBJ");

#else

			std::ifstream file(filename);
			if (!file)
				return false;

			std::vector<Vector3> positions{};
			std::vector<Vector3> normals{};
			std::vector<Vector2> UVs{};

			vertices.clear();
			indices.clear();

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

					positions.emplace_back(x, y, z);
				}
				else if (sCommand == "vt")
				{
					// Vertex TexCoord
					float u, v;
					file >> u >> v;
					UVs.emplace_back(u, 1 - v);
				}
				else if (sCommand == "vn")
				{
					// Vertex Normal
					float x, y, z;
					file >> x >> y >> z;

					normals.emplace_back(x, y, z);
				}
				else if (sCommand == "f")
				{
					//if a face is read:
					//construct the 3 vertices, add them to the vertex array
					//add three indices to the index array
					//add the material index as attibute to the attribute array
					//
					// Faces or triangles
					Vertex_PosTex vertex{};
					size_t iPosition, iTexCoord, iNormal;

					uint32_t tempIndices[3];
					for (size_t iFace = 0; iFace < 3; iFace++)
					{
						// OBJ format uses 1-based arrays
						file >> iPosition;
						vertex.position = positions[iPosition - 1];

						if ('/' == file.peek())//is next in buffer ==  '/' ?
						{
							file.ignore();//read and ignore one element ('/')

							if ('/' != file.peek())
							{
								// Optional texture coordinate
								file >> iTexCoord;
								vertex.uv = UVs[iTexCoord - 1];
							}

							if ('/' == file.peek())
							{
								file.ignore();

								// Optional vertex normal
								file >> iNormal;
								vertex.normal = normals[iNormal - 1];
							}
						}

						vertices.push_back(vertex);
						tempIndices[iFace] = uint32_t(vertices.size()) - 1;
						//indices.push_back(uint32_t(vertices.size()) - 1);
					}

					indices.push_back(tempIndices[0]);
					if (flipAxisAndWinding)
					{
						indices.push_back(tempIndices[2]);
						indices.push_back(tempIndices[1]);
					}
					else
					{
						indices.push_back(tempIndices[1]);
						indices.push_back(tempIndices[2]);
					}
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');
			}

			//Cheap Tangent Calculations
			for (uint32_t i = 0; i < indices.size(); i += 3)
			{
				uint32_t index0 = indices[i];
				uint32_t index1 = indices[size_t(i) + 1];
				uint32_t index2 = indices[size_t(i) + 2];

				const Vector3& p0 = vertices[index0].position;
				const Vector3& p1 = vertices[index1].position;
				const Vector3& p2 = vertices[index2].position;
				const Vector2& uv0 = vertices[index0].uv;
				const Vector2& uv1 = vertices[index1].uv;
				const Vector2& uv2 = vertices[index2].uv;

				const Vector3 edge0 = p1 - p0;
				const Vector3 edge1 = p2 - p0;
				const Vector2 diffX = Vector2(uv1.x - uv0.x, uv2.x - uv0.x);
				const Vector2 diffY = Vector2(uv1.y - uv0.y, uv2.y - uv0.y);
				float r = 1.f / Vector2::Cross(diffX, diffY);

				Vector3 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
				vertices[index0].tangent += tangent;
				vertices[index1].tangent += tangent;
				vertices[index2].tangent += tangent;
			}

			//Fix the tangents per vertex now because we accumulated
			for (auto& v : vertices)
			{
				v.tangent = Vector3::Reject(v.tangent, v.normal).Normalized();

				if (flipAxisAndWinding)
				{
					v.position.z *= -1.f;
					v.normal.z *= -1.f;
					v.tangent.z *= -1.f;
				}

			}

			return true;
#endif
		}
#pragma warning(pop)

		// Weight of a vertex uses not vertex itself, but next two vertices
		static float CalcWeight(const Vertex_Out& nextVertex, const Vertex_Out& previousVertex, const Vector2& vectorToPixel, float areaParallelogram)
		{

			const Vector2 v0{ nextVertex.position.x, nextVertex.position.y };
			const Vector2 v1{ previousVertex.position.x, previousVertex.position.y };
			const Vector2 pixel{ vectorToPixel.x, vectorToPixel.y };

			const Vector2 v0ToV1{ v1 - v0 };
			const Vector2 v0ToPixel{ pixel - v0 };

			return Vector2::Cross(v0ToV1, v0ToPixel) / areaParallelogram;
		}

		static float Interpolate(float v0value, float v1value, float v2value, float weight0, float weight1, float weight2)
		{
			return (1 / (weight0 / v0value + weight1 / v1value + weight2 / v2value));
		}

		static float CalcAreaParallelogram(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2)
		{
			Vector2 v0ToV1{ v1.position.x - v0.position.x, v1.position.y - v0.position.y };
			Vector2 v0ToV2{ v2.position.x - v0.position.x, v2.position.y - v0.position.y };

			return Vector2::Cross(v0ToV1, v0ToV2);
		}

		static Pixel2D CalcBoundingBoxMin(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2)
		{
			int xMin{ static_cast<int>(std::min(std::min(v0.position.x, v1.position.x), v2.position.x)) };
			int yMin{ static_cast<int>(std::min(std::min(v0.position.y, v1.position.y), v2.position.y)) };

			xMin = (xMin < 0 ? 0 : xMin);
			yMin = (yMin < 0 ? 0 : yMin);

			return Pixel2D(xMin, yMin);
		}

		static Pixel2D CalcBoundingBoxMax(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, int width, int height)
		{
			int xMax{ static_cast<int>(std::max(std::max(v0.position.x, v1.position.x), v2.position.x)) + 1 };
			int yMax{ static_cast<int>(std::max(std::max(v0.position.y, v1.position.y), v2.position.y)) + 1 };

			xMax = (xMax >= width ? width - 1 : xMax);
			yMax = (yMax >= height ? height - 1 : yMax);

			return Pixel2D(xMax, yMax);
		}
	}
}