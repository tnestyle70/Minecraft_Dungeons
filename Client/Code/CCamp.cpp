#include "pch.h"
#include "CCamp.h"
#include "CBackGround.h"
#include "CProtoMgr.h"
#include "CBlockMgr.h"
#include "CManagement.h"
#include "CDynamicCamera.h"
#include "CStage.h"
#include "CSceneChanger.h"

CCamp::CCamp(LPDIRECT3DDEVICE9 pGraphicDev)
	:CScene(pGraphicDev)
{
}

CCamp::~CCamp()
{
}

HRESULT CCamp::Ready_Scene()
{
	if (FAILED(Ready_Light()))
		return E_FAIL;

	if (FAILED(Ready_Environment_Layer(L"Environment_Layer")))
		return E_FAIL;

	if (FAILED(Ready_GameLogic_Layer(L"GameLogic_Layer")))
		return E_FAIL;

	if (FAILED(Ready_UI_Layer(L"UI_Layer")))
		return E_FAIL;

	return S_OK;
}

_int CCamp::Update_Scene(const _float& fTimeDelta)
{
	_int iExit = CScene::Update_Scene(fTimeDelta);

	if (GetAsyncKeyState('9'))
	{
		if (FAILED(CSceneChanger::ChangeScene(m_pGraphicDev, eSceneType::SCENE_STAGE)))
		{
			MSG_BOX("Stage Create Failed");
			return -1;
		}
	}

	return iExit;
}

void CCamp::LateUpdate_Scene(const _float& fTimeDelta)
{
	CScene::LateUpdate_Scene(fTimeDelta);
}

void CCamp::Render_Scene()
{
}

HRESULT CCamp::Ready_Environment_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();

	if (!pLayer)
		return E_FAIL;

	CGameObject* pGameObject = nullptr;

	pGameObject = CBackGround::Create(m_pGraphicDev,
		L"Proto_CampLoadingTexture");

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"BackGround", pGameObject)))
		return E_FAIL;

	m_mapLayer.insert({ pLayerTag, pLayer });

	return S_OK;
}

HRESULT CCamp::Ready_GameLogic_Layer(const _tchar* pLayerTag)
{
	return S_OK;
}

HRESULT CCamp::Ready_UI_Layer(const _tchar* pLayerTag)
{
	return S_OK;
}

HRESULT CCamp::Ready_Light()
{
	return S_OK;
}

CCamp* CCamp::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CCamp* pCamp = new CCamp(pGraphicDev);
	
	if (FAILED(pCamp->Ready_Scene()))
	{
		Safe_Release(pCamp);
		MSG_BOX("pCamp Create Failed");
		return nullptr;
	}

	return pCamp;
}

void CCamp::Free()
{
	CScene::Free();
}
