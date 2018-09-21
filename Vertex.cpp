#include "DXUT.h"
#include "Vertex.h"
#include "d3dx11effect.h"
#include "EffectMGR.h"


#pragma region InputLayoutDesc

D3D11_INPUT_ELEMENT_DESC InputLayOutDesc::PosDesc[1] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

D3D11_INPUT_ELEMENT_DESC InputLayOutDesc::SimpleDesc[2] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

D3D11_INPUT_ELEMENT_DESC InputLayOutDesc::BasicDesc[3] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

D3D11_INPUT_ELEMENT_DESC InputLayOutDesc::DissolveDesc[3] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "COLOR",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

D3D11_INPUT_ELEMENT_DESC InputLayOutDesc::SkinDesc[6] =
{
	{ "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "TANGENT",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "WEIGHTS",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "BONEINDICES",  0, DXGI_FORMAT_R32G32B32A32_SINT,   0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

D3D11_INPUT_ELEMENT_DESC InputLayOutDesc::NormalBasicDesc[4] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

#pragma endregion

#pragma region inputLayout
ID3D11InputLayout* InputLayOut::PosInputLayout = 0;
ID3D11InputLayout* InputLayOut::SimpleInputLayout = 0;
ID3D11InputLayout* InputLayOut::BasicInputLayout = 0;
ID3D11InputLayout* InputLayOut::DissolveInputLayout = 0;
ID3D11InputLayout* InputLayOut::NormalInputLayout = 0;
ID3D11InputLayout* InputLayOut::SkinInputLayout = 0;
#pragma endregion

void InputLayOut::InitAllInputLayout(ID3D11Device * device)
{
	
	D3DX11_PASS_DESC passDesc;
	UINT numElements;

	//  ===============================================================  //
	//                              Pos  
	// ================================================================

	numElements = ARRAYSIZE(InputLayOutDesc::PosDesc);

	EffectMGR::SkyE->GetTech("SkyRender")->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayOutDesc::PosDesc, 1, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &PosInputLayout));

	//  ===============================================================  //
	//                            Basic
	// ================================================================

	numElements = ARRAYSIZE(InputLayOutDesc::BasicDesc);

	EffectMGR::BasicE->GetTech("DirLight")->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayOutDesc::BasicDesc, 3, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &BasicInputLayout));



	//  ===============================================================  //
	//                              SIMPLE 
	// ================================================================


	numElements = ARRAYSIZE(InputLayOutDesc::SimpleDesc);

	EffectMGR::SimpleE->GetTech("Light1")->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayOutDesc::SimpleDesc, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &SimpleInputLayout));


	//  ===============================================================  //
	//                              Normal
	// ================================================================


	numElements = ARRAYSIZE(InputLayOutDesc::DissolveDesc);

	EffectMGR::DissolveE->GetTech("BasicDissolve")->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayOutDesc::DissolveDesc, 3, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &DissolveInputLayout));

	//  ===============================================================  //
	//                              Skin
	// ================================================================
	numElements = ARRAYSIZE(InputLayOutDesc::SkinDesc);

	EffectMGR::BasicE->GetTech("SkinDirLight")->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayOutDesc::SkinDesc, 6, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &SkinInputLayout));

	//numElements = ARRAYSIZE(InputLayOutDesc::NormalBasicDesc);

	//EffectMGR::NormalE->GetTech("DirLight")->GetPassByIndex(0)->GetDesc(&passDesc);
	//HR(device->CreateInputLayout(InputLayOutDesc::NormalBasicDesc, numElements, passDesc.pIAInputSignature,
	//	passDesc.IAInputSignatureSize, &NormalInputLayout));

}
