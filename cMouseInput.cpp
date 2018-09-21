#include "DXUT.h"
#include "cMouseInput.h"
#include "Camera.h"
#include "Main.h"


cMouseInput::cMouseInput() : m_pCam(nullptr)
{
}

cMouseInput::~cMouseInput()
{
}

void cMouseInput::OnMouseDown(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		m_LastMousePos.x = x;
		m_LastMousePos.y = y;

		SetCapture(D3DMain::GetInstance()->GetHWND());
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		m_LastMousePos.x = x;
		m_LastMousePos.y = y;

		SetCapture(D3DMain::GetInstance()->GetHWND());
	}

}

void cMouseInput::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void cMouseInput::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - m_LastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - m_LastMousePos.y));

		m_pCam->InputMouse(dx, dy);

		m_pCam->UpdateViewMatrix();

	}
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
}
