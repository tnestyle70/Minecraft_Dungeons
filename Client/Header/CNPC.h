#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CPlayerBody.h"
#include "CCollider.h"
#include "CDialogueBox.h"

enum NPC_PART
{
    NPC_HEAD,
    NPC_BODY,
    NPC_LARM,
    NPC_RARM,
    NPC_LLEG,
    NPC_RLEG,
    NPC_PART_END
};

class CNPC : public CGameObject
{
private:
    explicit CNPC(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CNPC();

public:
    virtual HRESULT Ready_GameObject(_vec3 vPos);
    virtual _int    Update_GameObject(const _float& fTimeDelta);
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void    Render_GameObject();

private:
    HRESULT Add_Component();
    void    Render_Part(NPC_PART ePart, _float fAngleX, _float fAngleY, _float fAngleZ,
        const _matrix& matRootWorld);

public:
    void    Interact();
    bool    Is_Interactable(const _vec3& vPlayerPos, const _vec3& vPickPos);
    void    Set_DialogueBox(CDialogueBox* pBox) { m_pDialogueBox = pBox; }

public:
    static CNPC* Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos);
    Engine::CTransform* Get_Transform() { return m_pTransformCom; }
private:
    virtual void Free();

private:
    CPlayerBody* m_pBufferCom[NPC_PART_END] = {};
    Engine::CTransform* m_pTransformCom = nullptr;
    Engine::CTexture* m_pTextureCom = nullptr;
    Engine::CCollider* m_pColliderCom = nullptr;

    _matrix m_matPartWorld[NPC_PART_END];
    _vec3   m_vPartOffset[NPC_PART_END];
    _vec3   m_vPartScale[NPC_PART_END];

    bool    m_bTalking = false;
    CDialogueBox* m_pDialogueBox = nullptr;
};