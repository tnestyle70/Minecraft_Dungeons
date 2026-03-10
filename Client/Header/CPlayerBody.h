#pragma once
#include "CVIBuffer.h"
#include "CCubeBodyTex.h"

class CPlayerBody : public Engine::CVIBuffer
{
private:
	explicit CPlayerBody(LPDIRECT3DDEVICE9 pGraphicDev, const FACE_UV* pFaceUVs);
	virtual ~CPlayerBody();

public:
	virtual HRESULT		Ready_Buffer();
	virtual void		Render_Buffer();

	// CVIBuffer 프로토타입 시스템 미사용 → Clone 미구현
	virtual Engine::CComponent* Clone() { return nullptr; }

private:
	FACE_UV		m_FaceUVs[6];

public:
	static CPlayerBody* Create(LPDIRECT3DDEVICE9 pGraphicDev, const FACE_UV* pFaceUVs);

private:
	virtual void	Free();
};