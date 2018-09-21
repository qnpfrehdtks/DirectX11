#pragma once
#include "Main.h"
#include <d3d11.h>
#include <DirectXCollision.h>

using namespace DirectX;

class Picking
{
	BoundingBox m_BBox;
	

public:
	Picking();
	~Picking();
};

