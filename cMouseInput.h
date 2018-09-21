#pragma once



class cCamera;

class cMouseInput
{

private:
	cCamera* m_pCam;
	POINT m_LastMousePos;


public:
	cMouseInput();
	~cMouseInput();

	virtual void OnMouseDown(WPARAM btnState, int x, int y);
	virtual void OnMouseUp(WPARAM btnState, int x, int y);
	virtual void OnMouseMove(WPARAM btnState, int x, int y);

	POINT GetMousePt() { return m_LastMousePos; }

	void SetCamera(cCamera* cam) { m_pCam = cam; }

};



