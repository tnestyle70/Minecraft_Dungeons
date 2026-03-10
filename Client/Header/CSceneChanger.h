#pragma once
#include "Engine_Define.h"

enum eSceneType
{
	SCENE_LOGO,
	SCENE_STAGE,
	SCENE_SQUIDCOAST,
	SCENE_CAMP,
	SCENE_REDSTONE,
	SCENE_OBSIDIAN
};

class CSceneChanger
{
public:
	static HRESULT ChangeScene(LPDIRECT3DDEVICE9 pGraphicDev,
		eSceneType eType);
};