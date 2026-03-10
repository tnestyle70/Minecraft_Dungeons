#include "pch.h"
#include "CSquidCoast.h"
#include "CBackGround.h"
#include "CProtoMgr.h"
#include "CCamp.h"
#include "CBlockMgr.h"
#include "CDynamicCamera.h"
#include "CSceneChanger.h"

CSquidCoast::CSquidCoast(LPDIRECT3DDEVICE9 pGraphicDev)
	:CScene(pGraphicDev)
{
}

CSquidCoast::~CSquidCoast()
{
}

HRESULT CSquidCoast::Ready_Scene()
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

_int CSquidCoast::Update_Scene(const _float& fTimeDelta)
{
	_int iExit = CScene::Update_Scene(fTimeDelta);

	if (GetAsyncKeyState('0'))
	{
		if (FAILED(CSceneChanger::ChangeScene(m_pGraphicDev, eSceneType::SCENE_CAMP)))
		{
			MSG_BOX("Camp Create Failed");
			return -1;
		}
	}

	//CBlockMgr::GetInstance()->Update(fTimeDelta);

	return iExit;
}

void CSquidCoast::LateUpdate_Scene(const _float& fTimeDelta)
{
	CScene::LateUpdate_Scene(fTimeDelta);
}

void CSquidCoast::Render_Scene()
{
}

HRESULT CSquidCoast::Ready_Environment_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();

	if (!pLayer)
		return E_FAIL;

	CGameObject* pGameObject = nullptr;

	//loding screen
	pGameObject = CBackGround::Create(m_pGraphicDev, L"Proto_SquidCoastLoadingTexture");

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"BackGround", pGameObject)))
		return E_FAIL;

	m_mapLayer.insert({ pLayerTag, pLayer });

	//BlockMgr
	//if (FAILED(CBlockMgr::GetInstance()->Ready_BlockMgr(m_pGraphicDev)))
	//{
	//	MSG_BOX("BlockMgr Create Failed");
	//	return E_FAIL;
	//}
	//CBlockMgr::GetInstance()->LoadBlocks(L"../Bin/Data/Stage1.dat");

	return S_OK;
}

HRESULT CSquidCoast::Ready_GameLogic_Layer(const _tchar* pLayerTag)
{
	return S_OK;
}

HRESULT CSquidCoast::Ready_UI_Layer(const _tchar* pLayerTag)
{
	return S_OK;
}

HRESULT CSquidCoast::Ready_Light()
{
	return S_OK;
}

CSquidCoast* CSquidCoast::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CSquidCoast* pSquidCoast = new CSquidCoast(pGraphicDev);

	if (FAILED(pSquidCoast->Ready_Scene()))
	{
		Safe_Release(pSquidCoast);
		MSG_BOX("pSquidCoast Create Failed");
		return nullptr;
	}

	return pSquidCoast;
}

void CSquidCoast::Free()
{
	CScene::Free();
}
