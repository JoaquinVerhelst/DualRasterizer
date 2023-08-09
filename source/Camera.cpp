#include "pch.h"
#include "Camera.h"



namespace dae
{


	Camera::Camera(float _fovAngle, Vector3 _origin, float _aspectRatio)
		:
		m_Origin{ _origin },
		m_FovAngle{ _fovAngle },
		m_AspectRatio{ _aspectRatio }
	{
		m_Fov = tanf((m_FovAngle * TO_RADIANS) / 2.f);
	}

	void Camera::CalculateViewMatrix()
	{
		const Matrix finalRotation = Matrix::CreateRotation({ m_TotalPitch, m_TotalYaw, 0.f });

		m_Forward = finalRotation.TransformVector(Vector3::UnitZ);
		m_Right = Vector3::Cross(Vector3::UnitY, m_Forward).Normalized();
		m_Up = Vector3::Cross(m_Forward, m_Right).Normalized();

		m_InvViewMatrix = { m_Right, m_Up, m_Forward, m_Origin };

		m_ViewMatrix = m_InvViewMatrix.Inverse();

		//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
		//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
	}

	void Camera::CalculateProjectionMatrix()
	{
		m_ProjectionMatrix = Matrix::CreatePerspectiveFovLH(m_Fov, m_AspectRatio, m_NearPlane, m_FarPlane);

		//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
	}

	Matrix Camera::GetViewMatrix() const
	{
		return m_ViewMatrix;
	}

	const Matrix& Camera::GetInverseViewMatrix() const
	{
		 return m_InvViewMatrix;
	}

	Matrix Camera::GetProjectionMatrix() const
	{
		return m_ProjectionMatrix;
	}

	Vector3 Camera::GetOrigin() const
	{
		return m_Origin;
	}

	void Camera::KeyboardInput(Vector3& origin, const float deltaTime)
	{
		//Keyboard Input

		const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

		if (pKeyboardState[SDL_SCANCODE_LSHIFT] == 1)
		{
			m_CurrentMovementSpeed *= 4;
		}


		if (pKeyboardState[SDL_SCANCODE_W] == 1)
			origin += m_CurrentMovementSpeed * m_Forward * deltaTime;
		if (pKeyboardState[SDL_SCANCODE_S] == 1)
			origin -= m_CurrentMovementSpeed * m_Forward * deltaTime;
		if (pKeyboardState[SDL_SCANCODE_A] == 1)
			origin -= m_Right * m_CurrentMovementSpeed * deltaTime;
		if (pKeyboardState[SDL_SCANCODE_D] == 1)
			origin += m_Right * m_CurrentMovementSpeed * deltaTime;
	}

	void Camera::MouseInput(Vector3& origin, const float deltaTime)
	{
		//Mouse Input

		int mouseX{}, mouseY{};
		const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);


		switch (mouseState)
		{
		case SDL_BUTTON_LMASK:

			origin -= m_Forward * (mouseY * m_CurrentMovementSpeed / 2 * deltaTime);

			m_TotalYaw += mouseX * m_RotationSpeed * deltaTime;
			break;
		case SDL_BUTTON_RMASK:

			m_TotalYaw += mouseX * m_RotationSpeed * deltaTime;
			m_TotalPitch -= mouseY * m_RotationSpeed * deltaTime;
			break;
		case SDL_BUTTON_X2:

			origin.y -= mouseY * m_CurrentMovementSpeed * deltaTime;
			break;

		}
	}

	void Camera::Update(const Timer* pTimer)
	{
		const float deltaTime = pTimer->GetElapsed();

		m_CurrentMovementSpeed = m_MovementSpeed ;

		Vector3 position{};

		KeyboardInput(position, deltaTime);

		MouseInput(position, deltaTime);

		m_Origin += position;

		Matrix finalRotation = Matrix::CreateRotationX(m_TotalPitch) * Matrix::CreateRotationY(m_TotalYaw);
		m_Forward = finalRotation.TransformVector(Vector3::UnitZ);
		m_Forward.Normalize();

		//Update Matrices
		CalculateViewMatrix();
		CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
	}


}