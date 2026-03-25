#include "pch.h"
#include "CExplosionLight.h"

CExplosionLight::CExplosionLight(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos)
    : m_pGraphicDev(pGraphicDev), m_vPos(vPos)
{
    m_pGraphicDev->AddRef();

    
    m_pExplosionCube = Engine::CCubeTex::Create(m_pGraphicDev);

   
    D3DLIGHT9 tLight;
    ZeroMemory(&tLight, sizeof(D3DLIGHT9));
    tLight.Type = D3DLIGHT_POINT;
    tLight.Diffuse = { 1.f, 1.f, 1.f, 1.f };
    tLight.Ambient = { 0.f, 0.f, 0.f, 1.f };
    tLight.Position = { m_vPos.x, m_vPos.y + 1.5f, m_vPos.z };
    tLight.Range = 0.f;
    tLight.Attenuation0 = 0.f;
    tLight.Attenuation1 = 0.05f;
    tLight.Attenuation2 = 0.f;

    m_pGraphicDev->SetLight(m_iLightIndex, &tLight);
    m_pGraphicDev->LightEnable(m_iLightIndex, TRUE);
}

CExplosionLight::~CExplosionLight()
{
    m_pGraphicDev->LightEnable(m_iLightIndex, FALSE);
    Safe_Release(m_pExplosionCube);
    Safe_Release(m_pGraphicDev);
}

void CExplosionLight::Update(const _float& fTimeDelta)
{
    if (m_bDone) return;

    m_fTimer += fTimeDelta;

   
    _float      fRange = 0.f;
    D3DXCOLOR   tColor = { 1.f, 1.f, 1.f, 1.f };

    if (m_fTimer < m_fExpandEnd1)
    {
        
        _float fRatio = m_fTimer / m_fExpandEnd1;
        fRange = 80.f * fRatio;
        tColor = { 1.f, 1.f, 1.f, 1.f };
    }
    else if (m_fTimer < m_fExpandEnd2)
    {
        
        _float fRatio = (m_fTimer - m_fExpandEnd1) / (m_fExpandEnd2 - m_fExpandEnd1);
        fRange = 80.f + 20.f * fRatio;
        tColor = { 1.f, 1.f - fRatio * 0.15f, 0.f, 1.f };
    }
    else if (m_fTimer < m_fLightEnd)
    {
        
        _float fRatio = (m_fTimer - m_fExpandEnd2) / (m_fLightEnd - m_fExpandEnd2);
        fRange = m_fMaxRange * (1.f - fRatio);
        _float fG = 0.85f + (0.4f - 0.85f) * fRatio;
        _float fB = 0.4f + (0.1f - 0.4f) * fRatio;
        tColor = { 1.f, fG, fB, 1.f };
    }
    else
    {
        m_bDone = true;
        return;
    }

    D3DLIGHT9 tLight;
    ZeroMemory(&tLight, sizeof(D3DLIGHT9));
    tLight.Type = D3DLIGHT_POINT;
    tLight.Diffuse = tColor;
    tLight.Position = { m_vPos.x, m_vPos.y + 1.5f, m_vPos.z };
    tLight.Range = fRange;
    tLight.Attenuation0 = 0.f;
    tLight.Attenuation1 = 0.05f;
    tLight.Attenuation2 = 0.f;

    m_pGraphicDev->SetLight(m_iLightIndex, &tLight);
}

void CExplosionLight::Render()
{
    
    if (m_fTimer < m_fFlashEnd)
    {
        _float fAlpha = m_fMaxAlpha * (1.f - m_fTimer / m_fFlashEnd);
        DWORD  dwAlpha = (DWORD)(fAlpha * 255.f);

        D3DVIEWPORT9 vp;
        m_pGraphicDev->GetViewport(&vp);

        struct FLASHVTX { float x, y, z, rhw; DWORD color; };
        DWORD dwColor = D3DCOLOR_ARGB(dwAlpha, 255, 230, 130);

        FLASHVTX quad[4] =
        {
            { 0.f,             (float)vp.Height, 0.f, 1.f, dwColor },
            { 0.f,             0.f,              0.f, 1.f, dwColor },
            { (float)vp.Width, (float)vp.Height, 0.f, 1.f, dwColor },
            { (float)vp.Width, 0.f,              0.f, 1.f, dwColor },
        };

        DWORD dwOldAlpha, dwOldSrc, dwOldDst, dwOldZWrite, dwOldLighting;
        m_pGraphicDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwOldAlpha);
        m_pGraphicDev->GetRenderState(D3DRS_SRCBLEND, &dwOldSrc);
        m_pGraphicDev->GetRenderState(D3DRS_DESTBLEND, &dwOldDst);
        m_pGraphicDev->GetRenderState(D3DRS_ZWRITEENABLE, &dwOldZWrite);
        m_pGraphicDev->GetRenderState(D3DRS_LIGHTING, &dwOldLighting);

        m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE);
        m_pGraphicDev->SetTexture(0, nullptr);
        m_pGraphicDev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        m_pGraphicDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(FLASHVTX));

        m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwOldAlpha);
        m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, dwOldSrc);
        m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, dwOldDst);
        m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, dwOldZWrite);
        m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, dwOldLighting);
    }

   
    if (m_fTimer >= m_fSphereEnd || !m_pExplosionCube) return;

    _float fRatio = m_fTimer / m_fSphereEnd;
    _float fScale = m_fMaxScale * fRatio;
    _float fAlpha = 1.f - fRatio;   
    DWORD  dwAlpha = (DWORD)(fAlpha * 255.f);

    D3DXMATRIX matScale, matTrans, matWorld;
    D3DXMatrixScaling(&matScale, fScale, fScale, fScale);
    D3DXMatrixTranslation(&matTrans, m_vPos.x, m_vPos.y + 1.5f, m_vPos.z);
    matWorld = matScale * matTrans;

    DWORD dwOldAlpha, dwOldSrc, dwOldDst, dwOldZWrite,
        dwOldLighting, dwOldCull;
    m_pGraphicDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwOldAlpha);
    m_pGraphicDev->GetRenderState(D3DRS_SRCBLEND, &dwOldSrc);
    m_pGraphicDev->GetRenderState(D3DRS_DESTBLEND, &dwOldDst);
    m_pGraphicDev->GetRenderState(D3DRS_ZWRITEENABLE, &dwOldZWrite);
    m_pGraphicDev->GetRenderState(D3DRS_LIGHTING, &dwOldLighting);
    m_pGraphicDev->GetRenderState(D3DRS_CULLMODE, &dwOldCull);

    
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE);
    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    
    m_pGraphicDev->SetTexture(0, nullptr);
    m_pGraphicDev->SetRenderState(D3DRS_TEXTUREFACTOR,
        D3DCOLOR_ARGB(dwAlpha, 255, 160, 30));
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);

    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
    m_pExplosionCube->Render_Buffer();

    
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwOldAlpha);
    m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, dwOldSrc);
    m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, dwOldDst);
    m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, dwOldZWrite);
    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, dwOldLighting);
    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, dwOldCull);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
}