#include "pch.h"
#include "CLoadingScene.h"
#include "CFontMgr.h"
#include "CRenderer.h"

CLoadingScene::CLoadingScene(LPDIRECT3DDEVICE9 pGraphicDev)
	:CScene(pGraphicDev)
{
}

CLoadingScene::~CLoadingScene()
{
}

HRESULT CLoadingScene::Ready_Scene()
{
	//Initialize Camera!!
	_matrix matView, matProj;
	D3DXMatrixIdentity(&matView);
	D3DXMatrixIdentity(&matProj);
	m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matProj);
	m_pGraphicDev->SetTransform(D3DTS_VIEW, &matView);

	//로딩 텍스쳐
	m_pLoadingTexture = CBackGround::Create(m_pGraphicDev, m_pTextureName);
	if (!m_pLoadingTexture)
		return E_FAIL;

	//백그라운드 로딩 쓰레드 돌리기
	m_pLoading = CLoading::Create(m_pGraphicDev, m_eLoadingID);
	if (!m_pLoading)
		return E_FAIL;

	return S_OK;
}

_int CLoadingScene::Update_Scene(const _float& fTimeDelta)
{
	CRenderer::GetInstance()->Add_RenderGroup(RENDER_UI, m_pLoadingTexture);

	if (m_bRenderOnce)
	{
		m_fDisplayTimer += fTimeDelta;
	}

	if (m_bRenderOnce &&  !m_bSceneChanged && m_pLoading->Get_Finish() && m_fDisplayTimer >= m_fMinDisplayTime)
	{
		m_bSceneChanged = true;

		//Render Group Clear Before Change Scene!!!!
		CRenderer::GetInstance()->Clear_RenderGroup();

		if (FAILED(CSceneChanger::ChangeScene(m_pGraphicDev, m_eNextScene)))
		{
			MSG_BOX("CLoadingScene: Scene Change Failed");
			return -1;
		}
	}

	return 0;
}

void CLoadingScene::LateUpdate_Scene(const _float& fTimeDelta)
{
}

void CLoadingScene::Render_Scene()
{	
	m_bRenderOnce = true;

	//로딩 진행 텍스트
	_vec2 vPos{ 0.f, 0.f };
	CFontMgr::GetInstance()->Render_Font(
		L"Font_Default", m_pLoading->Get_String(), &vPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
}

CLoadingScene* CLoadingScene::Create(LPDIRECT3DDEVICE9 pGraphicDev,
	CLoading::LOADINGID eLoadingID,
	eSceneType eNextScene,
	const _tchar* pLoadingTexture)
{
	CLoadingScene* pScene = new CLoadingScene(pGraphicDev);

	pScene->m_eLoadingID = eLoadingID;
	pScene->m_eNextScene = eNextScene;
	pScene->m_pTextureName = pLoadingTexture;

	if (FAILED(pScene->Ready_Scene()))
	{
		Safe_Release(pScene);
		MSG_BOX("CLoadingScene Create Failed");
		return nullptr;
	}

	return pScene;
}

void CLoadingScene::Free()
{
	Safe_Release(m_pLoading);
	Safe_Release(m_pLoadingTexture);
	CScene::Free();
}
