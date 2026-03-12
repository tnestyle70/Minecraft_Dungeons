#pragma once
#include "CScene.h"
#include "CBackGround.h"
#include "CLoading.h"
#include "CSceneChanger.h"

class CLoadingScene : public CScene
{
protected:
	explicit CLoadingScene(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CLoadingScene();
public:
	virtual HRESULT Ready_Scene() override;
	virtual _int Update_Scene(const _float& fTimeDelta) override;
	virtual void LateUpdate_Scene(const _float& fTimeDelta) override;
	virtual void Render_Scene();
private:
	CLoading* m_pLoading = nullptr;
	CBackGround* m_pLoadingTexture = nullptr;

	CLoading::LOADINGID m_eLoadingID =  CLoading::LOADING_END;//로드할 스테이지
	eSceneType m_eNextScene = eSceneType::SCENE_END;
	const _tchar* m_pTextureName = nullptr;
	bool m_bSceneChanged = false;

	//로딩씬 체크
	_int m_iFrameCount = 0;
	bool m_bRenderOnce = false;
public:
	static CLoadingScene* Create(LPDIRECT3DDEVICE9 pGraphicDev,
		CLoading::LOADINGID eLoadingID,
		eSceneType eNextScene,
		const _tchar* pLoadingTexture);
private:
	virtual void Free();
};