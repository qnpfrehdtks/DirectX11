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
	// ȿ�� fx���� �̹� ���İ� 0.1������ �κ��� �����ؼ� ������ �ϰ� �ִ�.
	// �ĸ� ������ ��Ȱ��ȭ�Ͽ�, �׸���.
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
	alphaToCoverageDesc.IndependentBlendEnable = false; // ȥ���� ����� ���ΰ�?
	alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	// ���� ��� ���� ����ũ��, ���� �÷��׵��� �ϳ� �̻� �����ؼ� ������ �� �ִ�.
	// �̴� ȥ���� ����� �ĸ� ������ � ���� ä�ε鿡 ����� ���ΰ��� �����Ѵ�.

	HR(device->CreateBlendState(&alphaToCoverageDesc, &AlphaToCoverageBS));
	// ���� �� BlendState ��ü�� ��� ���ձ� �ܰ迡 ������ �Ѵ�. �̸� ���� �޼ҵ�� ���߿�....


	// �ش� Ÿ���� �׸��� �ʰڴٴ� �������� �� ���. �� Blend�� ����Ѵ�.
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
	// ȥ�� ���¸� �ϳ� �����ϰ� ����
	// C = Csrc X Fscr + Cdst X Fdst
	// ���⼭ a�� Source �� ���İ�.
	// C = Csrc X (a,a,a) + Cdst X (1 - a, 1 - a, 1 - a)
	// C = asCdst + (1 - as)Csrc
	// a��  0���� ������ ������, 1�� ������ ���� ����.

	// ���� ��ü�� �׸� �� ��Ģ.
	// Blending�� ���� �ʴ� ��ü�� �켱������ �׸���.
	// �׷� ����, Blending�� ����ϴ� ��ü���� ī�޶���� �Ÿ��� �������� ����.
	// �׷� ����, ī�޶󿡼� �� �ͺ��� �׷� ������. 
	//-> ����? ���� ��ü�� �����ϴٸ� ��ü �ڿ� �ִ� ����� ��ü�� ���� ������ �ϱ� ����.
	// �׷��� �ĸ� ���ۿ��� ������ ��ü �ڿ� �ִ� ��ü���� �̸� �׷����� �Ѵ�. �׷��� ���� �ȼ��� �� ��� ����� �ȼ��� ȥ�� ����.
	// 

	D3D11_BLEND_DESC transparentDesc = { 0 };
	transparentDesc.AlphaToCoverageEnable = false;     // true�� �����ϸ� �Ĺ��̳� â�칮 �ؽ�ó�� �������� 
													   // ������ ���� ǥ��ȭ ����� ���� ������ ��ȯ�� Ȱ��ȭ �Ѵ�.
	transparentDesc.IndependentBlendEnable = false;     // true�� �ϸ� ȥ���� ���������� ������ �� �ִ�.

	transparentDesc.RenderTarget[0].BlendEnable = true;        // D3D11_RENDER_TARGET_BLEND_DESC ���� ������¥��, �迭�� �� �迭��  i��° ���Ҵ�
															   // ���� ���� �����  i��° ���� ��� �ۤ����� ȥ�� ������ ���� ����ü�̴�.

	transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;      // �ҽ� RGB �ȼ��� ȥ�� ���.
	transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // ��� RGB �ȼ��� ȥ�� ���.
	transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;          // ȥ�� ���� ADD

	transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;       // �ҽ� �ȼ��� Alpha ȥ�� ���.
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;     // ��� �ȼ��� Alpha ȥ�� ���.
	transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;     // ȥ�� ���� ���ϱ�.


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

	/// ���ٽ� ���� ���ǹ�
	// StencilRef & ReadMask ���ǹ� Value & ReadMask �� ���̸� ���ٽ� ���ۿ� �ȼ��� ����ϰ�, �����̸� �ȼ� �Ⱒ.

	D3D11_DEPTH_STENCIL_DESC drawReflectionDesc;
	drawReflectionDesc.DepthEnable = true;
	drawReflectionDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // ���� ���� ���⿡�� Ȱ��ȭ�ȴ�.
	drawReflectionDesc.DepthFunc = D3D11_COMPARISON_LESS;  // ������� ���� ������ ���, 
														   //�� �ĸ� ���ۿ� �̹� ��ϵ� �ȼ� ���̺���, ���� �ȼ� ���̰� ������ ��츸
														   // ������ ���� �ϰڴ�.
	drawReflectionDesc.StencilEnable = true; // ���ٽ� ������ ������ �Ѵ�.
	drawReflectionDesc.StencilReadMask = 0xff;  // ���ٽ� ���� ���ǹ� �߿��� ReadMask�� �ش�Ǵ� ���̴�.
	drawReflectionDesc.StencilWriteMask = 0xff; // Ư�� ��Ʈ�� ���ŵ��� �ʵ��� �ϴ� ���ǹ�.

	drawReflectionDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP; // ���ٽ� ���� ���н� ���ٽ� ������ ���Ź��.
	drawReflectionDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP; // ���ٽ��� ��������� ���̿��� ������ ���
	drawReflectionDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP; // ���ٽǰ� ���� ��� ��� ������, �����ε� ������ �״�� �����Ѵ�.
	drawReflectionDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL; //  ���ٽ� ���� �� �Լ��� �����ϴ� �Լ�. 
																	   // ���ٽ� ���۰� �׸��� 1�� ��쿡�� ����ϵ��� �ϱ� ���ؼ� EQUAL�� �����Ѵ�.

																	   // We are not rendering backfacing polygons, so these settings do not matter.
	drawReflectionDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&drawReflectionDesc, &ReflectionDSS));

	// ���ٽ� ������ �ſ︸ �׸� ���̹Ƿ�, �ĸ� ���ۿ� �׷����� ���� ������ �Ұ�,
	//
	// MarkMirrorDSS
	//

	D3D11_DEPTH_STENCIL_DESC mirrorDesc;
	mirrorDesc.DepthEnable = true;
	mirrorDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // ���� ���� ���⸦ ��Ȱ��ȭ
	mirrorDesc.DepthFunc = D3D11_COMPARISON_LESS;
	mirrorDesc.StencilEnable = true;
	mirrorDesc.StencilReadMask = 0xff;
	mirrorDesc.StencilWriteMask = 0xff;

	mirrorDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE; // ���� ������ ���� Ref ������ ���� �������.
	mirrorDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;  //���ٽ� ���ۿ� ������ �Ҷ��� ���ٽ� ������ �׻� �����ϵ��� �Ѵ�.

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
	greaterDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // ���� ���� ���⸦ ��Ȱ��ȭ
	greaterDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	greaterDesc.StencilEnable = TRUE;
	greaterDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	greaterDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	greaterDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP; // ���� ������ ���� Ref ������ ���� �������.
	greaterDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;  //���ٽ� ���ۿ� ������ �Ҷ��� ���ٽ� ������ �׻� �����ϵ��� �Ѵ�.

																 // We are not rendering backfacing polygons, so these settings do not matter.
	greaterDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&greaterDesc, &GreaterDSS));

	// ���ٽ� ������ �ſ︸ �׸� ���̹Ƿ�, �ĸ� ���ۿ� �׷����� ���� ������ �Ұ�,
	//
	// LessDSS
	//

	D3D11_DEPTH_STENCIL_DESC LessDesc;
	greaterDesc.DepthEnable = true;
	greaterDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // ���� ���� ���⸦ ��Ȱ��ȭ
	greaterDesc.DepthFunc = D3D11_COMPARISON_LESS;
	greaterDesc.StencilEnable = TRUE;
	greaterDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	greaterDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	greaterDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP; // ���� ������ ���� Ref ������ ���� �������.
	greaterDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;  //���ٽ� ���ۿ� ������ �Ҷ��� ���ٽ� ������ �׻� �����ϵ��� �Ѵ�.

																 // We are not rendering backfacing polygons, so these settings do not matter.
	greaterDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	greaterDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&greaterDesc, &LessDSS));



	// �׸��� ���� ��, �ߺ����� �׸��� �Žð� �׷����� ���� ���� ���� ���� ȥ���� �����Ѵ�.
	//
	// NoDoubleDSS
	//

	D3D11_DEPTH_STENCIL_DESC noDoubleDesc;
	noDoubleDesc.DepthEnable = true;
	noDoubleDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // ���� ���� ���⸦ ��Ȱ��ȭ
	noDoubleDesc.DepthFunc = D3D11_COMPARISON_LESS;
	noDoubleDesc.StencilEnable = true;
	noDoubleDesc.StencilReadMask = 0xff;
	noDoubleDesc.StencilWriteMask = 0xff;

	noDoubleDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_ZERO; // ���� ������ �ش� ���ٽ� ���� �׸��� 1�����Ѵ�.
	noDoubleDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;  //���ٽ� ���ۿ� �׸��� 0�̶� ��ġ�ϴ� �ش� �ȼ��� ��� ��Ų��.

																  // We are not rendering backfacing polygons, so these settings do not matter.
	noDoubleDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;
	noDoubleDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&noDoubleDesc, &NoDoubleDSS));


	// SSAO �� �ߺ� ���� ����
	//
	// EqualDSS
	//

	D3D11_DEPTH_STENCIL_DESC equalDesc;
	ZeroMemory(&equalDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	equalDesc.DepthEnable = true;
	equalDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
	//���� ������ ���̴� ���Լ��̴�. ������� ���, �ȼ� ���̰� �� ���� ��쿡 ������ ����ϹǷΤ�
	// LESS�� �����Ѵ�. 
	equalDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	//���� ���� ���Ⱑ �����ǳ�, ���� ������ ����.

	HR(device->CreateDepthStencilState(&equalDesc, &EqualDSS));

}

void RenderStates::DestroyAll()
{
	SAFE_RELEASE(WireframeRS);
	SAFE_RELEASE(NoCullRS);
	SAFE_RELEASE(AlphaToCoverageBS);
	SAFE_RELEASE(TransparentBS);
}