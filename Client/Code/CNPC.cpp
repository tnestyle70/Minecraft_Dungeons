#include "pch.h"
#include "CNPC.h"
#include "CRenderer.h"
#include "CManagement.h"
#include "CBlockMgr.h"
#include "CCollider.h"
#include "CProtoMgr.h"

CNPC::CNPC(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
    , m_pTransformCom(nullptr)
    , m_pTextureCom(nullptr)
    , m_pColliderCom(nullptr)
{
    ZeroMemory(m_pBufferCom, sizeof(m_pBufferCom));
    ZeroMemory(m_matPartWorld, sizeof(m_matPartWorld));
}

CNPC::~CNPC()
{
}

HRESULT CNPC::Ready_GameObject(_vec3 vPos)
{
    if (FAILED(Add_Component()))
        return E_FAIL;

    m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);

    m_vPartScale[NPC_HEAD] = { 0.40f, 0.40f, 0.40f };
    m_vPartScale[NPC_BODY] = { 0.50f, 0.50f, 0.25f };
    m_vPartScale[NPC_LARM] = { 0.20f, 0.60f, 0.20f };
    m_vPartScale[NPC_RARM] = { 0.20f, 0.60f, 0.20f };
    m_vPartScale[NPC_LLEG] = { 0.20f, 0.60f, 0.20f };
    m_vPartScale[NPC_RLEG] = { 0.20f, 0.60f, 0.20f };

    m_vPartOffset[NPC_HEAD] = { 0.00f, 2.20f, 0.00f };
    m_vPartOffset[NPC_BODY] = { 0.00f, 1.40f, 0.00f };
    m_vPartOffset[NPC_LARM] = { 0.70f, 1.20f, 0.00f };
    m_vPartOffset[NPC_RARM] = { -0.70f, 1.20f, 0.00f };
    m_vPartOffset[NPC_LLEG] = { 0.26f, 0.45f, 0.00f };
    m_vPartOffset[NPC_RLEG] = { -0.26f, 0.45f, 0.00f };

    return S_OK;
}

_int CNPC::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    m_pColliderCom->Update_AABB(vPos);

    CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

    return iExit;
}

void CNPC::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CNPC::Render_GameObject()
{
    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    _matrix matRootWorld = *m_pTransformCom->Get_World();

    _float fYaw = D3DXToRadian(m_pTransformCom->m_vAngle.y);

    for (int i = 0; i < NPC_PART_END; ++i)
        Render_Part((NPC_PART)i, 0.f, fYaw, 0.f, matRootWorld);

    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

HRESULT CNPC::Add_Component()
{
        FACE_UV uvHead[6] = {
            {0.125f, 0.125f, 0.25f,  0.25f },
            {0.375f, 0.125f, 0.5f,   0.25f },
            {0.125f, 0.0f,   0.25f,  0.125f},
            {0.25f,  0.0f,   0.375f, 0.125f},
            {0.0f,   0.125f, 0.125f, 0.25f },
            {0.25f,  0.125f, 0.375f, 0.25f },
        };
        FACE_UV uvBody[6] = {
            {0.3125f, 0.3125f, 0.4375f, 0.5f   },
            {0.5f,    0.3125f, 0.625f,  0.5f   },
            {0.3125f, 0.25f,   0.4375f, 0.3125f},
            {0.4375f, 0.25f,   0.5625f, 0.3125f},
            {0.25f,   0.3125f, 0.3125f, 0.5f   },
            {0.4375f, 0.3125f, 0.5f,    0.5f   },
        };
        FACE_UV uvRArm[6] = {
            {0.6875f, 0.3125f, 0.75f,   0.5f   },
            {0.8125f, 0.3125f, 0.875f,  0.5f   },
            {0.6875f, 0.25f,   0.75f,   0.3125f},
            {0.75f,   0.25f,   0.8125f, 0.3125f},
            {0.625f,  0.3125f, 0.6875f, 0.5f   },
            {0.75f,   0.3125f, 0.8125f, 0.5f   },
        };
        FACE_UV uvLArm[6] = {
            {0.5625f, 0.8125f, 0.625f,  1.0f   },
            {0.6875f, 0.8125f, 0.75f,   1.0f   },
            {0.5625f, 0.75f,   0.625f,  0.8125f},
            {0.625f,  0.75f,   0.6875f, 0.8125f},
            {0.5f,    0.8125f, 0.5625f, 1.0f   },
            {0.625f,  0.8125f, 0.6875f, 1.0f   },
        };
        FACE_UV uvRLeg[6] = {
            {0.0625f, 0.3125f, 0.125f,  0.5f   },
            {0.1875f, 0.3125f, 0.25f,   0.5f   },
            {0.0625f, 0.25f,   0.125f,  0.3125f},
            {0.125f,  0.25f,   0.1875f, 0.3125f},
            {0.0f,    0.3125f, 0.0625f, 0.5f   },
            {0.125f,  0.3125f, 0.1875f, 0.5f   },
        };
        FACE_UV uvLLeg[6] = {
            {0.3125f, 0.8125f, 0.375f,  1.0f   },
            {0.4375f, 0.8125f, 0.5f,    1.0f   },
            {0.3125f, 0.75f,   0.375f,  0.8125f},
            {0.375f,  0.75f,   0.4375f, 0.8125f},
            {0.25f,   0.8125f, 0.3125f, 1.0f   },
            {0.375f,  0.8125f, 0.4375f, 1.0f   },
        };

        FACE_UV* uvTable[NPC_PART_END] = { uvHead, uvBody, uvLArm, uvRArm, uvRLeg, uvLLeg };
        const wchar_t* tagTable[NPC_PART_END] = {
            L"Com_HeadBuf", L"Com_BodyBuf",
            L"Com_LArmBuf", L"Com_RArmBuf",
            L"Com_LLegBuf", L"Com_RLegBuf"
        };

        for (int i = 0; i < NPC_PART_END; ++i)
        {
            m_pBufferCom[i] = CPlayerBody::Create(m_pGraphicDev, uvTable[i]);
            if (!m_pBufferCom[i]) return E_FAIL;
            m_mapComponent[ID_STATIC].insert({ tagTable[i], m_pBufferCom[i] });
        }

        // 텍스처
        Engine::CComponent* pComponent = nullptr;
        pComponent = m_pTextureCom = dynamic_cast<Engine::CTexture*>(
            CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_NPCTexture"));
        if (!pComponent) return E_FAIL;
        m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

        // 트랜스폼
        pComponent = m_pTransformCom = dynamic_cast<Engine::CTransform*>(
            CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
        if (!pComponent) return E_FAIL;
        m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

        // 콜라이더
        m_pColliderCom = CCollider::Create(m_pGraphicDev,
            _vec3(1.2f, 3.2f, 1.2f), _vec3(0.f, 1.3f, 0.f));
        if (!m_pColliderCom) return E_FAIL;
        m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

        return S_OK;
}

void CNPC::Render_Part(NPC_PART ePart, _float fAngleX, _float fAngleY, _float fAngleZ,
    const _matrix& matRootWorld)
{
    _matrix matScale;
    D3DXMatrixScaling(&matScale,
        m_vPartScale[ePart].x,
        m_vPartScale[ePart].y,
        m_vPartScale[ePart].z);

    _matrix matPivotDown;
    D3DXMatrixTranslation(&matPivotDown, 0.f, -m_vPartScale[ePart].y, 0.f);

    _matrix matRotX, matRotY, matRotZ;
    D3DXMatrixRotationX(&matRotX, fAngleX);
    D3DXMatrixRotationY(&matRotY, fAngleY);
    D3DXMatrixRotationZ(&matRotZ, fAngleZ);

    _matrix matJoint;
    D3DXMatrixTranslation(&matJoint,
        m_vPartOffset[ePart].x,
        m_vPartOffset[ePart].y + m_vPartScale[ePart].y,
        m_vPartOffset[ePart].z);

    _matrix matPartWorld = matScale * matPivotDown * matRotX * matRotY * matJoint * matRootWorld;

    m_matPartWorld[ePart] = matPartWorld;

    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matPartWorld);
    m_pTextureCom->Set_Texture(0);
    m_pBufferCom[ePart]->Render_Buffer();
}

void CNPC::Interact()
{
    if (!m_pDialogueBox)
        return;

    if (m_pDialogueBox->Is_Visible())
        m_pDialogueBox->Hide();
    else
        m_pDialogueBox->Show(L"Knight", L"어서오게, 여행자여! 마침 몬스터들 때문에 골머리를 썩고 있었다네");
}

bool CNPC::Is_Interactable(const _vec3& vPlayerPos, const _vec3& vPickPos)
{
    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);

    _vec3 vPlayerDiff = vPos - vPlayerPos;
    vPlayerDiff.y = 0.f;
    if (D3DXVec3Length(&vPlayerDiff) >= 2.f)
        return false;

    _vec3 vPickDiff = vPickPos - vPos;
    vPickDiff.y = 0.f;
    if (D3DXVec3Length(&vPickDiff) >= 2.f)
        return false;

    return true;
}

CNPC* CNPC::Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos)
{
    CNPC* pNPC = new CNPC(pGraphicDev);

    if (FAILED(pNPC->Ready_GameObject(vPos)))
    {
        Safe_Release(pNPC);
        MSG_BOX("CNPC Create Failed");
        return nullptr;
    }

    return pNPC;
}

void CNPC::Free()
{
    CGameObject::Free();
}