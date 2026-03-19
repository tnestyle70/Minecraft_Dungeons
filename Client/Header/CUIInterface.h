#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CUIInterface : public CGameObject
{
protected:
	CUIInterface(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CUIInterface();
public:
	virtual HRESULT Ready_GameObject();
	virtual _int Update_GameObject(const _float& fTimeDelta);
	virtual void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void Render_GameObject();
protected:
	//마우스 상태 체크
	bool IsMouseInRect(); //마우스 호버링 체크
	_vec2 GetMousePos(); //마우스 위치 받아오기
	//UI 공통 렌더 -> Transform, View, Projection
	void BeginUIRender();
	void EndUIRender();
protected:
	virtual void Hover() {}; //호버
	virtual void Clicked() {}; //클릭
	virtual void Leave() {};
	
	bool IsHovered() const { return m_bHovered; }
	bool IsClicked() const { return m_bClicked; }

protected:
	//스크린 좌표, 크기
	float m_fX = 0.f, m_fY = 0.f;
	float m_fW = 0.f, m_fH = 0.f;
	
	bool m_bHovered = false;
	bool m_bClicked = false;
	bool m_bLeaved = false;

	//UI 복원용 matrix
	_matrix m_matOriginView, m_matOriginProj;
	
	CRcTex* m_pBufferCom = nullptr;
	CTexture* m_pTextureCom = nullptr;
	
protected:
	virtual void Free();
};