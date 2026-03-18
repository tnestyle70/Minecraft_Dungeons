#include "pch.h"
#include "CObjectEditor.h"
#include "CBox.h"
#include "CDynamicCamera.h"

CObjectEditor::CObjectEditor(LPDIRECT3DDEVICE9 pGraphicDev)
    : CScene(pGraphicDev)
{
}

CObjectEditor::~CObjectEditor()
{
}

HRESULT CObjectEditor::Ready_Scene()
{
    if (FAILED(Ready_Prototype()))
        return E_FAIL;

    if (FAILED(Ready_Environment_Layer(L"Environment_Layer")))
        return E_FAIL;

    return S_OK;
}

_int CObjectEditor::Update_Scene(const _float& fTimeDelta)
{
    _int iExit = CScene::Update_Scene(fTimeDelta);

    Editor_Input();

    return iExit;
}

void CObjectEditor::LateUpdate_Scene(const _float& fTimeDelta)
{
    CScene::LateUpdate_Scene(fTimeDelta);
}

void CObjectEditor::Render_Scene()
{
    if (!m_pSelectedObject) return;

    CTransform* pTrans = dynamic_cast<CTransform*>(
        m_pSelectedObject->Get_Component(ID_DYNAMIC, L"Com_Transform"));

    CCollider* pCol = dynamic_cast<CCollider*>(
        m_pSelectedObject->Get_Component(ID_STATIC, L"Com_Collider"));

    if (!pTrans || !pCol) return;

    AABB aabb = pCol->Get_AABB();
    _vec3 vMin = aabb.vMin;
    _vec3 vMax = aabb.vMax;

    D3DXVECTOR3 v[8] =
    {
        { vMin.x, vMin.y, vMin.z }, { vMax.x, vMin.y, vMin.z },
        { vMax.x, vMin.y, vMax.z }, { vMin.x, vMin.y, vMax.z },
        { vMin.x, vMax.y, vMin.z }, { vMax.x, vMax.y, vMin.z },
        { vMax.x, vMax.y, vMax.z }, { vMin.x, vMax.y, vMax.z },
    };

    D3DXVECTOR3 lines[24] =
    {
        v[0],v[1], v[1],v[2], v[2],v[3], v[3],v[0],
        v[4],v[5], v[5],v[6], v[6],v[7], v[7],v[4],
        v[0],v[4], v[1],v[5], v[2],v[6], v[3],v[7],
    };

    DWORD dwLighting, dwFVF;
    m_pGraphicDev->GetRenderState(D3DRS_LIGHTING, &dwLighting);
    m_pGraphicDev->GetFVF(&dwFVF);

    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE);
    m_pGraphicDev->SetFVF(D3DFVF_XYZ);

    D3DXMATRIX matWorld;
    D3DXMatrixIdentity(&matWorld);
    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

    m_pGraphicDev->DrawPrimitiveUP(D3DPT_LINELIST, 12, lines, sizeof(D3DXVECTOR3));

    // 원래 상태 복구
    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, dwLighting);
    m_pGraphicDev->SetFVF(dwFVF);
}

HRESULT CObjectEditor::Ready_Environment_Layer(const _tchar* pLayerTag)
{
    CLayer* pLayer = CLayer::Create();

    if (!pLayer)
        return E_FAIL;

    CGameObject* pGameObject = nullptr;

    //dynamic camera
    _vec3 vEye{ 0.f, 10.f, -10.f };
    _vec3 vAt{ 0.f, 0.f, 1.f };
    _vec3 vUp{ 0.f, 1.f, 0.f };

    pGameObject = CDynamicCamera::Create(m_pGraphicDev, &vEye, &vAt, &vUp);

    if (!pGameObject)
        return E_FAIL;

    pLayer->Add_GameObject(L"DynamicCamera", pGameObject);

    m_mapLayer.insert({ pLayerTag, pLayer });

    return S_OK;
}

HRESULT CObjectEditor::Ready_Prototype()
{
    // Transform
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Transform", Engine::CTransform::Create(m_pGraphicDev))))
        return E_FAIL;

    // Box
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BoxBottomTex", Engine::CBoxBottomTex::Create(m_pGraphicDev))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BoxTopTex", Engine::CBoxTopTex::Create(m_pGraphicDev))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BoxTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Object/T_LargeBoxChest.png"))))
        return E_FAIL;

    return S_OK;
}

void CObjectEditor::Editor_Input()
{
    if (GetAsyncKeyState(VK_LBUTTON) & 0x0001)
    {
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
        {
            _vec3 vOrigin, vDir;
            Get_MouseRay(vOrigin, vDir);

            CGameObject* pPicked = nullptr;
            _float fMinT = FLT_MAX;

            for (auto& pair : m_mapEditObject)
            {
                CGameObject* pObj = pair.second;

                CTransform* pTrans = dynamic_cast<CTransform*>(
                    pObj->Get_Component(ID_DYNAMIC, L"Com_Transform"));

                CCollider* pCol = dynamic_cast<CCollider*>(
                    pObj->Get_Component(ID_STATIC, L"Com_Collider"));

                if (!pTrans || !pCol) continue;

                AABB aabb = pCol->Get_AABB();

                _float t = 0.f;
                if (IntersectRayAABB(vOrigin, vDir, aabb.vMin, aabb.vMax, t))
                {
                    if (t < fMinT)
                    {
                        fMinT = t;
                        pPicked = pObj;
                    }
                }
            }

            m_pSelectedObject = pPicked;
        }
        else
        {
            D3DXVECTOR3 vPos = Get_MouseWorldPos();

            CGameObject* pBox = CBox::Create(m_pGraphicDev);

            CTransform* pBoxTransformCom = dynamic_cast<CTransform*>(pBox->Get_Component(ID_DYNAMIC, L"Com_Transform"));

            if (pBox)
            {
                pBoxTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);

                // 기존처럼 렌더/업데이트용으로 레이어에도 추가
                m_mapLayer[L"Environment_Layer"]->Add_GameObject(L"Box", pBox);

                // 피킹/선택용으로 에디터 컨테이너에도 추가 (키를 unique하게)
                wstring wstrKey = L"Box_" + to_wstring(m_mapEditObject.size());
                m_mapEditObject.insert({ wstrKey, pBox });
            }
        }
    }
}

_vec3 CObjectEditor::Get_MouseWorldPos()
{
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(g_hWnd, &pt);

    D3DVIEWPORT9 vp;
    m_pGraphicDev->GetViewport(&vp);

    D3DXVECTOR3 vNear(pt.x, pt.y, 0.f);
    D3DXVECTOR3 vFar(pt.x, pt.y, 1.f);

    D3DXMATRIX matProj, matView, matWorld;
    m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matProj);
    m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);
    D3DXMatrixIdentity(&matWorld);

    D3DXVec3Unproject(&vNear, &vNear, &vp, &matProj, &matView, &matWorld);
    D3DXVec3Unproject(&vFar, &vFar, &vp, &matProj, &matView, &matWorld);

    float t = -vNear.y / (vFar.y - vNear.y);

    return vNear + (vFar - vNear) * t;
}

void CObjectEditor::Get_MouseRay(_vec3& vOrigin, _vec3& vDir)
{
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(g_hWnd, &pt);

    D3DVIEWPORT9 vp;
    m_pGraphicDev->GetViewport(&vp);

    D3DXVECTOR3 vNear(pt.x, pt.y, 0.f);
    D3DXVECTOR3 vFar(pt.x, pt.y, 1.f);

    D3DXMATRIX matProj, matView, matWorld;
    m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matProj);
    m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);
    D3DXMatrixIdentity(&matWorld);

    D3DXVec3Unproject(&vNear, &vNear, &vp, &matProj, &matView, &matWorld);
    D3DXVec3Unproject(&vFar, &vFar, &vp, &matProj, &matView, &matWorld);

    vOrigin = vNear;
    vDir = vFar - vNear;
    D3DXVec3Normalize(&vDir, &vDir);
}

bool CObjectEditor::IntersectRayAABB(const _vec3& rayOrigin, const _vec3& rayDir, const _vec3& min, const _vec3& max, _float& t)
{
    float tmin = (min.x - rayOrigin.x) / rayDir.x;
    float tmax = (max.x - rayOrigin.x) / rayDir.x;

    if (tmin > tmax) swap(tmin, tmax);

    float tymin = (min.y - rayOrigin.y) / rayDir.y;
    float tymax = (max.y - rayOrigin.y) / rayDir.y;

    if (tymin > tymax) swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    tmin = max(tmin, tymin);
    tmax = min(tmax, tymax);

    float tzmin = (min.z - rayOrigin.z) / rayDir.z;
    float tzmax = (max.z - rayOrigin.z) / rayDir.z;

    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    tmin = max(tmin, tzmin);

    t = tmin;
    return true;
}

CObjectEditor* CObjectEditor::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CObjectEditor* pObjectEditor = new CObjectEditor(pGraphicDev);

    if (FAILED(pObjectEditor->Ready_Scene()))
    {
        Safe_Release(pObjectEditor);
        MSG_BOX("ObjectEditor Create Failed");
        return nullptr;
    }

    return pObjectEditor;
}

void CObjectEditor::Free()
{
    m_mapEditObject.clear();
    CScene::Free();
}