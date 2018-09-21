#pragma once

#include "DXUT.h"
#include "d3dx11effect.h"
#include "LightMGR.h"
#include <iostream>
#include <map>
#include <unordered_map>


#ifndef EFFECTMGR_H
#define EFFECTMGR_H


using namespace DirectX;


class Effect {


protected:
	std::unordered_map<std::string, ID3DX11EffectTechnique* > m_TechTable;
	ID3DX11Effect * m_Effect;
	ID3D11Device* m_Device;

public:
	void InsertTech(LPCSTR str);
	ID3DX11EffectTechnique* GetTech(LPCSTR str);

public:
	Effect(ID3D11Device* device, LPCWSTR filename);
	virtual ~Effect();
};

#pragma region SimpleEffect
class SimpleEffect : public Effect
{

private:


public:
	SimpleEffect(ID3D11Device* device, LPCWSTR fxname);
	~SimpleEffect() {};

	// world, view, Proj , tex Space 값 설정.
	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorld(CXMMATRIX M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M) { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexTransform(CXMMATRIX M) { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }

	//  Fog 설정.  // 
	void SetFogColor(const FXMVECTOR v) { FogColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetFogStart(float f) { FogStart->SetFloat(f); }
	void SetFogRange(float f) { FogRange->SetFloat(f); }

	// 빛, 재질 설정.
	void SetDirLights(const sDirLight* lights) { DirLights->SetRawValue(lights, 0, 3 * sizeof(sDirLight)); }
	void SetMaterial(const sMaterial& mat) { Mat->SetRawValue(&mat, 0, sizeof(sMaterial)); }

	// SRV 값 설정.
	void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }


	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* TexTransform;

	ID3DX11EffectVectorVariable* EyePosW;

	ID3DX11EffectVectorVariable* FogColor;
	ID3DX11EffectScalarVariable* FogStart;
	ID3DX11EffectScalarVariable* FogRange;

	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* Mat;

	ID3DX11EffectShaderResourceVariable* DiffuseMap;
};
#pragma endregion

#pragma region SkyEffect
class SkyEffect : public Effect
{

private:


public:
	SkyEffect(ID3D11Device* device, LPCWSTR fxname);
	~SkyEffect() {};

	// world, view, Proj , tex Space 값 설정.
	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }

	// SRV 값 설정.
	void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }
	void SetCubeMap(ID3D11ShaderResourceView* tex) { CubeMap->SetResource(tex); }

	ID3DX11EffectMatrixVariable* WorldViewProj;

	ID3DX11EffectShaderResourceVariable* DiffuseMap;
	ID3DX11EffectShaderResourceVariable* CubeMap;
};
#pragma endregion

#pragma region BasicEffect
class BasicEffect : public Effect
{

private:


public:
	BasicEffect(ID3D11Device* device, LPCWSTR fxname);
	~BasicEffect() {};

	void SetToonTickness(float edge) { ToonTickness->SetFloat(edge); }

	// world, view, Proj , tex Space 값 설정.
	void SetWorldViewProj(CXMMATRIX M)             { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetViewProj(CXMMATRIX M)                  { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldViewProjTex(CXMMATRIX M)          { WorldViewProjTex->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetToonWVP(CXMMATRIX M) { ToonWVP->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorld(CXMMATRIX M)                     { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M)         { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetShadowTransform(CXMMATRIX M)           { ShadowTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexTransform(CXMMATRIX M)              { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetOutLineTransform(CXMMATRIX M)          { OutLineTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetEyePosW(const XMFLOAT3& v)             { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetBoneTransform(const XMFLOAT4X4* M, int count) 
	{ BoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(M), 0, count); }

	//  Fog 설정.  // 
	void SetFogColor(const FXMVECTOR v) { FogColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetFogStart(float f) { FogStart->SetFloat(f); }
	void SetFogRange(float f) { FogRange->SetFloat(f); }

	// 빛, 재질 설정.
	void SetDirLights(const sDirLight* lights, UINT count = 1) { DirLights->SetRawValue(lights, 0, count * sizeof(sDirLight)); }
	void SetPtLights(const sPtLight* lights, UINT count = 1) { PtLights->SetRawValue(lights, 0, count * sizeof(sPtLight)); }

	void SetMaterial(const sMaterial& mat) { Mat->SetRawValue(&mat, 0, sizeof(sMaterial)); }

	void SetRimColor(const XMFLOAT4& v) { RimLightColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetRimWidth(float f) { RimLightWidth->SetFloat(f); }
	// SRV 값 설정.
	void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }
	void SetShadowMap(ID3D11ShaderResourceView* tex) { ShadowMap->SetResource(tex); }
	void SetSsaoMap(ID3D11ShaderResourceView* tex) { SsaoMap->SetResource(tex); }
	void SetCubeMap(ID3D11ShaderResourceView* tex) { CubeMap->SetResource(tex); }
	void SetOutLine(ID3D11ShaderResourceView* tex) { OutLineMap->SetResource(tex); }
	void SetNormalMap(ID3D11ShaderResourceView* tex) { NormalMap->SetResource(tex); }
	void SetSpecMap(ID3D11ShaderResourceView* tex) { SpecMap->SetResource(tex); }

	void SetDissolveMap(ID3D11ShaderResourceView* tex) { DissolveMap->SetResource(tex); }
	void SetDissolveColorMap(ID3D11ShaderResourceView* tex) { DissolveColorMap->SetResource(tex); }

	void SetEdge(float edge) { Edge->SetFloat(edge); }
	void SetEdgeRange(float edge) { EdgeRange->SetFloat(edge); }
	void SetProgress(float edge) { Progress->SetFloat(edge); }


	ID3DX11EffectMatrixVariable* WorldViewProj;

	ID3DX11EffectMatrixVariable* ViewProj;
	ID3DX11EffectMatrixVariable* WorldViewProjTex;
	ID3DX11EffectMatrixVariable* ToonWVP;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* ShadowTransform;
	ID3DX11EffectMatrixVariable* TexTransform;
	ID3DX11EffectMatrixVariable* OutLineTransform;
	ID3DX11EffectMatrixVariable* BoneTransforms;

	ID3DX11EffectVectorVariable* EyePosW;

	ID3DX11EffectVectorVariable* FogColor;
	ID3DX11EffectScalarVariable* FogStart;
	ID3DX11EffectScalarVariable* FogRange;

	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* PtLights;
	ID3DX11EffectVariable* Mat;

	ID3DX11EffectVectorVariable* RimLightColor;
	ID3DX11EffectScalarVariable* RimLightWidth;

	ID3DX11EffectShaderResourceVariable* DiffuseMap;
	ID3DX11EffectShaderResourceVariable* NormalMap;
	ID3DX11EffectShaderResourceVariable* SpecMap;
	ID3DX11EffectShaderResourceVariable* ShadowMap;
	ID3DX11EffectShaderResourceVariable* OutLineMap;
	ID3DX11EffectShaderResourceVariable* SsaoMap;
	ID3DX11EffectShaderResourceVariable* CubeMap;


	ID3DX11EffectShaderResourceVariable* DissolveMap;
	ID3DX11EffectShaderResourceVariable* DissolveColorMap;

	ID3DX11EffectScalarVariable * ToonTickness;

	ID3DX11EffectScalarVariable * EdgeRange;
	ID3DX11EffectScalarVariable * Edge;
	ID3DX11EffectScalarVariable * Progress;

};
#pragma endregion

#pragma region ShadowEffect
class ShadowEffect : public Effect
{

private:


public:
	ShadowEffect(ID3D11Device* device, LPCWSTR fxname);
	~ShadowEffect() {};

	// world, view, Proj , tex Space 값 설정.
	void SetWorld(CXMMATRIX M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M) { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }

	void SetViewProj(CXMMATRIX M) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexTransform(CXMMATRIX M) { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }

	void SetBoneTransform(const XMFLOAT4X4* M, int count){
		BoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(M), 0, count);
	}

	
	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	// SRV 값 설정.
	void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }

	void SetNormalMap(ID3D11ShaderResourceView* normal) { NormalSRV->SetResource(normal); }

	void SetHeightScale(const FLOAT min) { HeightScale->SetFloat(min); }
	void SetMinTessDistance(const FLOAT min) { MinTessDistance->SetFloat(min); }
	void SetMaxTessDistance(const FLOAT max) { MaxTessDistance->SetFloat(max); }
	void SetMinTessFactor(const FLOAT scale) { MinTessFactor->SetFloat(scale); }
	void SetMaxTessFactor(const FLOAT scale) { MaxTessFactor->SetFloat(scale); }

	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* ViewProj;
	ID3DX11EffectMatrixVariable* TexTransform;
	ID3DX11EffectMatrixVariable* World;

	ID3DX11EffectVectorVariable* EyePosW;

	ID3DX11EffectShaderResourceVariable* DiffuseMap;

	ID3DX11EffectScalarVariable* HeightScale;
	ID3DX11EffectScalarVariable* MinTessDistance;
	ID3DX11EffectScalarVariable* MaxTessDistance;
	ID3DX11EffectScalarVariable* MinTessFactor;
	ID3DX11EffectScalarVariable* MaxTessFactor;

	ID3DX11EffectShaderResourceVariable * NormalSRV;
	ID3DX11EffectMatrixVariable* BoneTransforms;

};
#pragma endregion



#pragma region ShadowCubeMap
class ShadowCubeMapEffect : public Effect
{

private:


public:
	ShadowCubeMapEffect(ID3D11Device* device, LPCWSTR fxname);
	~ShadowCubeMapEffect() {};

	// world, view, Proj , tex Space 값 설정.
	void SetWorld(CXMMATRIX M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M) { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }

	//void SetViewProj(CXMMATRIX M) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetShadowGenMat(CXMMATRIX M) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexTransform(CXMMATRIX M) { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }

	void SetShadowGenMat(const XMFLOAT4X4* M, int count) {
		ShadowGenMat->SetMatrixArray(reinterpret_cast<const float*>(M), 0, count);
	}

	void SetBoneTransform(const XMFLOAT4X4* M, int count) {
		BoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(M), 0, count);
	}


	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	// SRV 값 설정.
	void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }

	void SetNormalMap(ID3D11ShaderResourceView* normal) { NormalSRV->SetResource(normal); }

	void SetHeightScale(const FLOAT min) { HeightScale->SetFloat(min); }


	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* ViewProj;
	ID3DX11EffectMatrixVariable* TexTransform;
	ID3DX11EffectMatrixVariable* World;

	ID3DX11EffectVectorVariable* EyePosW;

	ID3DX11EffectShaderResourceVariable* DiffuseMap;

	ID3DX11EffectScalarVariable* HeightScale;


	ID3DX11EffectShaderResourceVariable * NormalSRV;
	ID3DX11EffectMatrixVariable* BoneTransforms;
	ID3DX11EffectMatrixVariable* ShadowGenMat;

};
#pragma endregion



#pragma region OutlineEffect
class OutlineEffect : public Effect
{

private:


public:
	OutlineEffect(ID3D11Device* device, LPCWSTR fxname);
	~OutlineEffect() {};

	// world, view, Proj , tex Space 값 설정.
	//void SetViewToProj(CXMMATRIX M) { ViewToProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldViewProjInvTranspose(CXMMATRIX M) { WorldViewProjInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetFrustumCorners(const XMFLOAT4 v[4]) { FrustumCorners->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 4); }
	void SetOffsetVectors(const XMFLOAT4 v[14]) { OffsetVectors->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 14); }
	/*void SetWorldViewProjTex(CXMMATRIX M) { WorldViewProjTex->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorld(CXMMATRIX M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M) { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }*/
	//void SetShadowTransform(CXMMATRIX M) { ShadowTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexTransform(CXMMATRIX M) { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetViewToTexSpace(CXMMATRIX M) { ViewToTexSpace->SetMatrix(reinterpret_cast<const float*>(&M)); }
	//void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }

	//  Fog 설정.  // 
	//void SetFogColor(const FXMVECTOR v) { FogColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	//void SetFogStart(float f) { FogStart->SetFloat(f); }
	//void SetFogRange(float f) { FogRange->SetFloat(f); }

	// 빛, 재질 설정.
	//void SetDirLights(const sDirLight* lights, UINT count = 1) { DirLights->SetRawValue(lights, 0, count * sizeof(sDirLight)); }
	//void SetPtLights(const sPtLight* lights, UINT count = 1) { PtLights->SetRawValue(lights, 0, count * sizeof(sPtLight)); }
	//void SetMaterial(const sMaterial& mat) { Mat->SetRawValue(&mat, 0, sizeof(sMaterial)); }

	//void SetRimColor(const XMFLOAT4& v) { RimLightColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	//void SetRimWidth(float f) { RimLightWidth->SetFloat(f); }
	// SRV 값 설정.
	//void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }
	//void SetShadowMap(ID3D11ShaderResourceView* tex) { ShadowMap->SetResource(tex); }
	void SetNormalDepthMap(ID3D11ShaderResourceView* tex) { NormalDepthMap->SetResource(tex); }
	//void SetCubeMap(ID3D11ShaderResourceView* tex) { CubeMap->SetResource(tex); }

	ID3DX11EffectMatrixVariable* ViewToProj;
	ID3DX11EffectMatrixVariable* WorldViewProjInvTranspose;
	ID3DX11EffectVectorVariable* FrustumCorners;
	//ID3DX11EffectMatrixVariable* WorldViewProjTex;
	//ID3DX11EffectMatrixVariable* World;
	//ID3DX11EffectMatrixVariable* WorldInvTranspose;
	//ID3DX11EffectMatrixVariable* ShadowTransform;
	ID3DX11EffectMatrixVariable* TexTransform;
	//ID3DX11EffectVectorVariable* EyePosW;

	//ID3DX11EffectVectorVariable* FogColor;
	//ID3DX11EffectScalarVariable* FogStart;
	//ID3DX11EffectScalarVariable* FogRange;

	//ID3DX11EffectVariable* DirLights;
	//ID3DX11EffectVariable* PtLights;
	//ID3DX11EffectVariable* Mat;

	ID3DX11EffectVectorVariable* OffsetVectors;
	//ID3DX11EffectScalarVariable* RimLightWidth;

	//ID3DX11EffectShaderResourceVariable* DiffuseMap;
	//ID3DX11EffectShaderResourceVariable* ShadowMap;
	ID3DX11EffectMatrixVariable* ViewToTexSpace;
	ID3DX11EffectShaderResourceVariable* NormalDepthMap;
	//ID3DX11EffectShaderResourceVariable* CubeMap;
};
#pragma endregion


#pragma region GBufferUnPackEffect
class GBufferPackEffect : public Effect
{

private:


public:
	GBufferPackEffect(ID3D11Device* device, LPCWSTR fxname);
	~GBufferPackEffect() { };

	// world, view, Proj , tex Space 값 설정.
	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetViewProj(CXMMATRIX M) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldView(CXMMATRIX M) { WorldView->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetView(CXMMATRIX M) { View->SetMatrix(reinterpret_cast<const float*>(&M)); }
	//void SetWorldViewProjTex(CXMMATRIX M) { WorldViewProjTex->SetMatrix(reinterpret_cast<const float*>(&M)); }
	//void SetWorldInvTransposeView(CXMMATRIX M) { WorldInvTransposeView->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorld(CXMMATRIX M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetShadowTransform(CXMMATRIX M) { ShadowTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M) { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }

	void SetCubeMap(ID3D11ShaderResourceView* tex) { CubeMap->SetResource(tex); }
//	void SetTexTransform(CXMMATRIX M) { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
//	void SetOutLineTransform(CXMMATRIX M) { OutLineTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetBoneTransform(const XMFLOAT4X4* M, int count)
	{
		BoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(M), 0, count);
	}
	
	void SetHeightScale(float f) { HeightScale->SetFloat(f); }
	void SetSpecInt(float f) { SpecIntensity->SetFloat(f); }
	void SetSpecExp(float f) { SpecExp->SetFloat(f); }

	//  Fog 설정.  // 
//	void SetFogColor(const FXMVECTOR v) { FogColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
//	void SetFogStart(float f) { FogStart->SetFloat(f); }
//	void SetFogRange(float f) { FogRange->SetFloat(f); }

	// 빛, 재질 설정.
//	void SetDirLights(const sDirLight* lights, UINT count = 1) { DirLights->SetRawValue(lights, 0, count * sizeof(sDirLight)); }
//	void SetPtLights(const sPtLight* lights, UINT count = 1) { PtLights->SetRawValue(lights, 0, count * sizeof(sPtLight)); }

//	void SetMaterial(const sMaterial& mat) { Mat->SetRawValue(&mat, 0, sizeof(sMaterial)); }

//	void SetRimColor(const XMFLOAT4& v) { RimLightColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
//	void SetRimWidth(float f) { RimLightWidth->SetFloat(f); }
	// SRV 값 설정.
	void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }
	void SetNormalMap(ID3D11ShaderResourceView* tex) { NormalMap->SetResource(tex); }
	//void SetTextureMap(ID3D11ShaderResourceView* tex) { NormalMap->SetResource(tex); }
	void SetSpecMap(ID3D11ShaderResourceView* tex) { SpecMap->SetResource(tex); }
	void SetShadowMap(ID3D11ShaderResourceView* tex) { ShadowMap->SetResource(tex); }
	void SetHeightMap(ID3D11ShaderResourceView* tex) { HeightMap->SetResource(tex); }

	void SetMaskMap(ID3D11ShaderResourceView* tex) { MaskMap->SetResource(tex); }

	void SetIsBumped(bool Bumped) { isBumped->SetBool(Bumped); }
	void SetIsSpeced(bool spec) { isSpec->SetBool(spec); }
	void SetIsMask(bool mask) { isMask->SetBool(mask); }
	void SetIsPOM(bool mask) { isPOM->SetBool(mask); }
	void SetIsSSR(bool mask) { isSSR->SetBool(mask); }
	void SetIsShadowed(bool Bumped) { isShadow->SetBool(Bumped); }
	
//	void SetDissolveMap(ID3D11ShaderResourceView* tex) { DissolveMap->SetResource(tex); }
//	void SetDissolveColorMap(ID3D11ShaderResourceView* tex) { DissolveColorMap->SetResource(tex); }

//	void SetEdge(float edge) { Edge->SetFloat(edge); }
//	void SetEdgeRange(float edge) { EdgeRange->SetFloat(edge); }
//	void SetProgress(float edge) { Progress->SetFloat(edge); }


	ID3DX11EffectMatrixVariable* WorldViewProj;

	ID3DX11EffectMatrixVariable* ViewProj;
	ID3DX11EffectMatrixVariable* View;
//	ID3DX11EffectMatrixVariable* WorldInvTransposeView;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldView;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* ShadowTransform;
	ID3DX11EffectMatrixVariable* TexTransform;
	ID3DX11EffectMatrixVariable* OutLineTransform;
	ID3DX11EffectMatrixVariable* BoneTransforms;

	ID3DX11EffectVectorVariable* EyePosW;

	ID3DX11EffectVectorVariable* FogColor;
	ID3DX11EffectScalarVariable* FogStart;
	ID3DX11EffectScalarVariable* FogRange;

	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* PtLights;
	ID3DX11EffectVariable* Mat;

	ID3DX11EffectVectorVariable* RimLightColor;
	ID3DX11EffectScalarVariable* RimLightWidth;

	ID3DX11EffectShaderResourceVariable* DiffuseMap;
	ID3DX11EffectShaderResourceVariable* ShadowMap;
	ID3DX11EffectShaderResourceVariable* NormalMap;
	ID3DX11EffectShaderResourceVariable* SpecMap;
	ID3DX11EffectShaderResourceVariable* HeightMap;
	ID3DX11EffectShaderResourceVariable* MaskMap;

	ID3DX11EffectShaderResourceVariable* OutLineMap;
	ID3DX11EffectShaderResourceVariable* SsaoMap;
	ID3DX11EffectShaderResourceVariable* CubeMap;


	ID3DX11EffectShaderResourceVariable* DissolveMap;
	ID3DX11EffectShaderResourceVariable* DissolveColorMap;

	ID3DX11EffectScalarVariable * isBumped;
	ID3DX11EffectScalarVariable * isSpec;
	ID3DX11EffectScalarVariable * isMask;
	ID3DX11EffectScalarVariable * isPOM;
	ID3DX11EffectScalarVariable * isSSR;
	ID3DX11EffectScalarVariable * isShadow;

	ID3DX11EffectScalarVariable * EdgeRange;
	ID3DX11EffectScalarVariable * Edge;
	ID3DX11EffectScalarVariable * Progress;

	ID3DX11EffectScalarVariable * SpecExp;
	ID3DX11EffectScalarVariable * SpecIntensity;
	ID3DX11EffectScalarVariable * HeightScale;

};
#pragma endregion

#pragma region SSAOEffect
class SSAOEffect : public Effect
{

private:


public:
	SSAOEffect(ID3D11Device* device, LPCWSTR fxname);
	~SSAOEffect() {};

	void SetDepthMap(ID3D11ShaderResourceView* srv) { DepthMap->SetResource(srv); }
	void SetNormalMap(ID3D11ShaderResourceView* srv) { NormalMap->SetResource(srv); }
	void SetRandomMap(ID3D11ShaderResourceView* srv) { RandomVectorMap->SetResource(srv); }

	//void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetFrustumCorners(const XMFLOAT4 v[4]) { FrustumCorners->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 4); }
	void SetOffsetVector(const XMFLOAT4 v[14]) { OffsetVectors->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 14); }

	void SetProjProperty(const XMFLOAT4& v) { ProjMatProperty->SetFloatVector(reinterpret_cast<const float*>(&v)); }

	void SetView(CXMMATRIX m) { View->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetInvProj(CXMMATRIX m) { InvProj->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetViewToTex(CXMMATRIX m) { ViewToTexSpace->SetMatrix(reinterpret_cast<const float*>(&m)); }

	void SetRadius(float f) { SSAORadius->SetFloat(f); }
	void SetIntesity(float f) { SSAOIntensity->SetFloat(f); }


	ID3DX11EffectShaderResourceVariable * DepthMap;
	ID3DX11EffectShaderResourceVariable * NormalMap;
	ID3DX11EffectShaderResourceVariable * RandomVectorMap;

	ID3DX11EffectVectorVariable* ProjMatProperty;
	ID3DX11EffectVectorVariable* FrustumCorners;
	ID3DX11EffectVectorVariable* OffsetVectors;

	ID3DX11EffectMatrixVariable* View;
	ID3DX11EffectMatrixVariable* InvProj;
	ID3DX11EffectMatrixVariable* ViewToTexSpace;

	ID3DX11EffectScalarVariable * SSAORadius;
	ID3DX11EffectScalarVariable * SSAOIntensity;
};
#pragma endregion

#pragma region SSAOEffect2
class SSAOEffectSecond : public Effect
{

private:


public:
	SSAOEffectSecond(ID3D11Device* device, LPCWSTR fxname);
	~SSAOEffectSecond() {};

	void SetDepthMap(ID3D11ShaderResourceView* srv) { DepthMap->SetResource(srv); }
	void SetNormalMap(ID3D11ShaderResourceView* srv) { NormalMap->SetResource(srv); }
	void SetRandomMap(ID3D11ShaderResourceView* srv) { RandomVectorMap->SetResource(srv); }

	//void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetFrustumCorners(const XMFLOAT4 v[4]) { FrustumCorners->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 4); }
	void SetOffsetVector(const XMFLOAT4 v[14]) { OffsetVectors->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 14); }

	void SetProjProperty(const XMFLOAT4& v) { ProjMatProperty->SetFloatVector(reinterpret_cast<const float*>(&v)); }

	void SetView(CXMMATRIX m) { View->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetInvView(CXMMATRIX m) { InvView->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetInvProj(CXMMATRIX m) { InvProj->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetViewToTex(CXMMATRIX m) { ViewToTexSpace->SetMatrix(reinterpret_cast<const float*>(&m)); }

	ID3DX11EffectShaderResourceVariable * DepthMap;
	ID3DX11EffectShaderResourceVariable * NormalMap;
	ID3DX11EffectShaderResourceVariable * RandomVectorMap;

	ID3DX11EffectVectorVariable* ProjMatProperty;
	ID3DX11EffectVectorVariable* FrustumCorners;
	ID3DX11EffectVectorVariable* OffsetVectors;

	ID3DX11EffectMatrixVariable* View;
	ID3DX11EffectMatrixVariable* InvView;
	ID3DX11EffectMatrixVariable* InvProj;
	ID3DX11EffectMatrixVariable* ViewToTexSpace;
};
#pragma endregion


#pragma region GlowGeoEffect
class GlowGeoEffect : public Effect
{

private:


public:
	GlowGeoEffect(ID3D11Device* device, LPCWSTR fxname);
	~GlowGeoEffect() {};

	void SetEyePos(const XMFLOAT3 v) { EyePos->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetColor(const XMFLOAT4 v) { Color->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetSize(const XMFLOAT4& v) { Size->SetFloatVector(reinterpret_cast<const float*>(&v)); }

	void SetViewProj(CXMMATRIX m) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&m)); }

	ID3DX11EffectVectorVariable* EyePos;
	ID3DX11EffectVectorVariable* Color;
	ID3DX11EffectVectorVariable* Size;

	ID3DX11EffectMatrixVariable* ViewProj;

};
#pragma endregion


#pragma region SSREffect
class SSREffect : public Effect
{

private:


public:
	SSREffect(ID3D11Device* device, LPCWSTR fxname);
	~SSREffect() {};

	void SetEyePos(const XMFLOAT3 v) { EyePos->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetProperty(const XMFLOAT4 v) { Property->SetFloatVector(reinterpret_cast<const float*>(&v)); }

	void SetInvProj(CXMMATRIX m) { InvProj->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetProj(CXMMATRIX m) { Proj->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetView(CXMMATRIX m) { View->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetViewProj(CXMMATRIX m) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetWorld(CXMMATRIX m) { World->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetInvView(CXMMATRIX m) { InvView->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetInvViewProj(CXMMATRIX m) { InvViewProj->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetTexToWorld(CXMMATRIX m) { TexToWorld->SetMatrix(reinterpret_cast<const float*>(&m)); }

	void SetDepthMap(ID3D11ShaderResourceView* srv) { DepthMap->SetResource(srv); }
	void SetNormalMap(ID3D11ShaderResourceView* srv) { NormalMap->SetResource(srv); }
	void SetDiffuseMap(ID3D11ShaderResourceView* srv) { DiffuseMap->SetResource(srv); }
	void SetSpecMap(ID3D11ShaderResourceView* srv) { SpecMap->SetResource(srv); }

	void SetOutUAV(ID3D11UnorderedAccessView* uav) { OutputUAV->SetUnorderedAccessView(uav); }

	void setSSRPower(float f) { SSRPower->SetFloat(f); }

	ID3DX11EffectShaderResourceVariable * DepthMap;
	ID3DX11EffectShaderResourceVariable * NormalMap;
	ID3DX11EffectShaderResourceVariable * DiffuseMap;
	ID3DX11EffectShaderResourceVariable * SpecMap;

	ID3DX11EffectUnorderedAccessViewVariable* OutputUAV; // Texture 출력 자원 뷰 Effect.

	ID3DX11EffectVectorVariable* EyePos;
	ID3DX11EffectVectorVariable* Property;

	ID3DX11EffectMatrixVariable* InvProj;
	ID3DX11EffectMatrixVariable* Proj;
	ID3DX11EffectMatrixVariable* View;
	ID3DX11EffectMatrixVariable* ViewProj;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* InvView;
	ID3DX11EffectMatrixVariable* InvViewProj;
	ID3DX11EffectMatrixVariable* TexToWorld;

	ID3DX11EffectScalarVariable* SSRPower;

};
#pragma endregion


#pragma region SSREffect
class SSREffect2 : public Effect
{

private:


public:
	SSREffect2(ID3D11Device* device, LPCWSTR fxname);
	~SSREffect2() {};

	void SetEyePos(const XMFLOAT3 v) { EyePos->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetPerspectiveValue(const XMFLOAT4 v) { PerspectiveValue->SetFloatVector(reinterpret_cast<const float*>(&v)); }

	void SetInvProj(CXMMATRIX m) { InvProj->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetProj(CXMMATRIX m) { Proj->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetView(CXMMATRIX m) { View->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetViewProj(CXMMATRIX m) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetWorld(CXMMATRIX m) { World->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetWorldView(CXMMATRIX m) { WorldView->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetWorldViewProj(CXMMATRIX m) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetInvView(CXMMATRIX m) { InvView->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetInvViewProj(CXMMATRIX m) { InvViewProj->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetTexToWorld(CXMMATRIX m) { TexToWorld->SetMatrix(reinterpret_cast<const float*>(&m)); }

	void SetDepthMap(ID3D11ShaderResourceView* srv) { DepthMap->SetResource(srv); }
	void SetNormalMap(ID3D11ShaderResourceView* srv) { NormalMap->SetResource(srv); }
	void SetDiffuseMap(ID3D11ShaderResourceView* srv) { DiffuseMap->SetResource(srv); }
	void SetSpecMap(ID3D11ShaderResourceView* srv) { SpecMap->SetResource(srv); }

	void SetOutUAV(ID3D11UnorderedAccessView* uav) { OutputUAV->SetUnorderedAccessView(uav); }

	void SetViewAngleThreshold(float f) { ViewAngleThreshold->SetFloat(f); }
	void SetEdgeDistThreshold(float f) { EdgeDistThreshold->SetFloat(f); }
	void SetDepthBias(float f) { DepthBias->SetFloat(f); }
	void SetReflectionScale(float f) { ReflectiveValue->SetFloat(f); }


	ID3DX11EffectShaderResourceVariable * DepthMap;
	ID3DX11EffectShaderResourceVariable * NormalMap;
	ID3DX11EffectShaderResourceVariable * DiffuseMap;
	ID3DX11EffectShaderResourceVariable * SpecMap;

	ID3DX11EffectUnorderedAccessViewVariable* OutputUAV; // Texture 출력 자원 뷰 Effect.

	ID3DX11EffectVectorVariable* EyePos;
	ID3DX11EffectVectorVariable* PerspectiveValue;

	ID3DX11EffectMatrixVariable* InvProj;
	ID3DX11EffectMatrixVariable* Proj;
	ID3DX11EffectMatrixVariable* View;
	ID3DX11EffectMatrixVariable* ViewProj;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldView;
	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* InvView;
	ID3DX11EffectMatrixVariable* InvViewProj;
	ID3DX11EffectMatrixVariable* TexToWorld;

	ID3DX11EffectScalarVariable* ViewAngleThreshold;
	ID3DX11EffectScalarVariable* EdgeDistThreshold;
	ID3DX11EffectScalarVariable* DepthBias;
	ID3DX11EffectScalarVariable* ReflectiveValue;

};
#pragma endregion



#pragma region HazeHeatEffect
class HazeHeatEffect : public Effect
{

private:


public:
	HazeHeatEffect(ID3D11Device* device, LPCWSTR fxname);
	~HazeHeatEffect() {};

	void SetDiffuseMap(ID3D11ShaderResourceView* srv) { DiffuseMap->SetResource(srv); }
	void SetNoiseMap(ID3D11ShaderResourceView* srv) { NoiseMap->SetResource(srv); }

	void SetEyePos(const XMFLOAT3 v) { EyePos->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetColor(const XMFLOAT4 v) { Color->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetSize(const XMFLOAT4& v) { Size->SetFloatVector(reinterpret_cast<const float*>(&v)); }

	void SetViewProj(CXMMATRIX m) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&m)); }

	void SetTime(float f) { Time->SetFloat(f); }

	ID3DX11EffectShaderResourceVariable* DiffuseMap;
	ID3DX11EffectShaderResourceVariable* NoiseMap;

	ID3DX11EffectVectorVariable* EyePos;
	ID3DX11EffectVectorVariable* Color;
	ID3DX11EffectVectorVariable* Size;

	ID3DX11EffectMatrixVariable* ViewProj;

	ID3DX11EffectScalarVariable * Time;

};
#pragma endregion


#pragma region SpriteAnimationEffect
class SpriteAnimationEffect : public Effect
{

private:


public:
	SpriteAnimationEffect(ID3D11Device* device, LPCWSTR fxname);
	~SpriteAnimationEffect() {};

	void SetTexArray(ID3D11ShaderResourceView* srv) { TexArray->SetResource(srv); }
	void SetNoiseMap(ID3D11ShaderResourceView* srv) { NoiseSRV->SetResource(srv); }

	void SetEyePos(const XMFLOAT3 v) { EyePos->SetFloatVector(reinterpret_cast<const float*>(&v)); }
//	void SetColor(const XMFLOAT4 v) { Color->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetSize(const XMFLOAT4& v) { Size->SetFloatVector(reinterpret_cast<const float*>(&v)); }

	void SetViewProj(CXMMATRIX m) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&m)); }

	void SetIndex(float f) { Index->SetFloat(f); }
	void SetTime(float f) { Time->SetFloat(f); }

	ID3DX11EffectShaderResourceVariable* TexArray;
	ID3DX11EffectShaderResourceVariable* NoiseSRV;

	ID3DX11EffectVectorVariable* EyePos;
	ID3DX11EffectVectorVariable* Size;

	ID3DX11EffectMatrixVariable* ViewProj;

	ID3DX11EffectScalarVariable * Index;
	ID3DX11EffectScalarVariable * Time;


};
#pragma endregion



#pragma region SSAOBlur
class SSAOBlurEffect : public Effect
{

private:


public:
	SSAOBlurEffect(ID3D11Device* device, LPCWSTR fxname);
	~SSAOBlurEffect() {};

	void SetDepthMap(ID3D11ShaderResourceView* srv) { DepthMap->SetResource(srv); }
	void SetNormalMap(ID3D11ShaderResourceView* srv) { NormalMap->SetResource(srv); }
	void SetInputMap(ID3D11ShaderResourceView* srv) { InputMap->SetResource(srv); }

	void SetView(CXMMATRIX m) { View->SetMatrix(reinterpret_cast<const float*>(&m)); }

	void SetTexWidth(float f) { TexWidth->SetFloat(f); }
	void SetTexHeight(float f) { TexHeight->SetFloat(f); }

	void SetVectorAB(const XMFLOAT4 v) { VectorAB->SetFloatVector(reinterpret_cast<const float*>(&v)); }


	ID3DX11EffectShaderResourceVariable * DepthMap;
	ID3DX11EffectShaderResourceVariable * NormalMap;
	ID3DX11EffectShaderResourceVariable * InputMap;

	ID3DX11EffectVectorVariable* VectorAB;

	ID3DX11EffectMatrixVariable* View;

	ID3DX11EffectScalarVariable * TexWidth;
	ID3DX11EffectScalarVariable * TexHeight;
};
#pragma endregion

#pragma region NormalDepthEffect
class NormalDepthEffect : public Effect
{

private:


public:
	NormalDepthEffect(ID3D11Device* device, LPCWSTR fxname);
	~NormalDepthEffect() {};

	// world, view, Proj , tex Space 값 설정.
	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldView(CXMMATRIX M) { WorldView->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorld(CXMMATRIX M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldViewInvTranspose(CXMMATRIX M) { WorldInvTransposeView->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M) { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexTransform(CXMMATRIX M) { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldView;
	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* WorldInvTransposeView;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* TexTransform;
};
#pragma endregion

#pragma region BasicBlurEffect
class BasicBlurEffect : public Effect
{

private:
	ID3DX11EffectShaderResourceVariable * DepthSRV;
	ID3DX11EffectShaderResourceVariable * InputSRV;
	ID3DX11EffectUnorderedAccessViewVariable* OutputUAV; // Texture 출력 자원 뷰 Effect.

public:
	BasicBlurEffect(ID3D11Device* device, LPCWSTR fxname);
	~BasicBlurEffect() {};

	void SetDepthSRV(ID3D11ShaderResourceView* srv)   { DepthSRV->SetResource(srv); }
	void SetInputSRV(ID3D11ShaderResourceView* srv)   { InputSRV->SetResource(srv); }
	void SetOutUAV(ID3D11UnorderedAccessView* uav)    { OutputUAV->SetUnorderedAccessView(uav); }

};
#pragma endregion

#pragma region SSAOBlurEffect
class SSAOBlurCSEffect : public Effect
{

private:
	ID3DX11EffectShaderResourceVariable * NormalSRV;
	ID3DX11EffectShaderResourceVariable * DepthSRV;
	ID3DX11EffectShaderResourceVariable * InputSRV;
	ID3DX11EffectUnorderedAccessViewVariable* OutputUAV; // Texture 출력 자원 뷰 Effect.

	ID3DX11EffectVectorVariable* VectorAB;

public:
	SSAOBlurCSEffect(ID3D11Device* device, LPCWSTR fxname);
	~SSAOBlurCSEffect() {};

	void SetVectorAB(const XMFLOAT4 v) { VectorAB->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetNormalSRV(ID3D11ShaderResourceView* srv) { NormalSRV->SetResource(srv); }
	void SetDepthSRV(ID3D11ShaderResourceView* srv) { DepthSRV->SetResource(srv); }
	void SetInputSRV(ID3D11ShaderResourceView* srv) { InputSRV->SetResource(srv); }
	void SetOutUAV(ID3D11UnorderedAccessView* uav) { OutputUAV->SetUnorderedAccessView(uav); }

};
#pragma endregion


//#pragma region BasicBlur2Effect
//class BasicBlur2Effect : public Effect
//{
//
//private:
//	ID3DX11EffectScalarVariable * ResX;
//	ID3DX11EffectScalarVariable* ResY;
//
//	ID3DX11EffectShaderResourceVariable * InputSRV;
//	ID3DX11EffectUnorderedAccessViewVariable* OutputUAV; // Texture 출력 자원 뷰 Effect.
//
//public:
//	BasicBlur2Effect(ID3D11Device* device, LPCWSTR fxname);
//	~BasicBlur2Effect() {};
//
//	ID3DX11EffectScalarVariable* SetResX(int X) { ResX->SetInt(X); }
//	ID3DX11EffectScalarVariable* SetResY(int Y) { ResY->SetInt(Y); }
//	
//	void SetInputSRV(ID3D11ShaderResourceView* srv) { InputSRV->SetResource(srv); }
//	void SetOutUAV(ID3D11UnorderedAccessView* uav) { OutputUAV->SetUnorderedAccessView(uav); }
//
//};
//#pragma endregion

#pragma region HDRDownScaleEffect
class HDRDownScaleEffect : public Effect
{

private:
	ID3DX11EffectScalarVariable * ResX;
	ID3DX11EffectScalarVariable* ResY;

	ID3DX11EffectScalarVariable* Domain;

	ID3DX11EffectScalarVariable* GroupSize;

	ID3DX11EffectScalarVariable* BloomThreshold;
	ID3DX11EffectScalarVariable* Adaptation;

	ID3DX11EffectShaderResourceVariable * Avg1DSRV;
	ID3DX11EffectShaderResourceVariable * HDRInputSRV;
	ID3DX11EffectUnorderedAccessViewVariable* Avg1DUAV;

	ID3DX11EffectUnorderedAccessViewVariable* DownScale1DUAV;
	ID3DX11EffectUnorderedAccessViewVariable* DownScaleUAV;
	// Bloom 용
	ID3DX11EffectShaderResourceVariable * InputAvgLumSRV;
	ID3DX11EffectShaderResourceVariable * InputDownScaleSRV;

	ID3DX11EffectUnorderedAccessViewVariable* OutputBloomUAV;


public:
	HDRDownScaleEffect(ID3D11Device* device, LPCWSTR fxname);
	~HDRDownScaleEffect() {};

	void SetResX(const UINT x) { ResX->SetFloat(x); }
	void SetResY(const UINT y) { ResY->SetFloat(y); }
	void SetDomain(const UINT d) { Domain->SetFloat(d); }
	void SetGroupSize(const UINT g) { GroupSize->SetFloat(g); }

	void SetBloomThreshold(const float x) { BloomThreshold->SetFloat(x); }
	void SetAdaptation(const float x) { Adaptation->SetFloat(x); }

	void SetAvg1DSRV(ID3D11ShaderResourceView* srv) { Avg1DSRV->SetResource(srv); }
	void SetAvg1DUAV(ID3D11UnorderedAccessView* uav) { Avg1DUAV->SetUnorderedAccessView(uav); }

	void SetHDRInputSRV(ID3D11ShaderResourceView* srv) { HDRInputSRV->SetResource(srv); }

	void SetDownScale1DUAV(ID3D11UnorderedAccessView* uav) { DownScale1DUAV->SetUnorderedAccessView(uav); }
	void SetDownScaleUAV(ID3D11UnorderedAccessView* uav) { DownScaleUAV->SetUnorderedAccessView(uav); }

	void SetBloomAvgLumSRV(ID3D11ShaderResourceView* srv) { InputAvgLumSRV->SetResource(srv); }
	void SetBloomDownScaleSRV(ID3D11ShaderResourceView* srv) { InputDownScaleSRV->SetResource(srv); }

	void SetOutputBloomUAV(ID3D11UnorderedAccessView* uav) { OutputBloomUAV->SetUnorderedAccessView(uav); }


};
#pragma endregion

#pragma region HDRToneMapping
class HDRToneMappingEffect : public Effect
{

private:
	ID3DX11EffectMatrixVariable * World;
	ID3DX11EffectMatrixVariable * WorldViewProj;
	ID3DX11EffectMatrixVariable * InvProj;
	ID3DX11EffectMatrixVariable * TexTransform;
	ID3DX11EffectScalarVariable * MiddleGray;
	ID3DX11EffectScalarVariable* WhiteSqrt;
	ID3DX11EffectScalarVariable* BloomScale;

	ID3DX11EffectScalarVariable* ProjA;
	ID3DX11EffectScalarVariable* ProjB;

	ID3DX11EffectShaderResourceVariable * HDRTex;
	ID3DX11EffectShaderResourceVariable * AvgLumSRV;

	ID3DX11EffectShaderResourceVariable * BloomSRV;


public:
	HDRToneMappingEffect(ID3D11Device* device, LPCWSTR fxname);
	~HDRToneMappingEffect() {};

	void SetWorld(CXMMATRIX M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetProjInv(CXMMATRIX M) { InvProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexTransform(CXMMATRIX M) { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }

	void SetMiddelGray(float middleGray) { MiddleGray->SetFloat(middleGray); }
	void SetWhiteSqrt(float whiteSqrt) { WhiteSqrt->SetFloat(whiteSqrt); }
	void SetBloomScale(float bloomScale) { BloomScale->SetFloat(bloomScale); }

	void SetInputHDRSRV(ID3D11ShaderResourceView* srv) { HDRTex->SetResource(srv); }
	void SetAvgLumSRV(ID3D11ShaderResourceView* srv) { AvgLumSRV->SetResource(srv); }
	void SetBloomSRV(ID3D11ShaderResourceView* srv) { BloomSRV->SetResource(srv); }

	void SetProjA(float projA) { ProjA->SetFloat(projA); }
	void SetProjB(float projB) { ProjB->SetFloat(projB); }

};
#pragma endregion

#pragma region DebuggingEffect
class DebuggingEffect : public Effect
{
public:
	DebuggingEffect(ID3D11Device * device, LPCWSTR fxname);
	~DebuggingEffect() {}

public:
	// SRV 값 설정.
	void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }
	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }



private:
	ID3DX11EffectMatrixVariable * WorldViewProj;
	ID3DX11EffectShaderResourceVariable* DiffuseMap;
};
#pragma endregion


#pragma region DeferredShading
class DeferredShadingEffect : public Effect
{
public:
	DeferredShadingEffect(ID3D11Device * device, LPCWSTR fxname);
	~DeferredShadingEffect() {}

public:

	void SetDepthMap(ID3D11ShaderResourceView* srv)           { DepthMap->SetResource(srv); }
	void SetDiffuseSpecIntMap(ID3D11ShaderResourceView* srv)  { DiffuseSpecIntMap->SetResource(srv); }
	void SetNormalMap(ID3D11ShaderResourceView* srv)          { NormalMap->SetResource(srv); }
	void SetSpecPowMap(ID3D11ShaderResourceView* srv)         { SpecPowMap->SetResource(srv); }
	void SetSSAOMap(ID3D11ShaderResourceView* srv)            { SSAOMap->SetResource(srv); }

	void SetDirLights(const sDirLight* lights, UINT count = 1) { DirLights->SetRawValue(lights, 0, count * sizeof(sDirLight)); }
	void SetPtLights(sPtLight* lights, UINT count = 1) { PointLights->SetRawValue(lights, 0, count * sizeof(sPtLight)); }

	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetFrustumCorners(const XMFLOAT4 v[4]) { FrustumCorners->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 4); }

	void SetProjProperty(const XMFLOAT4& v) { ProjMatProperty->SetFloatVector(reinterpret_cast<const float*>(&v)); }

	void SetInvProj(CXMMATRIX m) { InvProj->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetInvView(CXMMATRIX m) { InvView->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetProjTex(CXMMATRIX m) { ProjTex->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetInvViewProj(CXMMATRIX m) { InvViewProj->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetWorldViewProj(CXMMATRIX m) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&m)); }

	void SetShadowTransform(CXMMATRIX M) { ShadowTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetShadowMap(ID3D11ShaderResourceView* tex) { ShadowMap->SetResource(tex); }
private:

	ID3DX11EffectShaderResourceVariable * DepthMap;
	ID3DX11EffectShaderResourceVariable * DiffuseSpecIntMap;
	ID3DX11EffectShaderResourceVariable * NormalMap;
	ID3DX11EffectShaderResourceVariable * SpecPowMap;
	ID3DX11EffectShaderResourceVariable* ShadowMap;
	ID3DX11EffectShaderResourceVariable* SSAOMap;

	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* PointLights;
	ID3DX11EffectVariable* EyePosW;

	ID3DX11EffectVectorVariable* ProjMatProperty;
	ID3DX11EffectVectorVariable* FrustumCorners;
	ID3DX11EffectMatrixVariable* ShadowTransform;

	ID3DX11EffectMatrixVariable* InvView;
	ID3DX11EffectMatrixVariable* ProjTex;
	ID3DX11EffectMatrixVariable* InvViewProj;
	ID3DX11EffectMatrixVariable* InvProj;
	ID3DX11EffectMatrixVariable* WorldViewProj;



};
#pragma endregion


#pragma region DeferredShadingPointLight
class DeferredShadingPointLightEffect : public Effect
{
public:
	DeferredShadingPointLightEffect(ID3D11Device * device, LPCWSTR fxname);
	~DeferredShadingPointLightEffect() {}

public:

	void SetDepthMap(ID3D11ShaderResourceView* srv) { DepthMap->SetResource(srv); }
	void SetDiffuseSpecIntMap(ID3D11ShaderResourceView* srv) { DiffuseSpecIntMap->SetResource(srv); }
	void SetNormalMap(ID3D11ShaderResourceView* srv) { NormalMap->SetResource(srv); }
	void SetSpecPowMap(ID3D11ShaderResourceView* srv) { SpecPowMap->SetResource(srv); }

	void SetPtLights(const sPtLight* lights, UINT count = 1) { PointLights->SetRawValue(lights, 0, count * sizeof(sPtLight));  }
	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetFrustumCorners(const XMFLOAT4 v[4]) { FrustumCorners->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 4); }

	void SetProjProperty(const XMFLOAT4& v) { ProjMatProperty->SetFloatVector(reinterpret_cast<const float*>(&v)); }

	void SetInvView(CXMMATRIX m) { InvView->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetInVP(CXMMATRIX m) { InvViewProj->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetLightProj(CXMMATRIX m) { LightProjection->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetWorldViewProj(CXMMATRIX m) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&m)); }
private:

	ID3DX11EffectShaderResourceVariable * DepthMap;
	ID3DX11EffectShaderResourceVariable * DiffuseSpecIntMap;
	ID3DX11EffectShaderResourceVariable * NormalMap;
	ID3DX11EffectShaderResourceVariable * SpecPowMap;

	ID3DX11EffectVariable* PointLights;
	ID3DX11EffectVariable* EyePosW;

	ID3DX11EffectVectorVariable* ProjMatProperty;
	ID3DX11EffectVectorVariable* FrustumCorners;

	ID3DX11EffectMatrixVariable* LightProjection;
	ID3DX11EffectMatrixVariable* InvViewProj;
	ID3DX11EffectMatrixVariable* InvView;
	ID3DX11EffectMatrixVariable* WorldViewProj;



};
#pragma endregion


#pragma region TileBasedDeferredEffect
class TileBasedDeferredEffect : public Effect
{
public:
	TileBasedDeferredEffect(ID3D11Device * device, LPCWSTR fxname);
	~TileBasedDeferredEffect() {}

public:

	void SetDepthMap(ID3D11ShaderResourceView* srv) { DepthMap->SetResource(srv); }
	void SetDiffuseSpecIntMap(ID3D11ShaderResourceView* srv) { DiffuseSpecIntMap->SetResource(srv); }
	void SetNormalMap(ID3D11ShaderResourceView* srv) { NormalMap->SetResource(srv); }
	void SetSpecPowMap(ID3D11ShaderResourceView* srv) { SpecPowMap->SetResource(srv); }
	void SetSSAOMap(ID3D11ShaderResourceView* srv) { SSAOMap->SetResource(srv); }
	void SetLightSRV(ID3D11ShaderResourceView* srv) { LightSRV->SetResource(srv); }
	void SetDirLightSRV(ID3D11ShaderResourceView* srv) { DirLightSRV->SetResource(srv); }
	void SetOutUAV(ID3D11UnorderedAccessView* uav) { OutputUAV->SetUnorderedAccessView(uav); }

	/*void SetDirLights(const sDirLight* lights, UINT count = 1) {
		DirLights->SetRawValue(lights, 0, count * sizeof(sDirLight));  
		LightCount += count;
	}
	void SetPtLights(sPtLight* lights, UINT count = 1) 
	{ PointLights->SetRawValue(lights, 0, count * sizeof(sPtLight)); 
	  LightCount += count;
	}*/

	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetFrustumCorners(const XMFLOAT4 v[4]) { FrustumCorners->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 4); }

	void SetProjProperty(const XMFLOAT4& v) { ProjMatProperty->SetFloatVector(reinterpret_cast<const float*>(&v)); }

	void SetView(CXMMATRIX m) { View->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetInvProj(CXMMATRIX m) { InvProj->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetInvView(CXMMATRIX m) { InvView->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetProj(CXMMATRIX m) { Proj->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetInvViewProj(CXMMATRIX m) { InvViewProj->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetWorldViewProj(CXMMATRIX m) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&m)); }
	void SetViewProj(CXMMATRIX m) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&m)); }

	void SetShadowTransform(CXMMATRIX M) { ShadowTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetShadowMap(ID3D11ShaderResourceView* tex) { ShadowMap->SetResource(tex); }
	void SetCubeMap(ID3D11ShaderResourceView* tex) { CubeMap->SetResource(tex); }

	void SetLightCount(int count) { LightCount->SetInt(count); }


	void SetRenderMode(int i) {

		switch (i) {
		case 0:
			SetisNormal(false);
			SetisSSAO(false);
			SetisTiledBase(false);
			SetisDepthBase(false);
			SetisDiffuseBase(false);
			break;
		case 1:
			SetisNormal(true);
			SetisSSAO(false);
			SetisTiledBase(false);
			SetisDepthBase(false);
			SetisDiffuseBase(false);
			break;
		case 2:
			SetisNormal(false);
			SetisSSAO(true);
			SetisTiledBase(false);
			SetisDepthBase(false);
			SetisDiffuseBase(false);
			break;
		case 3:
			SetisNormal(false);
			SetisSSAO(false);
			SetisTiledBase(true);
			SetisDepthBase(false);
			SetisDiffuseBase(false);
			break;
		case 4:
			SetisNormal(false);
			SetisSSAO(false);
			SetisTiledBase(false);
			SetisDepthBase(true);
			SetisDiffuseBase(false);
			break;
		case 5:
			SetisNormal(false);
			SetisSSAO(false);
			SetisTiledBase(false);
			SetisDepthBase(false);
			SetisDiffuseBase(true);
			break;

		}
		
	}

	void SetisNormal(float edge) { isNormal->SetBool(edge); }
	void SetisSSAO(bool edge) { isSSAO->SetBool(edge); }
	void SetisTiledBase(float edge) { isTileBase->SetBool(edge); }
	void SetisDepthBase(float edge) { isDepth->SetBool(edge); }
	void SetisDiffuseBase(float edge) { isDiffuse->SetBool(edge); }
private:

	ID3DX11EffectShaderResourceVariable * DepthMap;
	ID3DX11EffectShaderResourceVariable * DiffuseSpecIntMap;
	ID3DX11EffectShaderResourceVariable * NormalMap;
	ID3DX11EffectShaderResourceVariable * SpecPowMap;
	ID3DX11EffectShaderResourceVariable* ShadowMap;
	ID3DX11EffectShaderResourceVariable* SSAOMap;
	ID3DX11EffectShaderResourceVariable* LightSRV;
	ID3DX11EffectShaderResourceVariable* DirLightSRV;
	ID3DX11EffectShaderResourceVariable* CubeMap;

	ID3DX11EffectUnorderedAccessViewVariable* OutputUAV;

	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* PointLights;
	ID3DX11EffectVariable* EyePosW;


	ID3DX11EffectVectorVariable* ProjMatProperty;
	ID3DX11EffectVectorVariable* FrustumCorners;
	ID3DX11EffectMatrixVariable* ShadowTransform;

	ID3DX11EffectMatrixVariable* InvView;
	ID3DX11EffectMatrixVariable* View;
	ID3DX11EffectMatrixVariable* Proj;
	ID3DX11EffectMatrixVariable* InvViewProj;
	ID3DX11EffectMatrixVariable* InvProj;
	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* ViewProj;

	ID3DX11EffectScalarVariable* LightCount;

	ID3DX11EffectScalarVariable* isSSAO;
	ID3DX11EffectScalarVariable* isNormal;
	ID3DX11EffectScalarVariable* isDepth;
	ID3DX11EffectScalarVariable* isDiffuse;
	ID3DX11EffectScalarVariable* isTileBase;



};
#pragma endregion



#pragma region DissolveEffect

class DissolveEffect : public Effect
{
public:
	DissolveEffect(ID3D11Device * device, LPCWSTR fxname);
	~DissolveEffect() {}

public:
	// SRV 값 설정.
	void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }
	void SetDissolveMap(ID3D11ShaderResourceView* tex) { DissolveMap->SetResource(tex); }
	void SetDissolveColorMap(ID3D11ShaderResourceView* tex) { DissolveColorMap->SetResource(tex); }

	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexTransform(CXMMATRIX M) { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }


	

	void SetEdge(float edge) { Edge->SetFloat(edge); }
	void SetEdgeRange(float edge) { EdgeRange->SetFloat(edge); }
	void SetProgress(float edge) { Progress->SetFloat(edge); }


private:
	ID3DX11EffectMatrixVariable * WorldViewProj;
	ID3DX11EffectShaderResourceVariable* DiffuseMap;



	ID3DX11EffectMatrixVariable * TexTransform;

	ID3DX11EffectShaderResourceVariable* DissolveMap;
	ID3DX11EffectShaderResourceVariable* DissolveColorMap;

	ID3DX11EffectScalarVariable * EdgeRange;
	ID3DX11EffectScalarVariable * Edge;
	ID3DX11EffectScalarVariable * Progress;
};

#pragma endregion

#pragma region Parallax Occulusion Mapping
class POMEffect : public Effect
{

private:


public:
	POMEffect(ID3D11Device* device, LPCWSTR fxname);
	~POMEffect() {};

	// world, view, Proj , tex Space 값 설정.
	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorld(CXMMATRIX M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }

	// SRV 값 설정.
	void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }
	void SetHeightMap(ID3D11ShaderResourceView* tex) { HeightMap->SetResource(tex); }
	void SetSpecMap(ID3D11ShaderResourceView* tex) { SpecularMap->SetResource(tex); }


	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* World;

	ID3DX11EffectVectorVariable* EyePosW;

	ID3DX11EffectShaderResourceVariable* DiffuseMap;
	ID3DX11EffectShaderResourceVariable* HeightMap;
	ID3DX11EffectShaderResourceVariable* SpecularMap;


};
#pragma endregion



class EffectMGR
{


private:
	EffectMGR();
	~EffectMGR()
	{
		DestroyAll();
	}

public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();


public:
	static BasicEffect* BasicE;
	static SimpleEffect* SimpleE;
	static SkyEffect* SkyE;
	static ShadowEffect* ShadowE;
	static OutlineEffect* OutLineE;
	static NormalDepthEffect* NormalDepthE;
	static BasicBlurEffect* BlurE;
	static HDRDownScaleEffect* HDRDownScaleE;
	static HDRToneMappingEffect* ToneMappingE;
	static DebuggingEffect* DebuggingE;
	static DissolveEffect* DissolveE;
	static GBufferPackEffect* gBufferE;
	static DeferredShadingEffect* DeferredShadingE;
	static DeferredShadingPointLightEffect* DeferredShadingPtE;
	static SSAOEffect* SsaoE;
	static SSAOEffectSecond* SsaoSecondE;
	static SSAOBlurEffect* SSAOBlurE;
	static SSAOBlurCSEffect* SSAOCSBlurE;
	static TileBasedDeferredEffect* TileE;
	static GlowGeoEffect* GlowE;
	static HazeHeatEffect* HazeE;
	static SpriteAnimationEffect* SpriteE;
	static POMEffect* PomE;
	static SSREffect* ScreenSpaceEffect;
	static SSREffect2* ScreenSpaceEffect2;
	static ShadowCubeMapEffect* ShadowCubeMapE;
	//static BasicBlur2Effect* Blur2E;
	//static NormalEffect* NormalE;

};


#endif
