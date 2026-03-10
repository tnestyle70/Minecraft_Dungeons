#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CMonsterBody.h"

class CMonster : public CGameObject
{
private:
    explicit CMonster(LPDIRECT3DDEVICE9 pGraphicDev);
    explicit CMonster(const CGameObject& rhs);
    virtual ~CMonster();

public:
    virtual     HRESULT     Ready_GameObject();
    virtual     _int        Update_GameObject(const _float& fTimeDelta);
    virtual     void        LateUpdate_GameObject(const _float& fTimeDelta);
    virtual     void        Render_GameObject();

private:
    HRESULT     Add_Component();

private:
    CMonsterBody* m_pBodyCom = nullptr;
    Engine::CTransform* m_pTransformCom = nullptr;
    Engine::CTexture* m_pTextureCom = nullptr;

    EMonsterType    m_eType = EMonsterType::ZOMBIE;
    bool            m_bIsMoving = true;  // 테스트용, 추후 이동 여부로 교체

public:
   
    static CMonster* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        EMonsterType eType = EMonsterType::ZOMBIE);

private:
    virtual void Free();
};