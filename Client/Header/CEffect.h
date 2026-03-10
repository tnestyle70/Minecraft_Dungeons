#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CEffect : public CGameObject
{
private:
	explicit CEffect(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CEffect(const CGameObject& rhs);
	virtual ~CEffect();
public:
	HRESULT Ready_GameObject() override;
	_int		Update_GameObject(const _float& fTimeDelta) override;
	void		LateUpdate_GameObject(const _float& fTimeDelta) override;
	void		Render_GameObject() override;
private:
	HRESULT Add_Component();
private:
	CRcTex* m_pRcTex = nullptr;
	CTransform* m_pTransCom = nullptr;
	CTexture* m_pTexture = nullptr;
	//이펙트 재생을 위한 프레임 설정
	_float m_fFrame = 0.f;
public:
	static CEffect* Create(LPDIRECT3DDEVICE9 pGraphicDev);
private:
	virtual void Free();

};