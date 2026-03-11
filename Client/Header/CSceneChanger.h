#pragma once
#include "Engine_Define.h"

enum eSceneType
{
	SCENE_LOGO,
	//로딩 씬 진입
	SCENE_STAGE,
	SCENE_SQUIDCOAST,
	SCENE_CAMP,
	SCENE_REDSTONE,
	SCENE_OBSIDIAN,

	//실제 게임 씬
	SCENE_STAGE_PLAY,
	SCENE_SQUIDCOAST_PLAY,
	SCENE_REDSTONE_PLAY,
	SCENE_OBSIDIAN_PLAY,
	SCENE_CAMP_PLAY,

	SCENE_END
};

class CSceneChanger
{
public:
	static HRESULT ChangeScene(LPDIRECT3DDEVICE9 pGraphicDev,
		eSceneType eType);
};