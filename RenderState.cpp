//***************************************************************************************
// RenderStates.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "DXUT.h"
#include "RenderState.h"

ID3D11RasterizerState* RenderStates::WireframeRS = 0;
ID3D11RasterizerState* RenderStates::NoCullRS = 0;
ID3D11RasterizerState* RenderStates::CullClockwiseRS = 0;
ID3D11RasterizerState* RenderStates::FrontCullRS = 0;

ID3D11BlendState*      RenderStates::AlphaToCoverageBS = 0;
ID3D11BlendState*      RenderStates::TransparentBS = 0;
ID3D11BlendState*      RenderStates::MirrorBS = 0;
ID3D11BlendState*      RenderStates::NoRenderTargetWritesBS = 0;
ID3D11BlendState*      RenderStates::DefaultRenderTargetBS = 0;

ID3D11DepthStencilState* RenderStates::MirrorDSS = 0;
ID3D11DepthStencilState* RenderStates::ReflectionDSS = 0;
ID3D11DepthStencilState* RenderStates::NoDoubleDSS = 0;
ID3D11DepthStencilState* RenderStates::EqualDSS = 0;
ID3D11DepthStencilState* RenderStates::GreaterDSS = 0;
ID3D11DepthStencilState* RenderStates::LessDSS = 0;

void RenderStates::InitAll(ID3D11Device* device)
{
	//
	// WireframeRS
	//
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&wireframeDesc, &WireframeRS));

	//
	// NoCullRS
	//
	// 효과 fx에서 이미 알파가 0.1이하인 부분을 제거해서 렌더링 하고 있다.
	// 후면 선별을 비활성화하여, 그린다.
	D3D11_RASTERIZER_DESC noCullDesc;
	ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
	noCullDesc.FillMode = D3D11_FILL_SOLID;
	noCullDesc.CullMode = D3D11_CULL_NONE;
	noCullDesc.FrontCounterClockwise = false;
	noCullDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&noCullDesc, &NoCullRS));


	//
	// CullClockwiseRS
	//

	// Note: Define such that we still cull backfaces by making front faces CCW.
	// If we did not cull backfaces, then we have to worry about the BackFace
	// property in the D3D11_DEPTH_STENCIL_DESC.
	D3D11_RASTERIZER_DESC cullClockwiseDesc;
	ZeroMemory(&cullClockwiseDesc, sizeof(D3D11_RASTERIZER_DESC));
	cullClockwiseDesc.FillMode = D3D11_FILL_SOLID;
	cullClockwiseDesc.CullMode = D3D11_CULL_BACK;
	cullClockwiseDesc.FrontCounterClockwise = true;
	cullClockwiseDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&cullClockwiseDesc, &CullClockwiseRS));


	//
	// CullClockwiseRS
	//

	// Note: Define such that we still cull backfaces by making front faces CCW.
	// If we did not cull backfaces, then we have to worry about the BackFace
	// property in the D3D11_DEPTH_STENCIL_DESC.
	D3D11_RASTERIZER_DESC cullFrontDesc;
	ZeroMemory(&cullFrontDesc, sizeof(D3D11_RASTERIZER_DESC));
	cullFrontDesc.FillMode = D3D11_FILL_SOLID;
	cullFrontDesc.CullMode = D3D11_CULL_FRONT;
	cullFrontDesc.FrontCounterClockwise = false;
	cullFrontDesc.DepthClipEnable = false;

	HR(device->CreateRasterizerState(&cullFrontDesc, &FrontCullRS));


	//
	// AlphaToCoverageBS
	//

	D3D11_BLEND_DESC alphaToCoverageDesc = { 0 };
	alphaToCoverageDesc.AlphaToCoverageEnable = true;
	alphaToCoverageDesc.IndependentBlendEnable = false; // 혼합을 사용할 것인가?
	alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	// 렌더 대상 쓰기 마스크로, 다음 플래그들을 하나 이상 조합해서 지정할 수 있다.
	// 이는 혼합의 결과를 후면 버퍼의 어떤 색상 채널들에 기록할 것인가를 결정한다.

	HR(device->CreateBlendState(&alphaToCoverageDesc, &AlphaToCoverageBS));
	// 이제 이 BlendState 객체는 출력 병합기 단계에 묶여야 한다. 이를 위한 메소드는 나중에....


	// 해당 타겟을 그리지 않겠다는 설정으로 쓸 경우. 이 Blend를 사용한다.
	D3D11_BLEND_DESC noRenderTargetWritesDesc = { 0 };
	noRenderTargetWritesDesc.AlphaToCoverageEnable = false;
	noRenderTargetWritesDesc.IndependentBlendEnable = false;

	noRenderTargetWritesDesc.RenderTarget[0].BlendEnable = false;
	noRenderTargetWritesDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	noRenderTargetWritesDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	noRenderTargetWritesDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	noRenderTargetWritesDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	noRenderTargetWritesDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	noRenderTargetWritesDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	noRenderTargetWritesDesc.RenderTarget[0].RenderTargetWriteMask = 0;

	HR(device->CreateBlendState(&noRenderTargetWritesDesc, &NoRenderTargetWritesBS));


	//
	// TransparentBS
	//
	// 혼합 상태를 하나 생성하고 설정
	// C = Csrc X Fscr + Cdst X Fdst
	// 여기서 a는 Source 의 알파값.
	// C = Csrc X (a,a,a) + Cdst X (1 - a, 1 - a, 1 - a)
	// C = asCdst + (1 - as)Csrc
	// a이  0으로 갈수록 불투명, 1로 갈수록 완전 투명.

	// 투명 물체를 그릴 때 규칙.
	// Blending을 쓰지 않는 물체를 우선적으로 그릴것.
	// 그런 다음, Blending을 사용하는 물체들을 카메라와의 거리를 기준으로 정렬.
	// 그런 다음, 카메라에서 먼 것부터 그려 나간다. 
	//-> 이유? 가끔 물체가 투명하다면 물체 뒤에 있는 장면이 물체를 통해 보여야 하기 때문.
	// 그래서 후면 버퍼에는 투명한 물체 뒤에 있는 물체들이 미리 그려져야 한다. 그래야 원본 픽셀과 그 장면 대ㅐ상 픽셀과 혼합 가능.
	// 

	D3D11_BLEND_DESC transparentDesc = { 0 };
	transparentDesc.AlphaToCoverageEnable = false;     // true로 설정하면 식물이나 창살문 텍스처의 렌더링에 
													   // 유용한 다중 표본화 기법인 알파 포괄도 변환이 활성화 한다.
	transparentDesc.IndependentBlendEnable = false;     // true로 하면 혼합을 개별적으로 수행할 수 있다.

	transparentDesc.RenderTarget[0].BlendEnable = true;        // D3D11_RENDER_TARGET_BLEND_DESC 원소 여덟개짜리, 배열로 이 배열의  i번째 원소는
															   // 다중 렌더 대상의  i번째 렌더 대상에 작ㅇㅇ할 혼합 설정을 담은 구조체이다.

	transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;      // 소스 RGB 픽셀의 혼합 계수.
	transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // 대상 RGB 픽셀의 혼합 계수.
	transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;          // 혼합 연산 ADD

	transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;       // 소스 픽셀의 Alpha 혼합 계수.
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;     // 대상 픽셀의 Alpha 혼합 계수.
	transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;     // 혼합 연산 더하기.


	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(device->CreateBlendState(&transparentDesc, &TransparentBS));

	//
	// DrawDefaultRenderTargetDSS
	//
	// Create the additive blend state
	D3D11_BLEND_DESC descBlend;
	descBlend.AlphaToCoverageEnable = FALSE;
	descBlend.IndependentBlendEnable = FALSE;
	const D3D11_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
	{
		TRUE,
		D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD,
		D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD,
		D3D11_COLOR_WRITE_ENABLE_ALL,
	};
	for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		descBlend.RenderTarget[i] = defaultRenderTargetBlendDesc;
	HR(device->CreateBlendState(&descBlend, &DefaultRenderTargetBS));

	
	//
	// DrawReflectionDSS
	//

	/// 스텐실 판정 조건문
	// StencilRef & ReadMask 조건문 Value & ReadMask 가 참이면 스텐실 버퍼에 픽셀을 허용하고, 거짓이면 픽셀 기각.

	D3D11_DEPTH_STENCIL_DESC drawReflectionDesc;
	drawReflectionDesc.DepthEnable = true;
	drawReflectionDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // 깊이 버퍼 쓰기에도 활성화된다.
	drawReflectionDesc.DepthFunc = D3D11_COMPARISON_LESS;  // 통상적인 깊이 판정의 경우, 
														   //즉 후면 버퍼에 이미 기록된 픽셀 깊이보다, 현재 픽셀 깊이가 더작은 경우만
														   // 판정을 성공 하겠다.
	drawReflectionDesc.StencilEnable = true; // 스텐실 판정을 쓰도록 한다.
	drawReflectionDesc.StencilReadMask = 0xff;  // 스텐실 판정 조건문 중에서 ReadMask에 해당되는 값이다.
	drawReflectionDesc.StencilWriteMask = 0xff; // 특정 비트가 갱신되지 않도록 하는 조건문.

	drawReflectionDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP; // 스텐실 판정 실패시 스텐실 버퍼의 갱신방법.
	drawReflectionDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP; // 스텐실은 통과했으나 깊이에서 실패할 경우
	drawReflectionDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP; // 스텐실과 깊이 모두 통과 했을떼, 연산인데 지금은 그대로 유지한다.
	drawReflectionDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL; //  스텐실 판정 비교 함수를 지정하는 함수. 
																	   // 스텐실 버퍼가 항목이 1인 경우에만 통과하도록 하기 위해서 EQUAL로 설정한다.

																	   // We are not rendering backfacing polygons, so these settings do not matter.
	drawReflectionDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&drawReflectionDesc, &ReflectionDSS));

	// 스텐실 버퍼의 거울만 그릴 것이므로, 후면 버퍼에 그려지는 일이 없도록 할것,
	//
	// MarkMirrorDSS
	//

	D3D11_DEPTH_STENCIL_DESC mirrorDesc;
	mirrorDesc.DepthEnable = true;
	mirrorDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // 깊이 버퍼 쓰기를 비활성화
	mirrorDesc.DepthFunc = D3D11_COMPARISON_LESS;
	mirrorDesc.StencilEnable = true;
	mirrorDesc.StencilReadMask = 0xff;
	mirrorDesc.StencilWriteMask = 0xff;

	mirrorDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE; // 판정 성공시 현재 Ref 값으로 덮어 써버린다.
	mirrorDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;  //스텐실 버퍼에 렌더링 할때는 스텐실 판정이 항상 성공하도록 한다.

																 // We are not rendering backfacing polygons, so these settings do not matter.
	mirrorDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	mirrorDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	HR(device->CreateDepthStencilState(&mirrorDesc, &MirrorDSS));

	
	//
	// GreaterDSS
	//

	D3D11_DEPTH_STENCIL_DESC greaterDesc;
	greaterDesc.DepthEnable = true;
	greaterDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // 깊이 버퍼 쓰기를 비활성화
	greaterDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	greaterDesc.StencilEnable = TRUE;
	greaterDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	greaterDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	greaterDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP; // 판정 성공시 현재 Ref 값으로 덮어 써버린다.
	greaterDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;  //스텐실 버퍼에 렌더링 할때는 스텐실 판정이 항상 성공하도록 한다.

																 // We are not rendering backfacing polygons, so these settings do not matter.
	greaterDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&greaterDesc, &GreaterDSS));

	// 스텐실 버퍼의 거울만 그릴 것이므로, 후면 버퍼에 그려지는 일이 없도록 할것,
	//
	// LessDSS
	//

	D3D11_DEPTH_STENCIL_DESC LessDesc;
	greaterDesc.DepthEnable = true;
	greaterDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // 깊이 버퍼 쓰기를 비활성화
	greaterDesc.DepthFunc = D3D11_COMPARISON_LESS;
	greaterDesc.StencilEnable = TRUE;
	greaterDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	greaterDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	greaterDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP; // 판정 성공시 현재 Ref 값으로 덮어 써버린다.
	greaterDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;  //스텐실 버퍼에 렌더링 할때는 스텐실 판정이 항상 성공하도록 한다.

																 // We are not rendering backfacing polygons, so these settings do not matter.
	greaterDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&greaterDesc, &LessDSS));



	// 그림자 매핑 시, 중복으로 그림자 매시가 그려지는 것을 막기 위해 이중 혼합을 방지한다.
	//
	// NoDoubleDSS
	//

	D3D11_DEPTH_STENCIL_DESC noDoubleDesc;
	noDoubleDesc.DepthEnable = true;
	noDoubleDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // 깊이 버퍼 쓰기를 비활성화
	noDoubleDesc.DepthFunc = D3D11_COMPARISON_LESS;
	noDoubleDesc.StencilEnable = true;
	noDoubleDesc.StencilReadMask = 0xff;
	noDoubleDesc.StencilWriteMask = 0xff;

	noDoubleDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_ZERO; // 판정 성공시 해당 스텐실 버퍼 항목을 1증가한다.
	noDoubleDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;  //스텐실 버퍼에 항목이 0이랑 일치하는 해당 픽셀만 통과 시킨다.

																  // We are not rendering backfacing polygons, so these settings do not matter.
	noDoubleDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;
	noDoubleDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&noDoubleDesc, &NoDoubleDSS));


	// SSAO 시 중복 깊이 방지
	//
	// EqualDSS
	//

	D3D11_DEPTH_STENCIL_DESC equalDesc;
	ZeroMemory(&equalDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	equalDesc.DepthEnable = true;
	equalDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
	//깊이 판정에 쓰이는 비교함수이다. 통상적인 경우, 픽셀 깊이가 더 작은 경우에 판정이 통과하므로ㅡ
	// LESS를 지정한다. 
	equalDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	//깊이 버퍼 쓰기가 방지되나, 깊이 판정은 수행.

	HR(device->CreateDepthStencilState(&equalDesc, &EqualDSS));

}

void RenderStates::DestroyAll()
{
	SAFE_RELEASE(WireframeRS);
	SAFE_RELEASE(NoCullRS);
	SAFE_RELEASE(AlphaToCoverageBS);
	SAFE_RELEASE(TransparentBS);
}