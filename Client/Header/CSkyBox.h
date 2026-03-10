#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CSkyBox : public CGameObject
{
private:
	explicit CSkyBox(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CSkyBox(const CGameObject& rhs);
	virtual ~CSkyBox();

public:
	virtual			HRESULT		Ready_GameObject();
	virtual			_int		Update_GameObject(const _float& fTimeDelta);
	virtual			void		LateUpdate_GameObject(const _float& fTimeDelta);
	virtual			void		Render_GameObject();

private:
	HRESULT			Add_Component();

private:
	Engine::CCubeTex* m_pBufferCom;
	Engine::CTransform* m_pTransformCom;
	Engine::CTexture* m_pTextureCom;

public:
	static CSkyBox* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();

};

// 1. 스카이박스 출력 하기
// 2. 뷰 스페이스 변환 행렬, 원근 투영 행렬을 직접 구현하여 적용해라
