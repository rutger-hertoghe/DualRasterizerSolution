#pragma once
#include <cassert>
#include "Structs.h"
#include "Effect_PosTex.h"
#include "Effect_PosCol.h"
#include "Effect_PartCov.h"
#include "Utils.h"
#include "Camera.h"

namespace dae {
	enum class PrimitiveTopology
	{
		TriangleList,
		TriangleStrip
	};

	template<typename T_Vertex>
	class Mesh
	{
	public:
		// Dual Rasterizer
		explicit Mesh(ID3D11Device* pDevice, const std::string& objFilePath, EffectType fxType);
		explicit Mesh(ID3D11Device* pDevice, std::vector<T_Vertex> vertices, std::vector<uint32_t> indices);
		~Mesh();

		Mesh(const Mesh& other) = delete;
		Mesh operator=(const Mesh& other) = delete;
		Mesh(Mesh&& other) = delete;
		Mesh operator=(Mesh&& other) = delete;

		void SetWorldMatrix(const Matrix& worldMatrix);
		const Matrix& GetWorldMatrix() const;
		void UpdateRotation(float deltaTime);

		// Hardware Rasterizer
		void HardwareRender(ID3D11DeviceContext* pDeviceContext, ID3D11Device* pDevice);
		void UpdateEffectMatrices(Camera* pCamera);
		Effect* GetEffect();

		// Software Rasterizer
		//void VerticesToProjectionSpace(const Matrix& viewMatrix, const Matrix& projectionMatrix, const Vector3& cameraPos);
		std::vector<Vertex_Out>& GetVertexOutVector();
		uint32_t GetNumIndices();
		std::vector<uint32_t> GetIndices();
		
	private:
		// Dual Rasterizer
		Matrix m_WorldMatrix;
		std::vector<T_Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;

		// Hardware Rasterizer
		uint32_t m_NumIndices;
		Effect* m_pEffect;
		ID3D11Buffer* m_pVertexBuffer;
		ID3D11Buffer* m_pIndexBuffer;

		void InitializeEffect(ID3D11Device* pDevice, EffectType fxType);
		void InitializeBuffer(ID3D11Device* pDevice);

		// Software Rasterizer
		PrimitiveTopology m_pPrimitiveTopology{ PrimitiveTopology::TriangleList };
		std::vector<Vertex_Out> m_Vertices_out{};
	};



	//--------------------------
	// Template definitions
	//--------------------------

	//--------------------------------------
	// DUAL RASTERIZER
	//--------------------------------------
#pragma region DUAL_RASTERIZER
	template<typename T_Vertex>
	Mesh<T_Vertex>::Mesh(ID3D11Device* pDevice, const std::string& objFilePath, EffectType fxType)
		: m_Vertices{}
		, m_Indices{}
		, m_pIndexBuffer(nullptr)
		, m_pVertexBuffer(nullptr)
		, m_pEffect(nullptr)
		, m_WorldMatrix()
	{
		Utils::ParseOBJ(objFilePath, m_Vertices, m_Indices);

		InitializeEffect(pDevice, fxType);

		InitializeBuffer(pDevice);
	}

	template<typename T_Vertex>
	Mesh<T_Vertex>::Mesh(ID3D11Device* pDevice, std::vector<T_Vertex> vertices, std::vector<uint32_t> indices)
	{
		m_Vertices = vertices;
		m_Indices = indices;

		InitializeEffect(pDevice, EffectType::PosCol);
		InitializeBuffer(pDevice);
	}

	template<typename T_Vertex>
	Mesh<T_Vertex>::~Mesh()
	{
		SAFE_DELETE(m_pEffect)
		if(m_pVertexBuffer) m_pVertexBuffer->Release();
		if(m_pIndexBuffer) m_pIndexBuffer->Release();
	}

	template<typename T_Vertex>
	void Mesh<T_Vertex>::SetWorldMatrix(const Matrix& worldMatrix)
	{
		m_WorldMatrix = worldMatrix;
	}

	template<typename T_Vertex>
	const Matrix& Mesh<T_Vertex>::GetWorldMatrix() const
	{
		return m_WorldMatrix;
	}

	template<typename  T_Vertex>
	void Mesh<T_Vertex>::UpdateRotation(float deltaTime)
	{
		m_WorldMatrix = Matrix::CreateRotationY(deltaTime * PI_DIV_4) * m_WorldMatrix;
	}
#pragma endregion

	//--------------------------------------
	// HARDWARE RASTERIZER
	//--------------------------------------
#pragma region HARDWARE_RASTERIZER

	template<typename T_Vertex>
	void Mesh<T_Vertex>::HardwareRender(ID3D11DeviceContext* pDeviceContext, ID3D11Device* pDevice)
	{
		// Set Primitive Topology
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // What does my topology look like?

		// Set InputLayout
		pDeviceContext->IASetInputLayout(m_pEffect->GetInputLayout()); // What is a single vertex gonna look like?
		
		// Set Vertex Buffer
		UINT stride{};
		constexpr auto posTexSize{ sizeof(Vertex_PosTex) };
		constexpr auto posColSize{ sizeof(Vertex_PosCol) };
		switch (m_pEffect->GetFxType())
		{
		case EffectType::PosCol:
			stride = posColSize;
			break;
		case EffectType::PartCov:
		case EffectType::PosTex:
			stride = posTexSize;
			break;
		}
		//constexpr UINT stride{ sizeof(Vertex_PosCol) };
		constexpr UINT offset{ 0 };
		pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset); // Pass along vertices

		// SetIndexBuffer
		pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0); // Pass along vertex indices (what vertices belong together to form triangles)

		// Draw
		D3DX11_TECHNIQUE_DESC techDesc{};
		m_pEffect->GetTechnique()->GetDesc(&techDesc);
		for (UINT p{ 0 }; p < techDesc.Passes; ++p)
		{
			m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
			pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
		}
	}

	template<typename T_Vertex>
	void Mesh<T_Vertex>::UpdateEffectMatrices(Camera* pCamera)
	{
		m_pEffect->SetWorldMatrix(m_WorldMatrix);
		m_pEffect->SetInvViewMatrix(pCamera->invViewMatrix);
		m_pEffect->SetWorldViewProjMatrix(m_WorldMatrix * pCamera->viewMatrix * pCamera->projectionMatrix);
	}

	template<typename T_Vertex>
	Effect* Mesh<T_Vertex>::GetEffect()
	{
		return m_pEffect;
	}

	template<typename T_Vertex>
	void Mesh<T_Vertex>::InitializeEffect(ID3D11Device* pDevice, EffectType fxType)
	{
		switch (fxType)
		{
		case EffectType::PosTex:
			m_pEffect = new Effect_PosTex{ pDevice };
			break;
		case EffectType::PosCol:
			m_pEffect = new Effect_PosCol{ pDevice };
			break;
		case EffectType::PartCov:
			m_pEffect = new Effect_PartCov{ pDevice };
			break;
		}
	}

	template<typename T_Vertex>
	void Mesh<T_Vertex>::InitializeBuffer(ID3D11Device* pDevice)
	{
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(T_Vertex) * static_cast<uint32_t>(m_Vertices.size());
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = m_Vertices.data();

		HRESULT result{ pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer) };
		if (FAILED(result))
		{
			assert(false);
		}

		m_NumIndices = static_cast<uint32_t>(m_Indices.size());
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		initData.pSysMem = m_Indices.data();

		result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
		if (FAILED(result))
		{
			assert(false);
		}
	}

#pragma endregion


	//--------------------------------------
	// SOFTWARE RASTERIZER
	//--------------------------------------
#pragma region SOFTWARE_RASTERIZER
	//template<typename T_Vertex>
	//void Mesh<T_Vertex>::VerticesToProjectionSpace(const Matrix& viewMatrix, const Matrix& projectionMatrix, const Vector3& cameraPos)
	//{
	//	const Matrix wvpMatrix{ m_WorldMatrix * viewMatrix * projectionMatrix };
	//	m_Vertices_out.clear();
	//
	//	for(const auto& vertex : m_Vertices)
	//	{
	//		Vertex_Out outVertex{};
	//		outVertex.position.x = vertex.position.x;
	//		outVertex.position.y = vertex.position.y;
	//		outVertex.position.z = vertex.position.z;
	//		outVertex.position.w = 1.f;
	//
	//		outVertex.normal = vertex.normal;
	//		outVertex.tangent = vertex.tangent;
	//		outVertex.uv = vertex.uv;
	//
	//		// VIEW DIRECTION
	//		outVertex.viewDirection = wvpMatrix.TransformPoint(vertex.position) - cameraPos;
	//
	//		// VERTICES
	//		// Projection Space
	//		outVertex.position = wvpMatrix.TransformPoint(outVertex.position);
	//
	//		// Perspective Divide
	//		outVertex.position.x /= outVertex.position.w;
	//		outVertex.position.y /= outVertex.position.w;
	//		outVertex.position.z /= outVertex.position.w;
	//
	//		// NORMALS
	//		outVertex.normal = m_WorldMatrix.TransformVector(outVertex.normal);
	//		outVertex.normal.Normalize();
	//		outVertex.tangent = m_WorldMatrix.TransformVector(outVertex.tangent);
	//		outVertex.tangent.Normalize();
	//
	//		m_Vertices_out.emplace_back(outVertex);
	//	}
	//}

	template<typename T_Vertex>
	std::vector<Vertex_Out>& Mesh<T_Vertex>::GetVertexOutVector()
	{
		return m_Vertices_out;
	}

	template<typename T_Vertex>
	uint32_t dae::Mesh<T_Vertex>::GetNumIndices()
	{
		return m_NumIndices;
	}

	template<typename T_Vertex>
	std::vector<uint32_t> Mesh<T_Vertex>::GetIndices()
	{
		return m_Indices;
	}
#pragma endregion
}

