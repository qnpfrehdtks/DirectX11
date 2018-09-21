#include "DXUT.h"
#include "EffectMGR.h"
#include <iostream>
#include "SDKmisc.h"

#pragma region EffectMGR

BasicEffect*           EffectMGR::BasicE = nullptr;
SimpleEffect*          EffectMGR::SimpleE = nullptr;
SkyEffect*             EffectMGR::SkyE = nullptr;
ShadowEffect*          EffectMGR::ShadowE = nullptr;
OutlineEffect*         EffectMGR::OutLineE = nullptr;
NormalDepthEffect*     EffectMGR::NormalDepthE = nullptr;
BasicBlurEffect*       EffectMGR::BlurE = nullptr;
HDRDownScaleEffect*    EffectMGR::HDRDownScaleE = nullptr;
HDRToneMappingEffect*  EffectMGR::ToneMappingE = nullptr;
DebuggingEffect*       EffectMGR::DebuggingE = nullptr;
DissolveEffect*        EffectMGR::DissolveE = nullptr;
GBufferPackEffect*     EffectMGR::gBufferE = nullptr;
DeferredShadingEffect*             EffectMGR::DeferredShadingE = nullptr;
DeferredShadingPointLightEffect*   EffectMGR::DeferredShadingPtE = nullptr;
SSAOEffect*                        EffectMGR::SsaoE = nullptr;
SSAOEffectSecond*                  EffectMGR::SsaoSecondE = nullptr;
SSAOBlurEffect*                    EffectMGR::SSAOBlurE = nullptr;
SSAOBlurCSEffect*                  EffectMGR::SSAOCSBlurE = nullptr;
TileBasedDeferredEffect*           EffectMGR::TileE = nullptr;
GlowGeoEffect*                     EffectMGR::GlowE = nullptr;
HazeHeatEffect*                    EffectMGR::HazeE = nullptr;
SpriteAnimationEffect*                    EffectMGR::SpriteE = nullptr;
POMEffect*                                EffectMGR::PomE = nullptr;
SSREffect*                                EffectMGR::ScreenSpaceEffect = nullptr;
SSREffect2*                        EffectMGR::ScreenSpaceEffect2 = nullptr;
ShadowCubeMapEffect*               EffectMGR::ShadowCubeMapE = nullptr;
void EffectMGR::InitAll(ID3D11Device* device)
{
	ScreenSpaceEffect = new SSREffect(device, L"FX/SSR2.fx");
	SsaoSecondE =          new SSAOEffectSecond(device, L"FX/SSAOSecond.fx");
	SSAOBlurE =            new SSAOBlurEffect(device, L"FX/EdgeBlur.fx");
	BasicE =               new BasicEffect(device, L"FX/Basic.fx");
	SimpleE =              new SimpleEffect(device, L"FX/Simple.fx");
	SkyE =                 new SkyEffect(device, L"FX/Sky.fx");
	ShadowE =              new ShadowEffect(device, L"FX/BuildShadowMap.fx");
	OutLineE =             new OutlineEffect(device, L"FX/OutLineShading.fx");
	NormalDepthE =         new NormalDepthEffect(device, L"FX/BuildNoramlZ.fx");
	BlurE =                new BasicBlurEffect(device, L"FX/Blur.fx");
	HDRDownScaleE =        new HDRDownScaleEffect(device, L"FX/PostDownScaleFX.fx");
	ToneMappingE =         new HDRToneMappingEffect(device, L"FX/PosFX.fx");
	DebuggingE =           new DebuggingEffect(device, L"FX/Rendering.fx");
	DissolveE =            new DissolveEffect(device, L"FX/DissolveEdge.fx");
	gBufferE =             new GBufferPackEffect(device, L"FX/GBuffer.fx");
	DeferredShadingE =     new DeferredShadingEffect(device, L"FX/DeferredShadingDirLight.fx");
	DeferredShadingPtE =   new DeferredShadingPointLightEffect(device, L"FX/DeferredShadingPointLight.fx");
	SsaoE =                new SSAOEffect(device, L"FX/SSAO.fx");
	SSAOCSBlurE =          new SSAOBlurCSEffect(device, L"FX/SSAOBlurCS.fx");
	TileE =                new TileBasedDeferredEffect(device, L"FX/PerTileBaseRenderingCS.fx");
	GlowE =                new GlowGeoEffect(device, L"FX/GlowGS.fx");
	HazeE = new HazeHeatEffect(device, L"FX/HeatHaze.fx");
	SpriteE = new SpriteAnimationEffect(device, L"FX/SpriteFX.fx");
	PomE = new POMEffect(device, L"FX/ParellOcculusionMappingFX.fx");
	ScreenSpaceEffect2 = new SSREffect2(device, L"FX/SSR.fx");
	ShadowCubeMapE = new ShadowCubeMapEffect(device, L"FX/BuildShadowCubeMap.fx");
	//SsaoSecondE = new SSAOEffectSecond(device, L"FX/SSAOSecond.fx");
}

void EffectMGR::DestroyAll()
{
	SAFE_DELETE(BasicE);
	SAFE_DELETE(SimpleE);
	SAFE_DELETE(SkyE);
	SAFE_DELETE(OutLineE);
	SAFE_DELETE(ShadowE);
	SAFE_DELETE(NormalDepthE);
	SAFE_DELETE(BlurE);
	SAFE_DELETE(HDRDownScaleE);
	SAFE_DELETE(DebuggingE);
	SAFE_DELETE(SsaoE);

	SAFE_DELETE(ToneMappingE);
}

#pragma endregion




#pragma region Effect

void Effect::InsertTech(LPCSTR str)
{
	//if (!m_Effect) return;

	ID3DX11EffectTechnique * temp = m_Effect->GetTechniqueByName(str);
	m_TechTable.insert({ str, temp });
}

ID3DX11EffectTechnique * Effect::GetTech(LPCSTR str)
{
	auto i = m_TechTable[str];
	if (i == nullptr) return nullptr;


	return m_TechTable[str];
}

Effect::Effect(ID3D11Device* device, LPCWSTR filename) : m_Device(device)
{
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

#if D3D_COMPILER_VERSION >= 46

	// Read the D3DX effect file
	WCHAR str[MAX_PATH];
	DXUTFindDXSDKMediaFileCch(str, MAX_PATH, filename);

	HR(D3DX11CompileEffectFromFile(str, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, dwShaderFlags, 0, m_Device, &m_Effect, nullptr));

#else

	ID3DBlob* pEffectBuffer = nullptr;
	V_RETURN(DXUTCompileFromFile(L"Tutorial11.fx", nullptr, "none", "fx_5_0", dwShaderFlags, 0, &pEffectBuffer));
	hr = D3DX11CreateEffectFromMemory(pEffectBuffer->GetBufferPointer(), pEffectBuffer->GetBufferSize(), 0, pd3dDevice, &g_pEffect);
	SAFE_RELEASE(pEffectBuffer);
	if (FAILED(hr))
		return hr;

#endif
}


Effect::~Effect()
{
	SAFE_RELEASE(m_Effect);
}

#pragma endregion 


#pragma region BasicEffect
BasicEffect::BasicEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("DirLight");
	InsertTech("SkinDirLightNormal");
	InsertTech("SkinDirLightNormalSpec");
	InsertTech("TexDirLight");
	InsertTech("GammaTexDirLight");
	InsertTech("OutLine");
	InsertTech("SimpleLight");
	InsertTech("RimBasicLight");
	InsertTech("BasicDissolve");
	InsertTech("SkinDirLight");
	InsertTech("SkinDirLightNoRim");
	InsertTech("SkinDirLightToon");
	InsertTech("SkinDirLightBumped");
	InsertTech("SkinDirLightNoRimNoBumped");

	WorldViewProj = m_Effect->GetVariableByName("g_WorldViewProj")->AsMatrix();
	
	World = m_Effect->GetVariableByName("g_World")->AsMatrix();
	ViewProj = m_Effect->GetVariableByName("g_ViewProj")->AsMatrix();
	WorldInvTranspose = m_Effect->GetVariableByName("g_WorldInvTranspose")->AsMatrix();
	TexTransform = m_Effect->GetVariableByName("g_TexTransform")->AsMatrix();
	ShadowTransform = m_Effect->GetVariableByName("g_ShadowTransform")->AsMatrix();
	BoneTransforms = m_Effect->GetVariableByName("g_BoneTransform")->AsMatrix();
	OutLineTransform = m_Effect->GetVariableByName("g_OutLineTransform")->AsMatrix();
	WorldViewProjTex = m_Effect->GetVariableByName("g_WorldViewProjTex")->AsMatrix();
	ToonWVP = m_Effect->GetVariableByName("g_ToonWVP")->AsMatrix();

	EyePosW = m_Effect->GetVariableByName("g_EyePosW")->AsVector();

	/*FogColor = m_Effect->GetVariableByName("g_FogColor")->AsVector();
	FogStart = m_Effect->GetVariableByName("g_FogStart")->AsScalar();
	FogRange = m_Effect->GetVariableByName("g_FogRange")->AsScalar();*/
	DirLights = m_Effect->GetVariableByName("g_DirLight");
	PtLights = m_Effect->GetVariableByName("g_PtLight");
	Mat = m_Effect->GetVariableByName("g_Mat");

	RimLightWidth  = m_Effect->GetVariableByName("g_RimLightWidth")->AsScalar();
	RimLightColor = m_Effect->GetVariableByName("g_RimLightColor")->AsVector();


	DiffuseMap = m_Effect->GetVariableByName("g_TextureMap")->AsShaderResource();
	ShadowMap = m_Effect->GetVariableByName("g_ShadowMap")->AsShaderResource();
	OutLineMap = m_Effect->GetVariableByName("g_OutLineMap")->AsShaderResource();
	NormalMap = m_Effect->GetVariableByName("g_NormalMap")->AsShaderResource();
	SpecMap = m_Effect->GetVariableByName("g_SpecularMap")->AsShaderResource();

	DissolveColorMap = m_Effect->GetVariableByName("g_DissolveColorTex")->AsShaderResource();
	DissolveMap = m_Effect->GetVariableByName("g_DissolveTex")->AsShaderResource();


	ToonTickness = m_Effect->GetVariableByName("g_EdgeTickness")->AsScalar();

	Edge = m_Effect->GetVariableByName("g_Edge")->AsScalar();
	EdgeRange = m_Effect->GetVariableByName("g_EdgeRange")->AsScalar();
	Progress = m_Effect->GetVariableByName("g_Progress")->AsScalar();


	/*CubeMap = m_Effect->GetVariableByName("g_CubeMap")->AsShaderResource();
	ShadowMap = m_Effect->GetVariableByName("g_ShadowMap")->AsShaderResource();
	SsaoMap = m_Effect->GetVariableByName("g_SsaoMap")->AsShaderResource();*/

}
#pragma endregion 

SimpleEffect::SimpleEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("Light1");

	WorldViewProj = m_Effect->GetVariableByName("g_WorldViewProj")->AsMatrix();
	World = m_Effect->GetVariableByName("g_World")->AsMatrix();
	//WorldInvTranspose = m_Effect->GetVariableByName("g_WorldInvTranspose")->AsMatrix();
	//TexTransform = m_Effect->GetVariableByName("g_TexTransform")->AsMatrix();
	//EyePosW = m_Effect->GetVariableByName("g_EyePosW")->AsVector();
	//FogColor = m_Effect->GetVariableByName("g_FogColor")->AsVector();
	//FogStart = m_Effect->GetVariableByName("g_FogStart")->AsScalar();
	//FogRange = m_Effect->GetVariableByName("g_FogRange")->AsScalar();
	//DirLights = m_Effect->GetVariableByName("g_DirLights");
	//Mat = m_Effect->GetVariableByName("g_Material");
	//DiffuseMap = m_Effect->GetVariableByName("g_DiffuseMap")->AsShaderResource();
}

SkyEffect::SkyEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("SkyRender");

	WorldViewProj = m_Effect->GetVariableByName("g_WorldViewProj")->AsMatrix();
	CubeMap = m_Effect->GetVariableByName("g_CubeMap")->AsShaderResource();
}

ShadowEffect::ShadowEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("BuildShadowMapTech");
	InsertTech("BuildShadowMapSkinTech");
	InsertTech("TessBuildShadowMapTech");
	InsertTech("BuildShadowMapNoSkinTech");
	

	WorldViewProj = m_Effect->GetVariableByName("g_WorldViewProj")->AsMatrix();
	TexTransform = m_Effect->GetVariableByName("g_TexTransform")->AsMatrix();
	World = m_Effect->GetVariableByName("g_World")->AsMatrix();
	DiffuseMap = m_Effect->GetVariableByName("g_TextureMap")->AsShaderResource();

	WorldInvTranspose = m_Effect->GetVariableByName("g_WorldInvTranspose")->AsMatrix();
	ViewProj = m_Effect->GetVariableByName("g_ViewProj")->AsMatrix();
	EyePosW = m_Effect->GetVariableByName("g_EyePosW")->AsVector();


	MinTessDistance = m_Effect->GetVariableByName("g_MinTessDistance")->AsScalar();
	MaxTessDistance = m_Effect->GetVariableByName("g_MaxTessDistance")->AsScalar();
	MinTessFactor = m_Effect->GetVariableByName("g_MinTessFactor")->AsScalar();
	MaxTessFactor = m_Effect->GetVariableByName("g_MaxTessFactor")->AsScalar();

	HeightScale = m_Effect->GetVariableByName("g_HeightScale")->AsScalar();

	DiffuseMap = m_Effect->GetVariableByName("g_DiffuseMap")->AsShaderResource();
	NormalSRV = m_Effect->GetVariableByName("g_NormalMap")->AsShaderResource();

	BoneTransforms = m_Effect->GetVariableByName("g_BoneTransform")->AsMatrix();
}



//NormalEffect::NormalEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
//{
//	InsertTech("DirLight");
//
//	WorldViewProj = m_Effect->GetVariableByName("g_WorldViewProj")->AsMatrix();
//
//	World = m_Effect->GetVariableByName("g_World")->AsMatrix();
//	WorldInvTranspose = m_Effect->GetVariableByName("g_WorldInvTranspose")->AsMatrix();
//	TexTransform = m_Effect->GetVariableByName("g_TexTransform")->AsMatrix();
//
//	EyePosW = m_Effect->GetVariableByName("g_EyePosW")->AsVector();
//
//	DirLights = m_Effect->GetVariableByName("g_DirLight");
//	PtLights = m_Effect->GetVariableByName("g_PtLight");
//	Mat = m_Effect->GetVariableByName("g_Mat");
//
//	DiffuseMap = m_Effect->GetVariableByName("g_TextureMap")->AsShaderResource();
//	ShadowMap = m_Effect->GetVariableByName("g_ShadowMap")->AsShaderResource();
//	NormalMap = m_Effect->GetVariableByName("g_NormalMap")->AsShaderResource();
//
//}

OutlineEffect::OutlineEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("SilhouetteEdge");

	ViewToTexSpace = m_Effect->GetVariableByName("g_ViewToTexSpace")->AsMatrix();
	OffsetVectors = m_Effect->GetVariableByName("g_OffsetVectors")->AsVector();
	FrustumCorners = m_Effect->GetVariableByName("g_FrustumCorners")->AsVector();
	NormalDepthMap = m_Effect->GetVariableByName("g_NormalDepthMap")->AsShaderResource();

}

NormalDepthEffect::NormalDepthEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("NormalDepth");

	WorldInvTransposeView = m_Effect->GetVariableByName("g_WorldInvTransposeView")->AsMatrix();
	WorldInvTranspose = m_Effect->GetVariableByName("g_WorldInvTranspose")->AsMatrix();
	WorldViewProj = m_Effect->GetVariableByName("g_WorldViewProj")->AsMatrix();
	World = m_Effect->GetVariableByName("g_World")->AsMatrix();
	WorldView = m_Effect->GetVariableByName("g_WorldView")->AsMatrix();
	TexTransform = m_Effect->GetVariableByName("g_TexTransform")->AsMatrix();
}

BasicBlurEffect::BasicBlurEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("HorzBlur");
	InsertTech("VertBlur");

	DepthSRV = m_Effect->GetVariableByName("g_DepthMap")->AsShaderResource();
	InputSRV = m_Effect->GetVariableByName("g_Input")->AsShaderResource();
	OutputUAV = m_Effect->GetVariableByName("g_Output")->AsUnorderedAccessView(); // Texture Ãâ·Â ÀÚ¿ø ºä Effect.
}

HDRDownScaleEffect::HDRDownScaleEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("DownScaleFirst");
	InsertTech("DownScaleSecond");
	InsertTech("BloomRevealFX");

	ResX = m_Effect->GetVariableByName("g_ResX")->AsScalar();
	ResY = m_Effect->GetVariableByName("g_ResY")->AsScalar();
	Domain = m_Effect->GetVariableByName("g_Domain")->AsScalar(); // Texture Ãâ·Â ÀÚ¿ø ºä Effect.
	GroupSize = m_Effect->GetVariableByName("g_GroupSize")->AsScalar(); // Texture Ãâ·Â ÀÚ¿ø ºä Effect.

	HDRInputSRV = m_Effect->GetVariableByName("g_HDRTex")->AsShaderResource();
	Avg1DSRV = m_Effect->GetVariableByName("g_AverageValues1D")->AsShaderResource();


	DownScale1DUAV = m_Effect->GetVariableByName("g_AverageLum")->AsUnorderedAccessView();
	Avg1DUAV = m_Effect->GetVariableByName("g_AverageLum")->AsUnorderedAccessView();

	DownScaleUAV = m_Effect->GetVariableByName("g_HDRDownScale")->AsUnorderedAccessView();

	// Bloom ¿ë //
	BloomThreshold = m_Effect->GetVariableByName("g_fBloomThreshold")->AsScalar(); // Bloom ThresHold
	Adaptation = m_Effect->GetVariableByName("g_fAdaptation")->AsScalar(); // Texture Ãâ·Â ÀÚ¿ø ºä Effect.

	InputAvgLumSRV = m_Effect->GetVariableByName("g_InputAvgLum")->AsShaderResource();
	InputDownScaleSRV = m_Effect->GetVariableByName("g_HDRDownScaleTex")->AsShaderResource();

	 OutputBloomUAV = m_Effect->GetVariableByName("g_OutBloom")->AsUnorderedAccessView();


}

HDRToneMappingEffect::HDRToneMappingEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("PostFXPass");

	WorldViewProj = m_Effect->GetVariableByName("g_WorldViewProj")->AsMatrix();
	TexTransform = m_Effect->GetVariableByName("g_TexTransform")->AsMatrix();
	World = m_Effect->GetVariableByName("g_World")->AsMatrix();
	InvProj = m_Effect->GetVariableByName("g_ProjInv")->AsMatrix();


    MiddleGray = m_Effect->GetVariableByName("g_MiddleGrey")->AsScalar();
	BloomScale = m_Effect->GetVariableByName("g_BloomScale")->AsScalar();
	WhiteSqrt = m_Effect->GetVariableByName("g_LumWhiteSqr")->AsScalar();
	ProjA = m_Effect->GetVariableByName("g_ProjA")->AsScalar();
	ProjB = m_Effect->GetVariableByName("g_ProjB")->AsScalar();

	HDRTex = m_Effect->GetVariableByName("g_HDRTex")->AsShaderResource();
	AvgLumSRV = m_Effect->GetVariableByName("g_AvgLum")->AsShaderResource();
	BloomSRV = m_Effect->GetVariableByName("g_BloomTex")->AsShaderResource();
}

DebuggingEffect::DebuggingEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("ViewArgbTech");
	InsertTech("ViewRedTech");
	InsertTech("ViewGreenTech");
	InsertTech("ViewBlueTech");
	InsertTech("ViewAlphaTech");

	WorldViewProj = m_Effect->GetVariableByName("g_WorldViewProj")->AsMatrix();
	DiffuseMap = m_Effect->GetVariableByName("g_Texture")->AsShaderResource();
}

//BasicBlur2Effect::BasicBlur2Effect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
//{
//	InsertTech("HorzBlur");
//	InsertTech("VertBlur");
//
//	ResX = m_Effect->GetVariableByName("ResX")->AsScalar();
//	ResY = m_Effect->GetVariableByName("ResY")->AsScalar();
//
//	InputSRV = m_Effect->GetVariableByName("g_Input")->AsShaderResource();
//	OutputUAV = m_Effect->GetVariableByName("g_Output")->AsUnorderedAccessView(); // Texture Ãâ·Â ÀÚ¿ø ºä Effect.
//}

DissolveEffect::DissolveEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("BasicDissolve");

	DissolveColorMap = m_Effect->GetVariableByName("g_DissolveColorTex")->AsShaderResource();
	DissolveMap = m_Effect->GetVariableByName("g_DissolveTex")->AsShaderResource();
	DiffuseMap = m_Effect->GetVariableByName("g_DiffuseTex")->AsShaderResource();

	WorldViewProj = m_Effect->GetVariableByName("g_WorldViewProj")->AsMatrix();
	TexTransform = m_Effect->GetVariableByName("g_TexTransform")->AsMatrix();

	Edge = m_Effect->GetVariableByName("g_Edge")->AsScalar();
	EdgeRange = m_Effect->GetVariableByName("g_EdgeRange")->AsScalar();
	Progress = m_Effect->GetVariableByName("g_Progress")->AsScalar();
}

GBufferPackEffect::GBufferPackEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("GbufferBasic");
	InsertTech("GBufferGlow");
	InsertTech("GbufferSkin");
	InsertTech("GbufferPOM");
	InsertTech("GbufferBasicNormal");
	InsertTech("GbufferSkinNormal");

	World = m_Effect->GetVariableByName("g_World")->AsMatrix();
	WorldViewProj = m_Effect->GetVariableByName("g_WorldViewProj")->AsMatrix();
	WorldView = m_Effect->GetVariableByName("g_WorldView")->AsMatrix();
	View = m_Effect->GetVariableByName("g_View")->AsMatrix();
	WorldInvTranspose = m_Effect->GetVariableByName("g_WorldInvTranspose")->AsMatrix();
	//WorldInvTransposeView = m_Effect->GetVariableByName("g_WorldInvTransposeView")->AsMatrix();
	BoneTransforms = m_Effect->GetVariableByName("g_BoneTransform")->AsMatrix();
	ViewProj = m_Effect->GetVariableByName("g_ViewProj")->AsMatrix();
	ShadowTransform = m_Effect->GetVariableByName("g_ShadowTransform")->AsMatrix();
//	TexTransform = m_Effect->GetVariableByName("g_TexTransform")->AsMatrix();

	//WorldViewProjTex = m_Effect->GetVariableByName("g_WorldViewProjTex")->AsMatrix();

	EyePosW = m_Effect->GetVariableByName("g_EyePosW")->AsVector();

	DiffuseMap = m_Effect->GetVariableByName("g_DiffuseMap")->AsShaderResource();
	HeightMap = m_Effect->GetVariableByName("g_HeightMap")->AsShaderResource();
	ShadowMap = m_Effect->GetVariableByName("g_ShadowMap")->AsShaderResource();
	//OutLineMap = m_Effect->GetVariableByName("g_OutLineMap")->AsShaderResource();
	NormalMap = m_Effect->GetVariableByName("g_NormalMap")->AsShaderResource();

	isBumped = m_Effect->GetVariableByName("g_isBumped")->AsScalar();
	isSpec = m_Effect->GetVariableByName("g_isSpec")->AsScalar();
	isMask = m_Effect->GetVariableByName("g_isMask")->AsScalar();
	isPOM = m_Effect->GetVariableByName("g_isPOM")->AsScalar();
	isSSR = m_Effect->GetVariableByName("g_isSSR")->AsScalar();
	isShadow = m_Effect->GetVariableByName("g_isShadow")->AsScalar();

	SpecMap = m_Effect->GetVariableByName("g_SpecularMap")->AsShaderResource();
	MaskMap = m_Effect->GetVariableByName("g_MaskMap")->AsShaderResource();

	//DissolveColorMap = m_Effect->GetVariableByName("g_DissolveColorTex")->AsShaderResource();
	//DissolveMap = m_Effect->GetVariableByName("g_DissolveTex")->AsShaderResource();


	SpecExp = m_Effect->GetVariableByName("g_SpecExp")->AsScalar();
	SpecIntensity = m_Effect->GetVariableByName("g_SpecInt")->AsScalar();
	//Progress = m_Effect->GetVariableByName("g_Progress")->AsScalar();
	CubeMap = m_Effect->GetVariableByName("g_CubeMap")->AsShaderResource();
	HeightScale = m_Effect->GetVariableByName("g_HeightScale")->AsScalar();
	//CubeMap = m_Effect->GetVariableByName("g_CubeMap")->AsShaderResource();
	//ShadowMap = m_Effect->GetVariableByName("g_ShadowMap")->AsShaderResource();
	//SsaoMap = m_Effect->GetVariableByName("g_SsaoMap")->AsShaderResource();*/
}

DeferredShadingEffect::DeferredShadingEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	
	InsertTech("DeferredDirLight");
	InsertTech("DeferredDiffuse");
	InsertTech("DeferredDepth");
	InsertTech("DeferredNormal");
	InsertTech("DeferredDirShadow");
	InsertTech("DeferredDirSSAO");

	DepthMap = m_Effect->GetVariableByName("g_DepthMap")->AsShaderResource();
	DiffuseSpecIntMap = m_Effect->GetVariableByName("g_DiffuseSpecIntMap")->AsShaderResource();
	NormalMap = m_Effect->GetVariableByName("g_NormalMap")->AsShaderResource();
	SpecPowMap = m_Effect->GetVariableByName("g_SpecPowMap")->AsShaderResource();
	ShadowMap = m_Effect->GetVariableByName("g_ShadowMap")->AsShaderResource();
	SSAOMap = m_Effect->GetVariableByName("g_SSAOMap")->AsShaderResource();

	PointLights = m_Effect->GetVariableByName("g_PtLight");
	DirLights = m_Effect->GetVariableByName("g_DirLight");
	EyePosW = m_Effect->GetVariableByName("g_EyePosW")->AsVector();
	ProjMatProperty = m_Effect->GetVariableByName("g_ProjMatProperty")->AsVector();
	FrustumCorners = m_Effect->GetVariableByName("g_FrustumCorners")->AsVector();

	InvProj = m_Effect->GetVariableByName("g_InvProj")->AsMatrix();
	InvView = m_Effect->GetVariableByName("g_InvView")->AsMatrix();
	InvViewProj = m_Effect->GetVariableByName("g_InvViewProj")->AsMatrix();
	WorldViewProj = m_Effect->GetVariableByName("g_WorldViewProj")->AsMatrix();
	ProjTex = m_Effect->GetVariableByName("g_ProjTex")->AsMatrix();

	ShadowTransform = m_Effect->GetVariableByName("g_ShadowTransform")->AsMatrix();


}

DeferredShadingPointLightEffect::DeferredShadingPointLightEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("DeferredPtLight");
	InsertTech("DeferredDiffuse");
	InsertTech("DeferredDepth");
	InsertTech("DeferredNormal");
	//	InsertTech("DeferredSpecPow");

	DepthMap = m_Effect->GetVariableByName("g_DepthMap")->AsShaderResource();
	DiffuseSpecIntMap = m_Effect->GetVariableByName("g_DiffuseSpecIntMap")->AsShaderResource();
	NormalMap = m_Effect->GetVariableByName("g_NormalMap")->AsShaderResource();
	SpecPowMap = m_Effect->GetVariableByName("g_SpecPowMap")->AsShaderResource();


	PointLights = m_Effect->GetVariableByName("g_PtLight");
	EyePosW = m_Effect->GetVariableByName("g_EyePosW")->AsVector();
	ProjMatProperty = m_Effect->GetVariableByName("g_ProjMatProperty")->AsVector();
	FrustumCorners = m_Effect->GetVariableByName("g_FrustumCorners")->AsVector();


	InvView = m_Effect->GetVariableByName("g_InvView")->AsMatrix();
	InvViewProj = m_Effect->GetVariableByName("g_InvVP")->AsMatrix();
	WorldViewProj = m_Effect->GetVariableByName("g_WorldViewProj")->AsMatrix();
	LightProjection = m_Effect->GetVariableByName("g_LightProjection")->AsMatrix();

}

SSAOEffect::SSAOEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("Ssao");
	//	InsertTech("DeferredSpecPow");

	DepthMap = m_Effect->GetVariableByName("g_DepthMap")->AsShaderResource();
	NormalMap = m_Effect->GetVariableByName("g_NormalMap")->AsShaderResource();
	RandomVectorMap = m_Effect->GetVariableByName("g_RandomVecMap")->AsShaderResource();

	OffsetVectors = m_Effect->GetVariableByName("g_OffsetPoint")->AsVector();

	ProjMatProperty = m_Effect->GetVariableByName("g_ProjectProperty")->AsVector();
	FrustumCorners = m_Effect->GetVariableByName("g_FrustumCorners")->AsVector();

	View = m_Effect->GetVariableByName("g_View")->AsMatrix();
	InvProj = m_Effect->GetVariableByName("g_InvProj")->AsMatrix();
	ViewToTexSpace = m_Effect->GetVariableByName("g_ViewToTex")->AsMatrix();

	SSAORadius = m_Effect->GetVariableByName("g_OcclusionRadius")->AsScalar();
	SSAOIntensity = m_Effect->GetVariableByName("g_OcclusionIntensity")->AsScalar();



}

SSAOEffectSecond::SSAOEffectSecond(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("SsaoSecond");
	//	InsertTech("DeferredSpecPow");

	DepthMap = m_Effect->GetVariableByName("g_DepthMap")->AsShaderResource();
	NormalMap = m_Effect->GetVariableByName("g_NormalMap")->AsShaderResource();
	RandomVectorMap = m_Effect->GetVariableByName("g_RandomVecMap")->AsShaderResource();

	//OffsetVectors = m_Effect->GetVariableByName("g_OffsetPoint")->AsVector();

	//ProjMatProperty = m_Effect->GetVariableByName("g_ProjectProperty")->AsVector();
	//FrustumCorners = m_Effect->GetVariableByName("g_FrustumCorners")->AsVector();

	View = m_Effect->GetVariableByName("g_View")->AsMatrix();
	//InvView = m_Effect->GetVariableByName("g_InvView")->AsMatrix();
	InvProj = m_Effect->GetVariableByName("g_InvProj")->AsMatrix();
	//ViewToTexSpace = m_Effect->GetVariableByName("g_ViewToTex")->AsMatrix();
}

SSAOBlurEffect::SSAOBlurEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("EdgeBlurHorz");
	InsertTech("EdgeBlurVert");

	DepthMap = m_Effect->GetVariableByName("g_DepthMap")->AsShaderResource();
	NormalMap = m_Effect->GetVariableByName("g_NormalMap")->AsShaderResource();
	InputMap = m_Effect->GetVariableByName("g_InputMap")->AsShaderResource();

	View = m_Effect->GetVariableByName("g_View")->AsMatrix();

	TexWidth = m_Effect->GetVariableByName("g_TexelWidth")->AsScalar();
	TexHeight = m_Effect->GetVariableByName("g_TexelHeight")->AsScalar();

	VectorAB = m_Effect->GetVariableByName("g_ProjAB")->AsVector();

}

SSAOBlurCSEffect::SSAOBlurCSEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("HorzBlur");
	InsertTech("VertBlur");

	NormalSRV = m_Effect->GetVariableByName("g_NormalMap")->AsShaderResource();
	DepthSRV = m_Effect->GetVariableByName("g_DepthMap")->AsShaderResource();
	InputSRV = m_Effect->GetVariableByName("g_InputMap")->AsShaderResource();
	OutputUAV = m_Effect->GetVariableByName("g_OutputMap")->AsUnorderedAccessView(); // Texture Ãâ·Â ÀÚ¿ø ºä Effect.

	VectorAB = m_Effect->GetVariableByName("ProjAB")->AsVector();
}

TileBasedDeferredEffect::TileBasedDeferredEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname), LightCount(0)
{

	InsertTech("TileTest");
	InsertTech("TileSSAO");
	InsertTech("Depth");
	InsertTech("Normal");
	InsertTech("Diffuse");
	InsertTech("Spec");
	

	DepthMap = m_Effect->GetVariableByName("g_DepthMap")->AsShaderResource();
	DiffuseSpecIntMap = m_Effect->GetVariableByName("g_DiffuseSpecMap")->AsShaderResource();
	NormalMap = m_Effect->GetVariableByName("g_NormalMap")->AsShaderResource();
	SpecPowMap = m_Effect->GetVariableByName("g_SpecPowMap")->AsShaderResource();
	ShadowMap = m_Effect->GetVariableByName("g_ShadowMap")->AsShaderResource();
	SSAOMap = m_Effect->GetVariableByName("g_SSAOMap")->AsShaderResource();
	LightSRV = m_Effect->GetVariableByName("g_LightList")->AsShaderResource();
	DirLightSRV = m_Effect->GetVariableByName("g_DirectLight")->AsShaderResource();
	OutputUAV = m_Effect->GetVariableByName("g_OutputMap")->AsUnorderedAccessView();
	CubeMap = m_Effect->GetVariableByName("g_ShadowCubeMap")->AsShaderResource();
	//PointLights = m_Effect->GetVariableByName("g_PtLight");
	//DirLights = m_Effect->GetVariableByName("g_DirLight");
	EyePosW = m_Effect->GetVariableByName("g_EyePosW")->AsVector();

	Proj = m_Effect->GetVariableByName("g_Proj")->AsMatrix();
	InvProj = m_Effect->GetVariableByName("g_InvProj")->AsMatrix();
	InvView = m_Effect->GetVariableByName("g_InvView")->AsMatrix();
	InvViewProj = m_Effect->GetVariableByName("g_InvViewProj")->AsMatrix();
	View = m_Effect->GetVariableByName("g_View")->AsMatrix();
	ViewProj = m_Effect->GetVariableByName("g_ViewProj")->AsMatrix();
	WorldViewProj = m_Effect->GetVariableByName("g_WorldViewProj")->AsMatrix();
	ShadowTransform = m_Effect->GetVariableByName("g_ShadowTransform")->AsMatrix();

	ProjMatProperty = m_Effect->GetVariableByName("g_PerspectiveValues")->AsVector();

	
	LightCount = m_Effect->GetVariableByName("g_lightCount")->AsScalar();
	isNormal = m_Effect->GetVariableByName("g_isNormal")->AsScalar();
	isSSAO = m_Effect->GetVariableByName("g_isSSAO")->AsScalar();
	isTileBase = m_Effect->GetVariableByName("g_isTileBase")->AsScalar();
	isDepth = m_Effect->GetVariableByName("g_isDepth")->AsScalar();
	isDiffuse = m_Effect->GetVariableByName("g_isDiffuse")->AsScalar();

}

GlowGeoEffect::GlowGeoEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("GlowBasic");

	EyePos = m_Effect->GetVariableByName("g_EyePosW")->AsVector();
	Size = m_Effect->GetVariableByName("g_Size")->AsVector();
	Color = m_Effect->GetVariableByName("g_Color")->AsVector();

	ViewProj = m_Effect->GetVariableByName("g_ViewProj")->AsMatrix();
}

HazeHeatEffect::HazeHeatEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("HazeBasic");

	NoiseMap = m_Effect->GetVariableByName("g_NoiseMap")->AsShaderResource();
	DiffuseMap = m_Effect->GetVariableByName("g_TextureMap")->AsShaderResource();

	EyePos = m_Effect->GetVariableByName("g_EyePosW")->AsVector();
	Size = m_Effect->GetVariableByName("g_Size")->AsVector();
	Color = m_Effect->GetVariableByName("g_Color")->AsVector();

	ViewProj = m_Effect->GetVariableByName("g_ViewProj")->AsMatrix();

	Time = m_Effect->GetVariableByName("g_Time")->AsScalar();
}

SpriteAnimationEffect::SpriteAnimationEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("SpriteBasic");

	TexArray = m_Effect->GetVariableByName("g_TexArray")->AsShaderResource();
	NoiseSRV = m_Effect->GetVariableByName("g_NoiseMap")->AsShaderResource();

	EyePos = m_Effect->GetVariableByName("g_EyePosW")->AsVector();
	Size = m_Effect->GetVariableByName("g_Size")->AsVector();
	

	ViewProj = m_Effect->GetVariableByName("g_ViewProj")->AsMatrix();

	Index = m_Effect->GetVariableByName("g_Index")->AsScalar();
	Time = m_Effect->GetVariableByName("g_Time")->AsScalar();
}

POMEffect::POMEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("POMBasic");

	DiffuseMap = m_Effect->GetVariableByName("g_DiffuseMap")->AsShaderResource();
	SpecularMap = m_Effect->GetVariableByName("g_SpecularMap")->AsShaderResource();
	HeightMap = m_Effect->GetVariableByName("g_HeightMap")->AsShaderResource();

	EyePosW = m_Effect->GetVariableByName("g_EyePosW")->AsVector();
	
	WorldViewProj = m_Effect->GetVariableByName("g_WorldViewProj")->AsMatrix();
	World = m_Effect->GetVariableByName("g_World")->AsMatrix();

}

SSREffect::SSREffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("SSRTest");
	
	 DepthMap = m_Effect->GetVariableByName("g_DepthMap")->AsShaderResource();
	 NormalMap = m_Effect->GetVariableByName("g_NormalMap")->AsShaderResource();
	 DiffuseMap = m_Effect->GetVariableByName("g_DiffuseMap")->AsShaderResource();
	 SpecMap = m_Effect->GetVariableByName("g_SpecMap")->AsShaderResource();

	 OutputUAV = m_Effect->GetVariableByName("g_OutputMap")->AsUnorderedAccessView();
	
	 Property = m_Effect->GetVariableByName("g_PerspectiveValues")->AsVector();
     EyePos = m_Effect->GetVariableByName("g_EyePosWS")->AsVector();

	 Proj = m_Effect->GetVariableByName("g_Proj")->AsMatrix();
	 View = m_Effect->GetVariableByName("g_View")->AsMatrix();
	 InvProj = m_Effect->GetVariableByName("g_InvProj")->AsMatrix();
	 ViewProj = m_Effect->GetVariableByName("g_ViewProj")->AsMatrix();
	TexToWorld = m_Effect->GetVariableByName("g_TextureToWorld")->AsMatrix();
	World = m_Effect->GetVariableByName("g_World")->AsMatrix();
	InvViewProj = m_Effect->GetVariableByName("g_InvViewProj")->AsMatrix();
	InvView = m_Effect->GetVariableByName("g_InvView")->AsMatrix();
	SSRPower = m_Effect->GetVariableByName("g_ReflectPower")->AsScalar();
}

SSREffect2::SSREffect2(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("SSRTest2");
	InsertTech("SSRTest3");

	DepthMap = m_Effect->GetVariableByName("g_DepthMap")->AsShaderResource();
	NormalMap = m_Effect->GetVariableByName("g_NormalMap")->AsShaderResource();
	DiffuseMap = m_Effect->GetVariableByName("g_DiffuseMap")->AsShaderResource();
	SpecMap = m_Effect->GetVariableByName("g_SpecMap")->AsShaderResource();

	OutputUAV = m_Effect->GetVariableByName("g_OutputMap")->AsUnorderedAccessView();


	EyePos = m_Effect->GetVariableByName("g_EyePosWS")->AsVector();
	PerspectiveValue = m_Effect->GetVariableByName("g_PerspectiveValues")->AsVector();

	Proj = m_Effect->GetVariableByName("g_Proj")->AsMatrix();
	View = m_Effect->GetVariableByName("g_View")->AsMatrix();
	InvProj = m_Effect->GetVariableByName("g_InvProj")->AsMatrix();
	ViewProj = m_Effect->GetVariableByName("g_ViewProj")->AsMatrix();
	TexToWorld = m_Effect->GetVariableByName("g_TextureToWorld")->AsMatrix();
	World = m_Effect->GetVariableByName("g_World")->AsMatrix();
	InvViewProj = m_Effect->GetVariableByName("g_InvViewProj")->AsMatrix();
	InvView = m_Effect->GetVariableByName("g_InvView")->AsMatrix();
	WorldViewProj = m_Effect->GetVariableByName("g_WorldViewProj")->AsMatrix();
	WorldView = m_Effect->GetVariableByName("g_WorldView")->AsMatrix();
	World = m_Effect->GetVariableByName("g_World")->AsMatrix();

	ViewAngleThreshold = m_Effect->GetVariableByName("g_ViewAngleThreshold")->AsScalar();
	EdgeDistThreshold = m_Effect->GetVariableByName("g_EdgeDistThreshold")->AsScalar();
	DepthBias = m_Effect->GetVariableByName("g_DepthBias")->AsScalar();
	ReflectiveValue = m_Effect->GetVariableByName("g_ReflectionScale")->AsScalar();

	 
}

ShadowCubeMapEffect::ShadowCubeMapEffect(ID3D11Device * device, LPCWSTR fxname) : Effect(device, fxname)
{
	InsertTech("ShadowCubeMap");
	InsertTech("ShadowAniCubeMap");

	ShadowGenMat = m_Effect->GetVariableByName("g_ShadowGenMat")->AsMatrix();
	WorldViewProj = m_Effect->GetVariableByName("g_WorldViewProj")->AsMatrix();
	BoneTransforms = m_Effect->GetVariableByName("g_BoneTransform")->AsMatrix();
}
