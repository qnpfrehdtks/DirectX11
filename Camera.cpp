
#include "DXUT.h"
#include "Camera.h"
#include "MathHelper.h"

cCamera::cCamera()
: m_Position(15.0f, 5.0f, 5.0f),
m_Right(1.0f, 0.0f, 0.0f),
m_Up(0.0f, 1.0f, 0.0f),
m_Forward(0.0f, 0.0f, 1.0f), m_Speed(25.0f),
m_ChaForward(0.0f, 0.0f, 1.0f)
{
	SetProjection(0.25f * MathHelper::Pi, 1.0f, 0.1f, 3000.0f);
}


cCamera::~cCamera()
{
}

void cCamera::InputKey(float dt)
{
	if (GetAsyncKeyState('W') & 0x8000)
	{
		cCamera::Walk(40.0f * dt);
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		cCamera::Walk(-40.0f * dt);
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		cCamera::Strafe(-40.0f * dt);
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		cCamera::Strafe(40.0f * dt);
	}
}

void cCamera::InputKey(float dt, XMFLOAT3 dir)
{
	if (GetAsyncKeyState('W') & 0x8000)
	{
		Walk(40.0f * dt, dir);
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		Walk(-40.0f * dt, dir);
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		Strafe(-40.0f * dt, dir);
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		Strafe(40.0f * dt, dir);
	}
}

void cCamera::InputMouse(float dx, float dy)
{
	pitch(dy);
	RotateY(dx);
}


void cCamera::SetProjection(float fovY, float aspect, float nearZ, float farZ)
{
	// 수직 시야 각도
	m_FovY = fovY;

	// 가까운 평면까지의 길이,
	m_NearZ = nearZ;
	// 먼 평면 까지의 거리
	m_FarZ = farZ;
	m_Aspect = aspect;

	// 먼 평면의 높이 구하는 공식,
	m_NearWndHeight = 2.0f * m_NearZ * tanf(0.5* m_FovY);
	m_FarWndHeight = 2.0f * m_FarZ * tanf(0.5* m_FovY);

	// 투영 행렬을 계산해서 넣자.
	XMMATRIX p = XMMatrixPerspectiveFovLH(m_FovY, m_Aspect, m_NearZ, m_FarZ);

	XMStoreFloat4x4(&m_ProjMat, p);
	m_InvProj = XMMatrixInverse(NULL, p);

}

void cCamera::SetForward(float d)
{
}

void cCamera::LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp)
{
	XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
	XMVECTOR U = XMVector3Cross(L, R);

	XMStoreFloat3(&m_Position, pos);
	XMStoreFloat3(&m_Forward, L);
	XMStoreFloat3(&m_Right, R);
	XMStoreFloat3(&m_Up, U);
}

void cCamera::LookAt(const XMFLOAT3 & pos, const XMFLOAT3 & target, const XMFLOAT3 & up)
{
	XMVECTOR P = XMLoadFloat3(&pos);
	XMVECTOR T = XMLoadFloat3(&target);
	XMVECTOR U = XMLoadFloat3(&up);

	LookAt(P, T, U);
}

void cCamera::Walk(float d)
{
	// Floating Point -> Vector ,,, WOW
	XMVECTOR distance = XMVectorReplicate(d);
	XMVECTOR forward = XMLoadFloat3(&m_Forward);
	// Float3 -> Vector Transform Function :D
	XMVECTOR pos = XMLoadFloat3(&m_Position);

	// XMVectorMultiplyAdd = (Distance Vector * Forward Vector) + Positon ;
	XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(distance, forward, pos));

}

void cCamera::Strafe(float d)
{
	// Floating Point -> Vector ,,, WOW
	XMVECTOR distance = XMVectorReplicate(d);
	XMVECTOR right = XMLoadFloat3(&m_Right);
	// Float3 -> Vector Transform Function :D
	XMVECTOR pos = XMLoadFloat3(&m_Position);

	// XMVectorMultiplyAdd = (Distance Vector * Right Vector) + Positon ;
	XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(distance, right, pos));
}

void cCamera::Walk(float d, XMFLOAT3 dir)
{
	// Floating Point -> Vector ,,, WOW
	XMVECTOR distance = XMVectorReplicate(d);
	XMVECTOR forward = XMLoadFloat3(&dir);
	// Float3 -> Vector Transform Function :D
	XMVECTOR pos = XMLoadFloat3(&m_Position);

	forward = XMVector3Normalize(forward);
	// XMVectorMultiplyAdd = (Distance Vector * Forward Vector) + Positon ;
	XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(distance, forward, pos));
}

void cCamera::Strafe(float d, XMFLOAT3 dir)
{
	// Floating Point -> Vector ,,, WOW
	XMVECTOR distance = XMVectorReplicate(d);
	XMVECTOR right = XMLoadFloat3(&dir);
	// Float3 -> Vector Transform Function :D
	XMVECTOR pos = XMLoadFloat3(&m_Position);
	right = XMVector3Normalize(right);

	// XMVectorMultiplyAdd = (Distance Vector * Right Vector) + Positon ;
	XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(distance, right, pos));
}

// Pitch 축으로 카메라를 회전하는 경우.
void cCamera::pitch(float angle)
{
	// Yaw만 움직일테니 Right는 움직일 필요가 없을 것.
	// Right 축을 기준으로 angle 만큼 회저한 회전 행렬을 구해BoaYo~
	XMMATRIX rotMat = XMMatrixRotationAxis(XMLoadFloat3(&m_Right), angle);
	// Up Vector랑 회전 행렬을 곱해서 카메라 Up Vector Update 한다.
	XMStoreFloat3(&m_Up, XMVector3TransformNormal(XMLoadFloat3(&m_Up), rotMat));
	XMStoreFloat3(&m_Forward, XMVector3TransformNormal(XMLoadFloat3(&m_Forward), rotMat));


}

void cCamera::RotateY(float angle)
{
	// Up 축을 기준으로 angle 만큼 회전한 회전 행렬을 구해BoaYo~
	XMMATRIX rotMat = XMMatrixRotationY(angle);
	// Up Vector랑 회전 행렬을 곱해서 카메라 Up Vector Update 한다.

	// XMVector3TransformNormal Vector * Vector = Vector( Not Pt )
	XMStoreFloat3(&m_Up, XMVector3TransformNormal(XMLoadFloat3(&m_Up), rotMat));
	XMStoreFloat3(&m_Forward, XMVector3TransformNormal(XMLoadFloat3(&m_Forward), rotMat));
	XMStoreFloat3(&m_ChaForward, XMVector3TransformNormal(XMLoadFloat3(&m_ChaForward), rotMat));
	XMStoreFloat3(&m_Right, XMVector3TransformNormal(XMLoadFloat3(&m_Right), rotMat));
}


// Up, Forward, Right Vector 등을 정규 직교화 시킨다.
// 회전이나 Transform이 진행되다 보면 그 Basis 벡터들이 더이상 정규 직교화가 아닌 경우도 존재하기 때문에, 이런 method를 진행함.
void cCamera::UpdateViewMatrix()
{
	// 각 축 Vecotr르 긁어 오자.
	XMVECTOR right = XMLoadFloat3(&m_Right);
	XMVECTOR up = XMLoadFloat3(&m_Up);
	XMVECTOR forward = XMLoadFloat3(&m_Forward);
	XMVECTOR pos = XMLoadFloat3(&m_Position);

	// 벡터, Right, Up, Forward Orthogonal GOGO~!

	// Forwrad Vector 정규화
	forward = XMVector3Normalize(forward);
	// Up 벡터 구하기 위해 forward와 right 축 벡터 외적해서 up Vecotr 방향을 다시 구함.
	up = XMVector3Normalize(XMVector3Cross(forward, right));
	//up = XMVector4Normalize(up);
	// up 과 Forward가 직교인 시점에서 Right 둘을 외적해서  right Vector 구한다.
	right = XMVector3Cross(up, forward);
	//right = XMVector4Normalize(right);
	



	// Vector를 Float3으로 배정
	XMStoreFloat3(&m_Right, right);
	XMStoreFloat3(&m_Up, up);
	XMStoreFloat3(&m_Forward, forward);


	// T^-1 * R^-1 = View 행렬을 만들기 위한 공식,
	// View 좌표계 변환 행렬
	//  Ux   Vx   Wx  0
	//  Uy   Vy   Wy  0
	//  Uz   Uy   Wz  0 
	// -P.U -P.V -P.W 1  ( 카메라의 위치와 각 Basis 벡터 내적)


	// View 행렬 초기화 해줌.
	//앞에서 구한 forward, right, up등을 카메라의 좌표계를 구성하는 Basis Vector로 삼고 행렬에 입력해준다.

	float Qx = -XMVectorGetX(XMVector3Dot(pos, right));
	float Qy = -XMVectorGetX(XMVector3Dot(pos, up));
	float Qz = -XMVectorGetX(XMVector3Dot(pos, forward));


	m_ViewMat(0, 0) = m_Right.x;
	m_ViewMat(1, 0) = m_Right.y;
	m_ViewMat(2, 0) = m_Right.z;
	m_ViewMat(3, 0) = Qx;

	m_ViewMat(0, 1) = m_Up.x;
	m_ViewMat(1, 1) = m_Up.y;
	m_ViewMat(2, 1) = m_Up.z;
	m_ViewMat(3, 1) = Qy;

	m_ViewMat(0, 2) = m_Forward.x;
	m_ViewMat(1, 2) = m_Forward.y;
	m_ViewMat(2, 2) = m_Forward.z;
	m_ViewMat(3, 2) = Qz;

	m_ViewMat(0, 3) = 0.0f;
	m_ViewMat(1, 3) = 0.0f;
	m_ViewMat(2, 3) = 0.0f;
	m_ViewMat(3, 3) = 1.0f;



}

float cCamera::GetFovX() const
{
	return 2.0f * atan((GetNearWndWidth() * 0.5f) / m_NearZ);
}



