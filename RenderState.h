//***************************************************************************************
// RenderStates.h by Frank Luna (C) 2011 All Rights Reserved.
//   
// Defines render state objects.  
//***************************************************************************************

#ifndef RENDERSTATES_H
#define RENDERSTATES_H

#include <DXUT.h>

class RenderStates
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static ID3D11RasterizerState* WireframeRS;
	static ID3D11RasterizerState* NoCullRS;
	static ID3D11RasterizerState* FrontCullRS;
	static ID3D11RasterizerState* CullClockwiseRS;

	static ID3D11BlendState* AlphaToCoverageBS;
	static ID3D11BlendState* TransparentBS;
	static ID3D11BlendState* MirrorBS;
	static ID3D11BlendState* NoRenderTargetWritesBS;
	static ID3D11BlendState* DefaultRenderTargetBS;

	static ID3D11DepthStencilState* MirrorDSS;
	static ID3D11DepthStencilState* NoDoubleDSS;
	static ID3D11DepthStencilState* ReflectionDSS;
	static ID3D11DepthStencilState* EqualDSS;
	static ID3D11DepthStencilState* GreaterDSS;
	static ID3D11DepthStencilState* LessDSS;
};

#endif // RENDERSTATES_H