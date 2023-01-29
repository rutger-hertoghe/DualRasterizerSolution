#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}

		Vector3 origin{};
		float fovAngle{90.f};
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};

		const int m_SpeedScalar{ 2 };
		const float m_MouseSensitivityScalar{ 0.025f };
		const float m_MovementSpeed{ 50.f };

		Matrix projectionMatrix{};
		float m_AspectRatio{};

		float nearPlane{ 0.1f };
		float farPlane{ 100.f };
		// Near, far, wherever you are
		// I believe that the heart does go on

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f}, float aspectRatio = 1.f)
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;

			m_AspectRatio = aspectRatio;
		}

		void CalculateViewMatrix()
		{
			//ONB => invViewMatrix
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();
			invViewMatrix = Matrix{
				Vector3{right},
				Vector3{up},
				Vector3{forward},
				Vector3{origin}
			};

			//Inverse(ONB) => ViewMatrix
			viewMatrix = invViewMatrix.Inverse();

			//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			projectionMatrix = { Vector4{ 1.f / (m_AspectRatio * fov), 0.f, 0.f, 0.f },
				Vector4{ 0.f, 1.f / fov, 0.f, 0.f },
				Vector4{ 0.f, 0.f, farPlane / (farPlane - nearPlane), 1.f},
				Vector4{ 0.f, 0.f, -(farPlane * nearPlane) / (farPlane - nearPlane), 0.f} };
			//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		const Matrix& GetViewMatrix()
		{
			return viewMatrix;
		}

		const Matrix& GetProjectionMatrix()
		{
			return projectionMatrix;
		}

		void Update(float deltaTime)
		{
			//Keyboard Input
			ProcessKeyBoardInput(deltaTime);

			//Mouse Input
			ProcessMouseInput(); // NO DELTATIME NEEDED! Relative mouse movement is always to previous frame and thus already accounts for the time!

			const Matrix finalRotation{ Matrix::CreateRotation(totalPitch, totalYaw, 0) };
			forward = finalRotation.TransformVector(Vector3::UnitZ);
			forward.Normalize();
			//...

			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}
	
	private:
		void ProcessKeyBoardInput(float deltaTime)
		{
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			if(pKeyboardState[SDL_SCANCODE_LSHIFT])
			{
				deltaTime *= m_SpeedScalar;
			}

			if (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP])
			{
				MoveOnForwardAxis(deltaTime);
			}
			if (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT])
			{
				MoveOnRightAxis(deltaTime);
			}
			if (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN])
			{
				MoveOnForwardAxis(-deltaTime);
			}
			if (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT])
			{
				MoveOnRightAxis(-deltaTime);
			}
		}

		void ProcessMouseInput()
		{
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			const auto leftMouseButtonDown{ mouseState & SDL_BUTTON(SDL_BUTTON_LEFT) };
			const auto rightMouseButtonDown{ mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT) };

			if (leftMouseButtonDown && rightMouseButtonDown)
			{
				if (mouseY != 0) MoveOnUpAxis(-static_cast<float>(mouseY) * m_MouseSensitivityScalar);
			}
			else if (leftMouseButtonDown)
			{
				if (mouseY != 0) MoveOnForwardAxis(-static_cast<float>(mouseY) * m_MouseSensitivityScalar);
				if (mouseX != 0) Yaw(static_cast<float>(mouseX) * m_MouseSensitivityScalar);
			}
			else if (rightMouseButtonDown)
			{
				if (mouseY != 0) Pitch(-static_cast<float>(mouseY) * m_MouseSensitivityScalar);
				if (mouseX != 0) Yaw(static_cast<float>(mouseX) * m_MouseSensitivityScalar);
			}
		}

		void MoveOnForwardAxis(float deltaTime)
		{
			origin += forward * deltaTime * m_MovementSpeed;
		}
		void MoveOnUpAxis(float deltaTime)
		{
			origin += Vector3::UnitY * deltaTime * m_MovementSpeed;
		}
		void MoveOnRightAxis(float deltaTime)
		{
			origin += right * deltaTime * m_MovementSpeed;
		}
		void Yaw(float deltaTime)
		{
			totalYaw += deltaTime;
		}
		void Pitch(float deltaTime)
		{
			totalPitch += deltaTime;
		}
	};
}
