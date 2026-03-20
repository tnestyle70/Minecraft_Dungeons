#include "pch.h"
#include "CObjectEditor.h"
#include "CBox.h"
#include "CLamp.h"
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

    for (auto& pair : m_mapEditObject)
    {
        pair.second->Update_GameObject(fTimeDelta);
    }

    if (m_pPreviewObject)
    {
        _vec3 vPos = Get_MouseWorldPos();

        CTransform* pTrans = dynamic_cast<CTransform*>(
            m_pPreviewObject->Get_Component(ID_DYNAMIC, L"Com_Transform"));

        if (pTrans)
            pTrans->Set_Pos(vPos.x, vPos.y, vPos.z);
    }

    if (m_pPreviewObject)
    {
        m_pPreviewObject->Update_GameObject(fTimeDelta);
    }
     
    Editor_Input();

    return iExit;
}

void CObjectEditor::LateUpdate_Scene(const _float& fTimeDelta)
{
    CScene::LateUpdate_Scene(fTimeDelta);

    for (auto& pair : m_mapEditObject)
    {
        pair.second->LateUpdate_GameObject(fTimeDelta);
    }

    if (m_pPreviewObject)
    {
        m_pPreviewObject->LateUpdate_GameObject(fTimeDelta);
    }
}

void CObjectEditor::Render_Scene()
{
    for (auto& pair : m_mapEditObject)
    {
        pair.second->Render_GameObject();
    }

    if (m_pPreviewObject)
    {
        m_pPreviewObject->Render_GameObject();
    }

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

void CObjectEditor::Render_UI()
{
    // Object List
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(200, 300), ImGuiCond_Once);

    ImGui::Begin("Object List");

    ImGui::BeginChild("ObjectListChild", ImVec2(0, 0), true);

    for (auto& pair : m_mapEditObject)
    {
        std::string str(pair.first.begin(), pair.first.end());

        if (ImGui::Selectable(str.c_str(),
            m_pSelectedObject == pair.second))
        {
            m_pSelectedObject = pair.second;
        }
    }

    ImGui::EndChild();
    ImGui::End();

    Render_CreateUI();

    Render_Inspector();
}

void CObjectEditor::Render_CreateUI()
{
    ImGui::SetNextWindowPos(ImVec2(970, 10), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Once);

    ImGui::Begin("Create");

    if (ImGui::Button("Box"))
    {
        Start_CreateMode(L"Box");
    }

    if (ImGui::Button("Lamp"))
    {
        Start_CreateMode(L"Lamp");
    }

    ImGui::End();
}

void CObjectEditor::Render_Inspector()
{
    ImGui::SetNextWindowPos(ImVec2(970, 170), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Once);

    ImGui::Begin("Inspector");

    if (!m_pSelectedObject)
    {
        ImGui::Text("No Object Selected");
        ImGui::End();
        return;
    }

    CTransform* pTrans = dynamic_cast<CTransform*>(
        m_pSelectedObject->Get_Component(ID_DYNAMIC, L"Com_Transform"));

    if (!pTrans)
    {
        ImGui::End();
        return;
    }

    _vec3 vPos;
    pTrans->Get_Info(INFO_POS, &vPos);

    float pos[3] = { vPos.x, vPos.y, vPos.z };

    if (ImGui::DragFloat3("Position", pos, 0.1f))
    {
        pTrans->Set_Pos(pos[0], pos[1], pos[2]);
    }

    ImGui::End();
}

void CObjectEditor::Create_Object(const wstring& type)
{
    if (!m_pPreviewObject)
        return;

    CGameObject* pObj = nullptr;

    if (type == L"Box")
        pObj = CBox::Create(m_pGraphicDev);
    else if (type == L"Lamp")
        pObj = CLamp::Create(m_pGraphicDev);

    if (!pObj) return;

    CTransform* pPreviewTrans = dynamic_cast<CTransform*>(
        m_pPreviewObject->Get_Component(ID_DYNAMIC, L"Com_Transform"));

    CTransform* pNewTrans = dynamic_cast<CTransform*>(
        pObj->Get_Component(ID_DYNAMIC, L"Com_Transform"));

    if (pPreviewTrans && pNewTrans)
    {
        _vec3 vPos;
        pPreviewTrans->Get_Info(INFO_POS, &vPos);

        pNewTrans->Set_Pos(vPos.x, vPos.y, vPos.z);
    }

    static int iID = 0;
    wstring key = type + L"_" + to_wstring(iID++);

    m_mapEditObject.insert({ key, pObj });

    m_pSelectedObject = pObj;
}

void CObjectEditor::Start_CreateMode(const wstring& type)
{
    m_wstrCreateType = type;

    if (m_pPreviewObject)
        Safe_Release(m_pPreviewObject);

    if (type == L"Box")
        m_pPreviewObject = CBox::Create(m_pGraphicDev);
    else if (type == L"Lamp")
        m_pPreviewObject = CLamp::Create(m_pGraphicDev);
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

    if (FAILED(pLayer->Add_GameObject(L"DynamicCamera", pGameObject)))
        return E_FAIL;

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

    // Lamp
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_LampBodyTex", Engine::CLampBodyTex::Create(m_pGraphicDev))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_LampHeadTex", Engine::CLampHeadTex::Create(m_pGraphicDev))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_LampTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Object/T_Lamp.png"))))
        return E_FAIL;

    return S_OK;
}

void CObjectEditor::Editor_Input()
{
    ImGuiIO& io = ImGui::GetIO();

    if (io.WantCaptureMouse || io.WantCaptureKeyboard)
        return;

    bool bLButtonDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    bool bRButtonDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;

    bool bLButtonClicked = bLButtonDown && !m_bLButtonPrev;
    bool bRButtonClicked = bRButtonDown && !m_bRButtonPrev;

    m_bLButtonPrev = bLButtonDown;
    m_bRButtonPrev = bRButtonDown;

    if (bLButtonClicked)
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
    }

    if (bRButtonClicked)
    {
        if (m_pSelectedObject)
        {
            for (auto iter = m_mapEditObject.begin(); iter != m_mapEditObject.end(); )
            {
                if (iter->second == m_pSelectedObject)
                {
                    Safe_Release(iter->second);
                    iter = m_mapEditObject.erase(iter);
                }
                else
                    ++iter;
            }

            m_pSelectedObject = nullptr;
        }
    }

    if (GetAsyncKeyState(VK_RIGHT))
    {
        if (m_pSelectedObject)
        {
            CTransform* pTrans = dynamic_cast<CTransform*>(
                m_pSelectedObject->Get_Component(ID_DYNAMIC, L"Com_Transform"));

            pTrans->Rotation(ROT_Y, -3.f);
        }
    }
    else if (GetAsyncKeyState(VK_LEFT))
    {
        if (m_pSelectedObject)
        {
            CTransform* pTrans = dynamic_cast<CTransform*>(
                m_pSelectedObject->Get_Component(ID_DYNAMIC, L"Com_Transform"));

            pTrans->Rotation(ROT_Y, 3.f);
        }
    }

    if (bLButtonClicked && !m_wstrCreateType.empty())
    {
        Create_Object(m_wstrCreateType);

        Safe_Release(m_pPreviewObject);
        m_pPreviewObject = nullptr;
        m_wstrCreateType = L"";

        return;
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
    for (auto& pair : m_mapEditObject)
    {
        Safe_Release(pair.second);
    }
    m_mapEditObject.clear();

    CScene::Free();
}