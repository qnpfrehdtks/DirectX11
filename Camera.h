#pragma once
#include <Windows.h>

#include <DirectXMath.h>


#ifndef CAMERA_H
#define CAMERA_H

using namespace DirectX;

class cCamera
{

private:
	float m_Speed;
	DirectX::XMFLOAT3 m_Position; // �þ� ���� ����
	DirectX::XMFLOAT3 m_Right; // �þ� ���� X��
	DirectX::XMFLOAT3 m_Up; // �þ� Y��,
	DirectX::XMFLOAT3 m_Forward; // �þ� z��
	DirectX::XMFLOAT3 m_ChaForward;

	//����ü ���� ������
	float m_NearZ;  // ���� ����� ��� ������ ����
	float m_FarZ;  // ���� �� ��� ������ ����

	float m_Aspect;  // ��Ⱦ�� // Width / Height
	float m_FovY;    // ���� �þ߰�
	float m_NearWndHeight;  // ����� ����� ����
	float m_FarWndHeight;   // �� ����� ����

	DirectX::XMMATRIX m_InvProj;
	DirectX::XMFLOAT4X4 m_ProjMat;
	DirectX::XMFLOAT4X4 m_ViewMat;
	DirectX::XMFLOAT4X4 m_VP;
public:

	cCamera();
	virtual ~cCamera();

	// Camera Input functions
	void InputKey(float dt);
	void InputKey(float dt, XMFLOAT3 dir);
	void InputMouse(float dx, float dy);

	void SetProjection(float fovY, float aspect, float nearZ, float farZ);
	void SetForward(float d);
	

	// Define camera space via LookAt parameters.
	void LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp);
	void LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up);

	void Walk(float d);
	void Strafe(float d);

	void Walk(float d, XMFLOAT3 dir);
	void Strafe(float d, XMFLOAT3 dir);

	void pitch(float angle);
	void RotateY(float angle);

	void UpdateViewMatrix();



	void SetMoveForward(float x, float y, float z) { m_ChaForward = XMFLOAT3(x, y, z); }
	void SetMoveForward(XMFLOAT3& pos) { m_ChaForward = pos; }

	void SetPos(float x, float y, float z) { m_Position = XMFLOAT3(x, y, z); }
	void SetPos(XMFLOAT3& pos) { m_Position = pos; }

	void SetAspect(float width, float height) { m_Aspect = width / height; }



	// ����ü ���� ������ �Լ�
	float GetFovX() const; // ���� �þ߰��� ��� �Լ�.
	float GetFovY() const { return m_FovY; } // ���� �þ߰��� ��� �Լ�

	float GetNearWndHeight() const { return m_NearWndHeight; }
	float GetFarWndHeight() const { return m_FarWndHeight; }

	float GetNearWndWidth() const { return m_Aspect * m_NearWndHeight; }
	float GetFarWndWidth() const { return m_Aspect * m_FarWndHeight; }

	// Get Functions

	XMFLOAT3 GetPos() const { return m_Position; }
	XMVECTOR GetPosXM() const { return XMLoadFloat3(&m_Position); }

	XMFLOAT3 GetUp() const { return m_Up; }
	XMVECTOR GetUpXM() const { return XMLoadFloat3(&m_Up); }

	XMFLOAT3 GetRight() const { return m_Right; }
	XMVECTOR GetRightXM() const { return XMLoadFloat3(&m_Right); }

	XMFLOAT3 GetForward() const { return m_Forward; }
	XMVECTOR GetForwardFX() const { return XMLoadFloat3(&m_Forward); };


	XMFLOAT3 GetMoveForward() { return m_ChaForward; }
	XMVECTOR GetMoveForwardFX() const { return XMLoadFloat3(&m_ChaForward); }

	CXMMATRIX GetInvProjMat() const { return m_InvProj; }
	CXMMATRIX GetProjMat() const { return XMLoadFloat4x4(&m_ProjMat); }
	CXMMATRIX GetViewMat() const { return XMLoadFloat4x4(&m_ViewMat); }
	CXMMATRIX GetVP() const { return XMLoadFloat4x4(&m_VP); }

	float GetProjA() const { return m_ProjMat._33; }
	float GetProjB() const { return m_ProjMat._43; }

	//CXMMATRIX GetViewProjMat() const { return XMMatrixMultiply(GetViewMat(), GetProjMat()); }


	float GetNearZ() const { return m_NearZ; }
	float GetFarZ()const { return m_FarZ; }

	float GetAspect() const { return m_Aspect; }

	void SetVP(CXMMATRIX vp) { XMStoreFloat4x4(&m_VP,vp); }
};

#endif


