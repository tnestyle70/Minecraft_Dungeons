#include "pch.h"
#include "CCamp.h"
#include "CMonster.h"
#include "CPlayer.h"
#include "CMonsterAnim.h"
#include "CBlockMgr.h"
#include "CDynamicCamera.h"
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

	CBlockMgr::GetInstance()->Update(fTimeDelta);

	if (GetAsyncKeyState(VK_RETURN))
	{
		if (FAILED(CSceneChanger::ChangeScene(m_pGraphicDev, eSceneType::SCENE_REDSTONE)))
		{
			MSG_BOX("RedStone Create Failed");
			return -1;
		}
		return iExit;
	}

	return iExit;
}

void CCamp::LateUpdate_Scene(const _float& fTimeDelta)
{
	CScene::LateUpdate_Scene(fTimeDelta);
}

void CCamp::Render_Scene()
{
	CBlockMgr::GetInstance()->Render();
}

HRESULT CCamp::Ready_Environment_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();

	if (!pLayer)
		return E_FAIL;

	CGameObject* pGameObject = nullptr;

	//dynamic camera
	_vec3 vEye{ 0.f, 10.f, -10.f };
	_vec3 vAt{ 0.f, 0.f, 1.f };
	_vec3 vUp{ 0.f, 1.f, 0.f };

	pGameObject = CDynamicCamera::Create(m_pGraphicDev, &vEye, &vAt, &vUp);

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"DynamicCamera", pGameObject)))
		return E_FAIL;

	//SkyBox 추가
	
	//BlockMgr
	if (FAILED(CBlockMgr::GetInstance()->Ready_BlockMgr(m_pGraphicDev)))
	{
		MSG_BOX("block mgr create failed");
		return E_FAIL;
	}

	CBlockMgr::GetInstance()->LoadBlocks(L"../Bin/Data/Stage3.dat");

	CBlockMgr::GetInstance()->SetEditorMode(false);

	m_mapLayer.insert({ pLayerTag, pLayer });

	return S_OK;
}

HRESULT CCamp::Ready_GameLogic_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();

	if (!pLayer)
		return E_FAIL;

	CGameObject* pGameObject = nullptr;

	//Player
	pGameObject = CPlayer::Create(m_pGraphicDev);

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"Player", pGameObject)))
		return E_FAIL;

	//Monster
	pGameObject = CMonster::Create(m_pGraphicDev, EMonsterType::ZOMBIE);

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"Monster", pGameObject)))
		return E_FAIL;
	//멀티맵이라 이름 같아도 가능, 그냥 맵은 안 됨
	pGameObject = CMonster::Create(m_pGraphicDev, EMonsterType::SKELETON);

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"Monster", pGameObject)))
		return E_FAIL;

	m_mapLayer.insert({ pLayerTag, pLayer });

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
