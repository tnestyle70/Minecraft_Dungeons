#include "pch.h"
//#include "CNPC.h"
//#include "CRenderer.h"
//#include "CManagement.h"
//#include "CBlockMgr.h"
//#include "CCollider.h"
//#include "CProtoMgr.h"
//
//CNPC::CNPC(LPDIRECT3DDEVICE9 pGraphicDev)
//    : CGameObject(pGraphicDev)
//    , m_pTransformCom(nullptr)
//    , m_pTextureCom(nullptr)
//    , m_pColliderCom(nullptr)
//{
//    ZeroMemory(m_pBufferCom, sizeof(m_pBufferCom));
//    ZeroMemory(m_matPartWorld, sizeof(m_matPartWorld));
//}
//
//CNPC::~CNPC()
//{
//}
//
//HRESULT CNPC::Ready_GameObject(_vec3 vPos)
//{
//    if (FAILED(Add_Component()))
//        return E_FAIL;
//
//    m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
//
//    m_vPartScale[NPC_HEAD] = { 0.40f, 0.40f, 0.40f };
//    m_vPartScale[NPC_BODY] = { 0.50f, 0.50f, 0.25f };
//    m_vPartScale[NPC_LARM] = { 0.20f, 0.60f, 0.20f };
//    m_vPartScale[NPC_RARM] = { 0.20f, 0.60f, 0.20f };
//    m_vPartScale[NPC_LLEG] = { 0.20f, 0.60f, 0.20f };
//    m_vPartScale[NPC_RLEG] = { 0.20f, 0.60f, 0.20f };
//
//    m_vPartOffset[NPC_HEAD] = { 0.00f, 2.20f, 0.00f };
//    m_vPartOffset[NPC_BODY] = { 0.00f, 1.40f, 0.00f };
//    m_vPartOffset[NPC_LARM] = { 0.70f, 1.20f, 0.00f };
//    m_vPartOffset[NPC_RARM] = { -0.70f, 1.20f, 0.00f };
//    m_vPartOffset[NPC_LLEG] = { 0.26f, 0.45f, 0.00f };
//    m_vPartOffset[NPC_RLEG] = { -0.26f, 0.45f, 0.00f };
//
//    return S_OK;
//}
//
//_int CNPC::Update_GameObject(const _float& fTimeDelta)
//{
//    _int iExit = CGameObject::Update_GameObject(fTimeDelta);
//
//    _vec3 vPos;
//    m_pTransformCom->Get_Info(INFO_POS, &vPos);
//    m_pColliderCom->Update_AABB(vPos);
//
//    CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);
//
//    return iExit;
//}
//
//void CNPC::LateUpdate_GameObject(const _float& fTimeDelta)
//{
//    CGameObject::LateUpdate_GameObject(fTimeDelta);
//}
//
//void CNPC::Render_GameObject()
//{
//    _vec3 vPos;
//    m_pTransformCom->Get_Info(INFO_POS, &vPos);
//
//    _float fYaw;
//   // m_pTransformCom->Get_Info(INFO_ANGLE, nullptr); // 각도는 트랜스폼에서
//    fYaw = m_pTransformCom->m_vAngle.y;
//
//    _matrix matRootWorld;
//    D3DXMatrixTranslation(&matRootWorld, vPos.x, vPos.y, vPos.z);
//
//    for (int i = 0; i < NPC_PART_END; ++i)
//        Render_Part((NPC_PART)i, 0.f, fYaw, 0.f, matRootWorld);
//}
//
//HRESULT CNPC::Add_Component()
//{
//    Engine::CComponent* pComponent = nullptr;
//
//    // Transform
//    pComponent = m_pTransformCom = dynamic_cast<Engine::CTransform*>(
//        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
//    if (!pComponent) return E_FAIL;
//    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });
//
//    // Texture — NPC 전용 텍스처 프로토타입 필요
//    pComponent = m_pTextureCom = dynamic_cast<Engine::CTexture*>(
//        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_NPCTexture"));
//    if (!pComponent) return E_FAIL;
//    m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });
//
//    // Buffer (파츠)
//    for (int i = 0; i < NPC_PART_END; ++i)
//    {
//        m_pBufferCom[i] = dynamic_cast<CPlayerBody*>(
//            CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_PlayerBody"));
//      //  if (!m_pBufferCom[i]) return E_FAIL;
//      //  m_mapComponent[ID_STATIC].insert({ L"Com_Buffer" + to_wstring(i),
//      //      m_pBufferCom[i] });
//    }
//
//    // Collider
//    m_pColliderCom = CCollider::Create(m_pGraphicDev,
//        _vec3(0.8f, 3.2f, 0.8f), _vec3(0.f, 1.3f, 0.f));
//    if (!m_pColliderCom) return E_FAIL;
//    m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });
//
//    return S_OK;
//}
//
//void CNPC::Render_Part(NPC_PART ePart, _float fAngleX, _float fAngleY, _float fAngleZ,
//    const _matrix& matRootWorld)
//{
//    _matrix matScale, matRotX, matRotY, matRotZ, matTrans;
//
//    D3DXMatrixScaling(&matScale,
//        m_vPartScale[ePart].x,
//        m_vPartScale[ePart].y,
//        m_vPartScale[ePart].z);
//
//    D3DXMatrixRotationX(&matRotX, D3DXToRadian(fAngleX));
//    D3DXMatrixRotationY(&matRotY, D3DXToRadian(fAngleY));
//    D3DXMatrixRotationZ(&matRotZ, D3DXToRadian(fAngleZ));
//
//    D3DXMatrixTranslation(&matTrans,
//        m_vPartOffset[ePart].x,
//        m_vPartOffset[ePart].y,
//        m_vPartOffset[ePart].z);
//
//    m_matPartWorld[ePart] = matScale * matRotX * matRotY * matRotZ * matTrans * matRootWorld;
//
//    m_pGraphicDev->SetTransform(D3DTS_WORLD, &m_matPartWorld[ePart]);
//    m_pTextureCom->Set_Texture(0);
//    m_pBufferCom[ePart]->Render_Buffer();
//}
//
//void CNPC::Interact()
//{
//    m_bTalking = !m_bTalking;
//}
//
//bool CNPC::Is_Interactable(const _vec3& vPlayerPos, const _vec3& vPickPos)
//{
//    _vec3 vPos;
//    m_pTransformCom->Get_Info(INFO_POS, &vPos);
//
//    _vec3 vPlayerDiff = vPos - vPlayerPos;
//    vPlayerDiff.y = 0.f;
//    if (D3DXVec3Length(&vPlayerDiff) >= 2.f)
//        return false;
//
//    _vec3 vPickDiff = vPickPos - vPos;
//    vPickDiff.y = 0.f;
//    if (D3DXVec3Length(&vPickDiff) >= 2.f)
//        return false;
//
//    return true;
//}
//
//CNPC* CNPC::Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos)
//{
//    CNPC* pNPC = new CNPC(pGraphicDev);
//
//    if (FAILED(pNPC->Ready_GameObject(vPos)))
//    {
//        Safe_Release(pNPC);
//        MSG_BOX("CNPC Create Failed");
//        return nullptr;
//    }
//
//    return pNPC;
//}
//
//void CNPC::Free()
//{
//    CGameObject::Free();
//}