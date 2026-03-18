#pragma once
#include "Engine_Define.h"

enum eSceneType
{
	SCENE_LOGO,
	//로딩 씬 진입
	SCENE_SQUIDCOAST,
	SCENE_CAMP,
	SCENE_REDSTONE,
	SCENE_OBSIDIAN,
	//실제 게임 씬
	SCENE_SQUIDCOAST_PLAY,
	SCENE_REDSTONE_PLAY,
	SCENE_OBSIDIAN_PLAY,
	SCENE_CAMP_PLAY,

	//오브젝트 에디터 씬
	SCENE_OBJECT_EDITOR,

	SCENE_END
};

class CSceneChanger
{
public:
	static HRESULT ChangeScene(LPDIRECT3DDEVICE9 pGraphicDev,
		eSceneType eType);
};