#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

#include "directxmath.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle, float ar) :
			origin{ _origin },
			fovAngle{ _fovAngle },
			ar{ ar }
		{
		}


		Vector3 origin{};
		float fovAngle{ 90.f };
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };
		float ar;

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};

		Matrix projectionMatrix{};

		void Initialize(float _ar, float _fovAngle = 90.f, Vector3 _origin = { 0.f,0.f,0.f })
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;
			ar = _ar;
		}

		void CalculateViewMatrix()
		{
			//TODO W1
			//ONB => invViewMatrix
			right = { Vector3::Cross(Vector3::UnitY,forward).Normalized() };
			up = { Vector3::Cross(forward,right).Normalized() };

			invViewMatrix =
			{
				Vector4{right, 0},
				Vector4{up, 0},
				Vector4{forward, 0},
				Vector4{origin, 1}
			};
			//Inverse(ONB) => ViewMatrix

			viewMatrix = Matrix::Inverse(invViewMatrix);

			//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			//TODO W2

			projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, ar, 0.1f, 100.f);
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		Matrix GetViewProjMatrix() const
		{
			return viewMatrix * projectionMatrix;
		}

		void Update(const Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Camera Update Logic
			const float movementSpeed = 25.f * deltaTime;

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//todo: W2
			//assert(false && "Not Implemented Yet");

			//movement
			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += forward * movementSpeed;
			}
			else if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= forward * movementSpeed;
			}
			else if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= right * movementSpeed;
			}
			else if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += right * movementSpeed;
			}

			//rotation
			const float rotationSpeed{ 5.f * deltaTime };

			if ((mouseState & SDL_BUTTON_RMASK) != 0)
			{
				totalYaw += mouseX * rotationSpeed;
				totalPitch -= mouseY * rotationSpeed;

				CalculateViewMatrix();
			}

			//third movement
			if ((mouseState & SDL_BUTTON_LMASK) != 0 && (mouseState & SDL_BUTTON_RMASK) != 0)
			{
				origin += up * movementSpeed * mouseY;
			}
			//secondary movement/rotation
			else if ((mouseState & SDL_BUTTON_LMASK) != 0)
			{
				totalYaw += mouseX * rotationSpeed;
				const Vector3 movement = forward * static_cast<float>(-mouseY) * movementSpeed;

				origin += movement;

				CalculateViewMatrix();
			}

			Matrix finalRotationMat = Matrix::CreateRotation(totalPitch, totalYaw, 0);
			forward = finalRotationMat.TransformVector(Vector3::UnitZ);
			forward.Normalize();

			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}

		Matrix GetViewMatrix() const { return viewMatrix; };
		Matrix GetProjectionMatrix() const { return projectionMatrix; };
	};
}