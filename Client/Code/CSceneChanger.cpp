#include "pch.h"
#include "CSceneChanger.h"
#include "CManagement.h"
#include "CLoadingScene.h"
//stages
#include "CLogo.h"
#include "CStage.h"
#include "CCamp.h"
#include "CSquidCoast.h"
#include "CRedStone.h"
#include "CObsidian.h"

HRESULT CSceneChanger::ChangeScene(LPDIRECT3DDEVICE9 pGraphicDev, eSceneType eType)
{
	CScene* pScene = nullptr;

	switch (eType)
	{
	case SCENE_LOGO:
		pScene = CLogo::Create(pGraphicDev);
		break;
	case SCENE_STAGE:
		pScene = CLoadingScene::Create(
			pGraphicDev,
			CLoading::LOADING_STAGE,
			SCENE_STAGE_PLAY,
			L"Proto_ObsidianLoadingTexture");
		break;
	case SCENE_SQUIDCOAST:
		pScene = CLoadingScene::Create(
			pGraphicDev,
			CLoading::LOADIND_SQUIDCOAST,
			SCENE_SQUIDCOAST_PLAY,
			L"Proto_SquidCoastLoadingTexture");
		break;
	case SCENE_CAMP:
		pScene = CLoadingScene::Create(
			pGraphicDev,
			CLoading::LOADING_CAMP,
			SCENE_CAMP_PLAY,
			L"Proto_CampLoadingTexture");
		break;
	case SCENE_REDSTONE:
		pScene = CLoadingScene::Create(
			pGraphicDev,
			CLoading::LOADING_REDSTONE,
			SCENE_REDSTONE_PLAY,
			L"Proto_RedStoneLoadingTexture");
		break;
	case SCENE_OBSIDIAN:
		pScene = CLoadingScene::Create(
			pGraphicDev,
			CLoading::LOADING_OBSIDIAN,
			SCENE_OBSIDIAN_PLAY,
			L"Proto_ObsidianLoadingTexture");
		break;
	case SCENE_STAGE_PLAY:
		pScene = CStage::Create(pGraphicDev);
		break;
	case SCENE_SQUIDCOAST_PLAY:
		pScene = CSquidCoast::Create(pGraphicDev);
		break;
	case SCENE_CAMP_PLAY:
		pScene = CCamp::Create(pGraphicDev);
		break;
	case SCENE_REDSTONE_PLAY:
		pScene = CRedStone::Create(pGraphicDev);
		break;
	case SCENE_OBSIDIAN_PLAY:
		pScene = CObsidian::Create(pGraphicDev);
		break;	
	}

	if (!pScene)
		return E_FAIL;

	if (FAILED(CManagement::GetInstance()->Set_Scene(pScene)))
	{
		MSG_BOX("Stage Create Failed");
		return E_FAIL;
	}

	return S_OK;
}
