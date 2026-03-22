#pragma once
#include "CBase.h"
#include "CProtoMgr.h"

enum class eCursorState
{
	DEFAULT = 0, //기본 커서 상태
	CLICK,
	ENEMY, //적 콜라이더 충돌
	ENEMY_HOVER,
	CURSOR_END
};

class CCursorMgr : public CBase
{
	DECLARE_SINGLETON(CCursorMgr)
private:
	explicit CCursorMgr();
	virtual ~CCursorMgr();
public:
	HRESULT Ready_CursorMgr(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual _int Update(const _float& fTimeDelta);
	virtual void Render();
public:
	void SetCursorState(eCursorState eState) { m_eCursorState = eState; }
	eCursorState GetCursorState() { return m_eCursorState; }
	void GetPickingRay(_vec3& vRayOrigin, _vec3& vRayDir) const;
	bool IsClickedThisFrame() { return m_bClickedThisFrame; }
	bool IsClicked() { return m_bClicked; }
	const _vec2& Get_MousePos() const { return m_vMousePos; }
private:
	HRESULT AddComponent();
private:
	void BeginCursorRender();
	void EndCursorRender();
private:
	LPDIRECT3DDEVICE9 m_pGraphicDev = nullptr;
	eCursorState m_eCursorState = eCursorState::DEFAULT;
	//상태별 텍스쳐
	CRcTex* m_pBufferCom = nullptr;
	CTexture* m_pTextures[(_int)eCursorState::CURSOR_END] = {};
	//마우스 위치
	_vec2 m_vMousePos = { 0.f, 0.f };
	//클릭 감지
	bool m_bClicked = false;
	bool m_bClickedThisFrame = false;
	//렌더링 복원
	_matrix m_matOriginView, m_matOriginProj;
	static constexpr float m_fCursorSize = 64.f;
private:
	virtual void Free();
};