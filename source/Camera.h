#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	class Camera final
	{
	public:

		Camera(float _fovAngle = 90.f, Vector3 _origin = { 0.f,0.f,0.f }, float _aspecRatio = 1.f);

		void Update(const Timer* pTimer);

		void CalculateViewMatrix();
		void CalculateProjectionMatrix();


		Matrix GetViewMatrix() const;
		const Matrix& GetInverseViewMatrix() const;
		Matrix GetProjectionMatrix() const;

		Vector3 GetOrigin() const;

	private:

		Vector3 m_Origin{};
		float m_FovAngle{ 90.f };
		float m_Fov{ tanf((m_FovAngle * TO_RADIANS) / 2.f) };

		Vector3 m_Forward{ Vector3::UnitZ };
		Vector3 m_Up{ Vector3::UnitY };
		Vector3 m_Right{ Vector3::UnitX };

		float m_TotalPitch{};
		float m_TotalYaw{};

		Matrix m_InvViewMatrix{};
		Matrix m_ViewMatrix{};

		Matrix m_ProjectionMatrix{};

		float m_MovementSpeed{ 4.f };
		float m_CurrentMovementSpeed{};
		float m_RotationSpeed{ 10.f * TO_RADIANS};

		float m_NearPlane{ 0.1f };
		float m_FarPlane{ 100.f };
		float m_AspectRatio{};


		void KeyboardInput(Vector3& origin, const float deltaTime);
		void MouseInput(Vector3& origin, const float deltaTime);


	

	};
}

