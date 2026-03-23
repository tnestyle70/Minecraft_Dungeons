#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CBlockPlacer.h"

class CBlock : public CGameObject
{
private:
	explicit CBlock(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CBlock(const CGameObject& rhs);
	virtual ~CBlock();

public:
	virtual			HRESULT		Ready_GameObject(const _vec3& vPos, eBlockType eType);
	virtual			_int		Update_GameObject(const _float& fTimeDelta);
	virtual			void		LateUpdate_GameObject(const _float& fTimeDelta);
	virtual			void		Render_GameObject();

	eBlockType GetBlockType() { return m_eType; }
	_vec3 Get_Pos() { return m_vPos; }

private:
	HRESULT			Add_Component();
	HRESULT			Set_Material();
	const _tchar* GetTextureName();
	_vec3 GetColliderSize(eBlockType eType);
	_vec3 GetColliderOffset(eBlockType eType);
private:
	Engine::CCubeTex* m_pBufferCom = nullptr;
	Engine::CTransform* m_pTransformCom = nullptr;
	Engine::CTexture* m_pTextureCom = nullptr;
	Engine::CCollider* m_pColliderCom = nullptr;

	_vec3 m_vPos = { 0.f, 0.f, 0.f };
	eBlockType m_eType = eBlockType::BLOCK_END;
public:
	static CBlock* Create(LPDIRECT3DDEVICE9 pGraphicDev,
		const _vec3& vPos, eBlockType eType);

private:
	virtual void Free();
};