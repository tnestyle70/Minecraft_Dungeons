#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

//HUD 플레이어의 생성과 동기화를 시켜서 체력 정보를 업데이트하는 것이 핵심

class CPlayer;

class CHUD : public CGameObject
{
private:
	explicit CHUD(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CHUD(const CGameObject& rhs);
	virtual ~CHUD();

public:
	virtual			HRESULT		Ready_GameObject();
	virtual			_int		Update_GameObject(const _float& fTimeDelta);
	virtual			void		LateUpdate_GameObject(const _float& fTimeDelta);
	virtual			void		Render_GameObject();

public:
	void Set_Player(CPlayer* pPlayer) { m_pPlayer = pPlayer; };

private:
	HRESULT			Add_Component();
	void Render_BeginUI();
	void Render_EndUI();
private:
	Engine::CRcTex* m_pBufferCom = nullptr;
	Engine::CTexture* m_pTextureCom = nullptr;
	CTexture* m_pFilledHeart = nullptr;
	CTexture* m_pEmptyHeart = nullptr;

	CPlayer* m_pPlayer = nullptr;

	//Empty Heart 위치, 사이즈
	float m_fX, m_fY = 0.f;
	float m_fW, m_fH = 0.f;

	//플레이어 체력
	int m_iHP = 0;
	int m_iMaxHP = 0;

	//원본 행렬 저장
	_matrix m_matOriginView;
	_matrix m_matOriginProj;

public:
	static CHUD* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();

};

