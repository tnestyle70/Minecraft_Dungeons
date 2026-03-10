#include "pch.h"
#include "CSceneChanger.h"
#include "CManagement.h"
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
		pScene = CStage::Create(pGraphicDev);
		break;
	case SCENE_SQUIDCOAST:
		pScene = CSquidCoast::Create(pGraphicDev);
		break;
	case SCENE_CAMP:
		pScene = CCamp::Create(pGraphicDev);
		break;
	//case SCENE_REDSTONE:
	//	pScene = CRedStone::Create(pGraphicDev);
	//	break;
	//case SCENE_OBSIDIAN:
	//	pScene = CLogo::Create(pGraphicDev);
	//	break;
	default:
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
