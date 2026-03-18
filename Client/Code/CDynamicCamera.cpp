#include "pch.h"
#include "CDynamicCamera.h"
#include "CDInputMgr.h"

CDynamicCamera::CDynamicCamera(LPDIRECT3DDEVICE9 pGraphicDev)
    : CCamera(pGraphicDev), m_bFix(true), m_bCheck(true)
{
}

CDynamicCamera::CDynamicCamera(const CDynamicCamera& rhs)
    : CCamera(rhs), m_bFix(true), m_bCheck(true)
{
}

CDynamicCamera::~CDynamicCamera()
{
}

HRESULT CDynamicCamera::Ready_GameObject(const _vec3* pEye, 
    const _vec3* pAt, 
    const _vec3* pUp,
    const _float& fFov, 
    const _float& fAspect, 
    const _float& fNear,
    const _float& fFar)
{
    m_vEye = *pEye;
    m_vAt = *pAt;
    m_vUp = *pUp;

    m_fFov = fFov;
    m_fAspect = fAspect;
    m_fNear = fNear;
    m_fFar = fFar;

    if (FAILED(CCamera::Ready_GameObject()))
        return E_FAIL;

    m_fSpeed = 20.f;

    return S_OK;
}

_int CDynamicCamera::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CCamera::Update_GameObject(fTimeDelta);

    // F1 토글
    if (GetAsyncKeyState(VK_F2) & 0x8000)
    {
        if (!m_bF2Check)
        {
            m_bF2Check = true;
            m_bFollowMode = !m_bFollowMode;
            m_bFix = !m_bFollowMode;  // ← 추가
        }
    }
    else
    {
        m_bF2Check = false;
    }

    if (m_bActionCam)
    {
        Update_ActionCam(fTimeDelta);
    }
    else if (m_pTargetTransform && m_bFollowMode)
    {
        _vec3 vPlayerPos;
        m_pTargetTransform->Get_Info(INFO_POS, &vPlayerPos);
        m_vEye = vPlayerPos + m_vFollowOffset;
        m_vAt = vPlayerPos + _vec3(0.f, 1.5f, 0.f);
    }
    else
    {
        Key_Input(fTimeDelta);
        if (m_bFix)
        {
            Mouse_Fix();
            Mouse_Move();
        }
    }

    return iExit;
}

void CDynamicCamera::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CCamera::LateUpdate_GameObject(fTimeDelta);
}

_int CDynamicCamera::Update_ActionCam(const _float& fTimeDelta)
{
    if (!m_bActionCam)
        return 0;

    //WayPoint - vEye 로 방향 구해서, 거리 판단 이후 그 방향만큼 이동
    _vec3 vDir = m_wpTarget.vEye - m_vEye;
    _float fDist = D3DXVec3Length(&vDir);
   
    if (fDist < 1.f)
    {
        //목적지에 도달했는지 판단
        if (m_deqWayPoints.empty())
        {
            m_bActionCam = false;
            return 0;
        }
        //Move to Next Way Point
        m_wpTarget = m_deqWayPoints.front();
        m_deqWayPoints.pop_front();
        return 0;
    }
    else
    {
        D3DXVec3Normalize(&vDir, &vDir);
        m_vEye += vDir * fTimeDelta * m_fSpeed;
    }

    return 0;
}

void CDynamicCamera::SetActionCam()
{
    //Setting Way Points
    m_deqWayPoints.push_back({ { 10.f, 10.f, 5.f },{ m_vUp.x, m_vUp.y, m_vUp.z }, 5.f });
    m_deqWayPoints.push_back({ { 20.f, 10.f, 5.f }, { m_vUp.x, m_vUp.y, m_vUp.z }, 5.f });
    m_deqWayPoints.push_back({ { 50.f, 10.f, 5.f }, { m_vUp.x, m_vUp.y, m_vUp.z }, 5.f });
    m_deqWayPoints.push_back({ { m_vEye.x, m_vEye.y , m_vEye.z }, { m_vUp.x, m_vUp.y, m_vUp.z }, 5.f });

    //액션캠 시작, 끝 위치 설정 - 스테이지 기본 세팅대로, 웨이 포인트들 지정해주고 웨이 포인트 끝나면 캠 종료
    //m_wpStart = { m_vEye, m_vUp, 0.f };
    //처음 Target 설정
    m_wpTarget = m_deqWayPoints.front();
    m_deqWayPoints.pop_front();

    m_bActionCam = true;

    return;
}


void CDynamicCamera::Key_Input(const _float& fTimeDelta)
{
    _matrix matCamWorld;
    D3DXMatrixInverse(&matCamWorld, 0, &m_matView);

    if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_D) & 0x80)
    {
        _vec3   vRight;
        memcpy(&vRight, &matCamWorld.m[0][0], sizeof(_vec3));

        _vec3   vLength = *D3DXVec3Normalize(&vRight, &vRight) * m_fSpeed * fTimeDelta;

        m_vEye += vLength;
        m_vAt += vLength;
    }

    if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_A) & 0x80)
    {
        _vec3   vRight;
        memcpy(&vRight, &matCamWorld.m[0][0], sizeof(_vec3));

        _vec3   vLength = *D3DXVec3Normalize(&vRight, &vRight) * m_fSpeed * fTimeDelta;

        m_vEye -= vLength;
        m_vAt  -= vLength;
    }

    if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_W) & 0x80)
    {
        _vec3   vLook;
        memcpy(&vLook, &matCamWorld.m[2][0], sizeof(_vec3));

        _vec3   vLength = *D3DXVec3Normalize(&vLook, &vLook) * m_fSpeed * fTimeDelta;

        m_vEye += vLength;
        m_vAt += vLength;
    }

    if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_S) & 0x80)
    {
        _vec3   vLook;
        memcpy(&vLook, &matCamWorld.m[2][0], sizeof(_vec3));

        _vec3   vLength = *D3DXVec3Normalize(&vLook, &vLook) * m_fSpeed * fTimeDelta;

        m_vEye -= vLength;
        m_vAt  -= vLength;
    }


    if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_TAB) & 0x80)
    {
        if (m_bCheck)
            return;

        m_bCheck = true;

        if (m_bFix)
            m_bFix = false;
        
        else
            m_bFix = true;

    }

    else
    {
        m_bCheck = false;
    }

    if (false == m_bFix)
        return;
}

void CDynamicCamera::Mouse_Move()
{
    _matrix matCamWorld;
    D3DXMatrixInverse(&matCamWorld, 0, &m_matView);

    _long   dwMouseMove(0);

    if (dwMouseMove = CDInputMgr::GetInstance()->Get_DIMouseMove(DIMS_Y))
    {
        _vec3   vRight;
        memcpy(&vRight, &matCamWorld.m[0][0], sizeof(_vec3));

        _vec3   vLook = m_vAt - m_vEye;

        _matrix matRot;

        D3DXMatrixRotationAxis(&matRot, &vRight, D3DXToRadian(dwMouseMove / 10.f));

        D3DXVec3TransformNormal(&vLook, &vLook, &matRot);

        m_vAt = m_vEye + vLook;
    }

    if (dwMouseMove = CDInputMgr::GetInstance()->Get_DIMouseMove(DIMS_X))
    {
        _vec3   vUp{ 0.f, 1.f, 0.f };

        _vec3   vLook = m_vAt - m_vEye;

        _matrix matRot;

        D3DXMatrixRotationAxis(&matRot, &vUp, D3DXToRadian(dwMouseMove / 10.f));

        D3DXVec3TransformNormal(&vLook, &vLook, &matRot);

        m_vAt = m_vEye + vLook;
    }

}

void CDynamicCamera::Mouse_Fix()
{
    POINT       ptMouse{ WINCX >> 1, WINCY >> 1 };

    ClientToScreen(g_hWnd, &ptMouse);
    SetCursorPos(ptMouse.x, ptMouse.y);

}

CDynamicCamera* CDynamicCamera::Create(LPDIRECT3DDEVICE9 pGraphicDev,
    const _vec3* pEye, const _vec3* pAt, const _vec3* pUp,
    const _float& fFov,    const _float& fAspect, const _float& fNear, const _float& fFar)
{
    CDynamicCamera* pCamera = new CDynamicCamera(pGraphicDev);

    if (FAILED(pCamera->Ready_GameObject(pEye, pAt, pUp, 
        fFov, fAspect, fNear, fFar)))
    {
        Safe_Release(pCamera);
        MSG_BOX("Camera Create Failed");
        return nullptr;
    }

    return pCamera;
}

void CDynamicCamera::Free()
{
    CCamera::Free();
}
