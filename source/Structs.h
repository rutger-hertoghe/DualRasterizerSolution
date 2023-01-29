#pragma once
#include "Vector3.h"
#include "Vector4.h"
namespace dae
{
	struct Pixel2D
	{
		int x{};
		int y{};
	};

	struct Vertex_PosTex
	{
		Vector3 position{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
	};

	struct Vertex_Out
	{
		Vector4 position{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector3 viewDirection{};
	};

	struct Vertex_PosCol
	{
		Vector3 position{};
		Vector3 color{};
	};
}