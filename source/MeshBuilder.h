#pragma once
#include "Mesh.h"
#include "Structs.h"

using namespace dae;

namespace MeshBuilder
{
	inline Mesh<Vertex_PosCol>* CreateRectangularCuboid(ID3D11Device* pDevice, float width, float height, float depth)
	{
		std::vector<Vertex_PosCol> vertices{};

		vertices.push_back({ {width / 2, height / 2, depth / 2}, {}});		// 0:  W,  H,  D
		vertices.push_back({ {width / 2, -height / 2, depth / 2}, {}});		// 1:  W, -H,  D
		vertices.push_back({ {width / 2, height / 2, -depth / 2}, {}});		// 2:  W,  H, -D
		vertices.push_back({ {width / 2, -height / 2, -depth / 2}, {}});		// 3:  W, -H, -D
		vertices.push_back({ {-width / 2, height / 2, depth / 2}, {}});		// 4: -W,  H,  D
		vertices.push_back({ {-width / 2, -height / 2, depth / 2}, {}});		// 5: -W, -H,  D
		vertices.push_back({ {-width / 2, height / 2, -depth / 2}, {}});		// 6: -W,  H, -D
		vertices.push_back({ {-width / 2, -height / 2, -depth / 2}, {}});		// 7: -W, -H, -D

		std::vector<uint32_t> indices{
			0, 1, 4,	1, 5, 4,
			4, 5, 6,	5, 7, 6,
			2, 3, 7,	6, 3, 2,
			0, 2, 3,	3, 1, 0,
			0, 4, 6,	2, 0, 6,
			1, 3, 5,	7, 5, 3
		};

		return new Mesh(pDevice, vertices, indices);
	}
}