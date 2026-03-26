#include "CParticleEmitter.h"

CParticleEmitter::CParticleEmitter(LPDIRECT3DDEVICE9 pGraphicDev)
    :CGameObject(pGraphicDev)
    , m_pVB(nullptr), m_pTexture(nullptr), m_fEmitAccTime(0.f),
    m_bBurstDone(false), m_bDead(false)
{
    ZeroMemory(&m_tParticleDesc, sizeof(m_tParticleDesc));
}

CParticleEmitter::~CParticleEmitter()
{
}

HRESULT CParticleEmitter::Ready_Emitter(const ParticleDesc& desc, LPDIRECT3DTEXTURE9 pTexture
       ,eParticlePreset& ePresetType)
{
    m_tParticleDesc = desc;
    m_eParticleType = ePresetType;

    // 텍스처 참조 카운트 증가
    m_pTexture = pTexture;
    if (m_pTexture)
        m_pTexture->AddRef();

    // 파티클 풀 초기화 (전부 비활성)
    m_vecPool.resize(m_tParticleDesc.iMaxParticles);
    for (auto& p : m_vecPool)
        p.bActive = false;

    //  동적 VB — 매 프레임 내용이 바뀌므로 DYNAMIC + DEFAULT
    //   CVIBuffer의 MANAGED 방식과 다름에 주의
    if (FAILED(m_pGraphicDev->CreateVertexBuffer(
        m_tParticleDesc.iMaxParticles * sizeof(ParticleVertex),
        D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
        PARTICLE_FVF,
        D3DPOOL_DEFAULT,
        &m_pVB,
        NULL)))
    {
        MSG_BOX("ParticleEmitter: VB Create Failed");
        return E_FAIL;
    }

    // Burst 방식이면 즉시 전부 방출
    if (m_tParticleDesc.fEmitRate == 0.f)
        Emit_Burst();

    return S_OK;
}

_int CParticleEmitter::Update_GameObject(const _float& fTimeDelta)
{
    if (m_bDead)
        return 0;

    // Rate 방식: 시간에 따라 서서히 방출
    if (m_tParticleDesc.fEmitRate > 0.f)
        Emit_ByRate(fTimeDelta);

    Update_Particles(fTimeDelta);

    // bLoop false + 모든 입자 비활성 + Burst 완료 → 이미터 사망
    if (!m_tParticleDesc.bLoop && m_bBurstDone)
    {
        _bool bAllDead = true;
        for (auto& p : m_vecPool)
        {
            if (p.bActive) { bAllDead = false; break; }
        }
        if (bAllDead)
            m_bDead = true;
    }

    return 0;
}

void CParticleEmitter::LateUpdate_GameObject(const _float& fTimeDelta)
{

}

void CParticleEmitter::Render_GameObject()
{
    if (m_bDead)
        return;

    Set_RenderState();
    Render_Particles();
    Reset_RenderState();
}

void CParticleEmitter::Reset_Particle(Particle& particle)
{
    particle.bActive = true;

    particle.vPos = m_tParticleDesc.vEmitPos;

    // ── 방향: vEmitDir 기준으로 fSpreadAngle 범위 내 랜덤 벡터 생성
    // 1. vEmitDir에 수직인 임의 벡터 두 개 구하기
    D3DXVECTOR3 vRight, vUp;
    D3DXVECTOR3 vDir = m_tParticleDesc.vEmitDir;

    // vDir과 수직인 축 하나 구함
    D3DXVECTOR3 vTemp = (fabsf(vDir.x) < 0.9f)
        ? D3DXVECTOR3(1.f, 0.f, 0.f)
        : D3DXVECTOR3(0.f, 1.f, 0.f);

    D3DXVec3Cross(&vRight, &vDir, &vTemp);
    D3DXVec3Normalize(&vRight, &vRight);
    D3DXVec3Cross(&vUp, &vRight, &vDir);
    D3DXVec3Normalize(&vUp, &vUp);

    // 2. 원뿔 안 랜덤 방향
    _float fAngle = ((_float)rand() / RAND_MAX) * m_tParticleDesc.fSpreadAngle;
    _float fAround = ((_float)rand() / RAND_MAX) * D3DX_PI * 2.f;

    D3DXVECTOR3 vFinal = vDir * cosf(fAngle)
        + (vRight * cosf(fAround) + vUp * sinf(fAround)) * sinf(fAngle);
    D3DXVec3Normalize(&vFinal, &vFinal);

    // 3. 속도 크기 랜덤
    _float fSpeed = m_tParticleDesc.fMinSpeed
        + ((_float)rand() / RAND_MAX) * (m_tParticleDesc.fMaxSpeed - m_tParticleDesc.fMinSpeed);

    particle.vVelocity = vFinal * fSpeed;

    // 수명 랜덤
    particle.fMaxLifeTime = m_tParticleDesc.fMinLifeTime
        + ((_float)rand() / RAND_MAX) * (m_tParticleDesc.fMaxLifeTime - m_tParticleDesc.fMinLifeTime);
    particle.fLifeTime = particle.fMaxLifeTime;

    // 크기 랜덤
    particle.fSize = m_tParticleDesc.fMinSize
        + ((_float)rand() / RAND_MAX) * (m_tParticleDesc.fMaxSize - m_tParticleDesc.fMinSize);

    // 색상: 원본 텍스처 모드면 (1,1,1, colorStart.a), 아니면 colorStart
    if (m_tParticleDesc.bUseTextureAsIs)
        particle.color = D3DXCOLOR(1.f, 1.f, 1.f, m_tParticleDesc.colorStart.a);
    else
        particle.color = m_tParticleDesc.colorStart;
}

void CParticleEmitter::Emit_Burst()
{
    for (auto& p : m_vecPool)
        Reset_Particle(p);

    m_bBurstDone = true;
}

CParticleEmitter* CParticleEmitter::Create_Attack(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos, _vec3 vDir, LPDIRECT3DTEXTURE9 pTexture)
{
    ParticleDesc desc;
    ZeroMemory(&desc, sizeof(ParticleDesc));
    desc.vEmitPos = vPos;
    desc.vEmitDir = vDir;  //방향 직접 지정
    desc.fSpreadAngle = D3DX_PI * 0.4f;
    desc.fMinSpeed = 0.1f;     desc.fMaxSpeed = 0.3f;
    desc.fMinLifeTime = 0.5f;  desc.fMaxLifeTime = 0.8f;
    desc.fMinSize = 20.f;      desc.fMaxSize = 60.f;
    desc.colorStart = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
    desc.colorEnd = D3DXCOLOR(1.f, 1.f, 1.f, 0.f);
    desc.bUseTextureAsIs = true;
    desc.iMaxParticles = 10;
    desc.fEmitRate = 0.f;  // Burst
    desc.bLoop = false;
    desc.fGravity = 4.f;

    eParticlePreset eType = PARTICLE_ATTACK;
    return Create(pGraphicDev, eType, desc, pTexture);
}

void CParticleEmitter::Emit_ByRate(const _float& fTimeDelta)
{
    m_fEmitAccTime += fTimeDelta;

    _float fInterval = 1.f / m_tParticleDesc.fEmitRate;

    while (m_fEmitAccTime >= fInterval)
    {
        m_fEmitAccTime -= fInterval;

        // 비활성 슬롯 하나 찾아서 활성화
        for (auto& p : m_vecPool)
        {
            if (!p.bActive)
            {
                Reset_Particle(p);
                break;
            }
        }
    }
}

void CParticleEmitter::Update_Particles(const _float& fTimeDelta)
{
    for (auto& particle : m_vecPool)
    {
        if (!particle.bActive)
            continue;

        //수명 감소
        particle.fLifeTime -= fTimeDelta;
        if (particle.fLifeTime <= 0.f)
        {
            particle.bActive = false;
                    continue;
        }
        //중력
        particle.vVelocity.y -= m_tParticleDesc.fGravity * fTimeDelta;
        //이동
        particle.vPos += particle.vVelocity * fTimeDelta;
        //색상보간
        _float fRatio = 1.f - (particle.fLifeTime / particle.fMaxLifeTime);
        if (m_tParticleDesc.bUseTextureAsIs)
        {
            // 원본 텍스처 그대로: RGB는 1, 알파만 보간(페이드)
            _float fA = m_tParticleDesc.colorStart.a + fRatio * (m_tParticleDesc.colorEnd.a - m_tParticleDesc.colorStart.a);
            particle.color = D3DXCOLOR(1.f, 1.f, 1.f, fA);
        }
        else
            D3DXColorLerp(&particle.color, &m_tParticleDesc.colorStart, &m_tParticleDesc.colorEnd, fRatio);
    }
}

void CParticleEmitter::Render_Particles()
{
    if (!m_pVB)
        return;

    ParticleVertex* pData = nullptr;
    _uint iActiveCnt = 0;

    // D3DLOCK_DISCARD: 이전 데이터 버리고 새로 씀 → GPU와 충돌 없이 빠름
    m_pVB->Lock(0, 0, (void**)&pData, D3DLOCK_DISCARD);

    for (auto& p : m_vecPool)
    {
        if (!p.bActive)
            continue;

        pData[iActiveCnt].vPos = p.vPos;
        pData[iActiveCnt].color = p.color;
        pData[iActiveCnt].fSize = p.fSize;
        ++iActiveCnt;
    }

    m_pVB->Unlock();

    if (iActiveCnt == 0)
        return;

    m_pGraphicDev->SetTexture(0, m_pTexture);

    m_pGraphicDev->SetStreamSource(0, m_pVB, 0, sizeof(ParticleVertex));
    m_pGraphicDev->SetFVF(PARTICLE_FVF);

    m_pGraphicDev->DrawPrimitive(D3DPT_POINTLIST, 0, iActiveCnt);
}

void CParticleEmitter::Set_RenderState()
{
    m_pGraphicDev->SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
    //  POINTSCALEENABLE = FALSE
    //    TRUE면 fSize(픽셀)가 거리로 나눠져서 멀수록 점처럼 보임
    //    FALSE면 fSize가 픽셀 단위 그대로 고정 크기로 출력됨
    m_pGraphicDev->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);

    //switch (m_tParticleDesc.)
    //{
    //default:
    //    break;
    //}

    float fMin = 1.f, fMax = 64.f;

    switch (m_eParticleType)
    {
    case PARTICLE_FOOTSTEP:
        fMin = 1.f; fMax = 8.f;
        break;
    case PARTICLE_ATTACK:
        fMin = 1.f; fMax = 128.f;
        break;
    case PARTICLE_HIT:
        fMin = 1.f; fMax = 8.f;
        break;
    case PARTICLE_FIREWORK:
        fMin = 1.f; fMax = 8.f;
        break;
    case PARTICLE_DYNAMITE:
        fMin = 1.f; fMax = 6.f;
        break;
    case PARTICLE_BOSS_ATTACK:
        fMin = 1.f; fMax = 6.f;
        break;
    }

    m_pGraphicDev->SetRenderState(D3DRS_POINTSIZE_MIN, FtoDW(fMin));
    m_pGraphicDev->SetRenderState(D3DRS_POINTSIZE_MAX, FtoDW(fMax));

    if (m_tParticleDesc.bUseTextureAsIs)
    {
        // ── 원본 텍스처 그대로, 알파 블렌딩 없음 (FOOTSTEP 등)
        m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA); //일반 알파 블렌딩
        m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

        // 알파 테스트 — 알파 0에 가까운 픽셀 완전히 버림
        m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        m_pGraphicDev->SetRenderState(D3DRS_ALPHAREF, 0x10);
        m_pGraphicDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

        m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    }
    else
    {
        // ── Additive 블렌딩 (HIT, FIREWORK, BOSS 등)
        m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        m_pGraphicDev->SetRenderState(D3DRS_ALPHAREF, 0x10);
        m_pGraphicDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

        //ColorOption 다시 초기화해주기
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    }
}

void CParticleEmitter::Reset_RenderState()
{
    m_pGraphicDev->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
    m_pGraphicDev->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

    // TextureStage 원복 — 다른 오브젝트 렌더에 영향 안 주도록
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
}


CParticleEmitter* CParticleEmitter::Create(LPDIRECT3DDEVICE9 pGraphicDev, eParticlePreset& eType,
    const ParticleDesc& desc, LPDIRECT3DTEXTURE9 pTexture)
{
    //Custom Particle
    CParticleEmitter* pEmitter = new CParticleEmitter(pGraphicDev);

    if (FAILED(pEmitter->Ready_Emitter(desc, pTexture, eType)))
    {
        Safe_Release(pEmitter);
        MSG_BOX("ParticleEmitter Create Failed");
        return nullptr;
    }

    return pEmitter;
}

CParticleEmitter* CParticleEmitter::Create(LPDIRECT3DDEVICE9 pGraphicDev,
    eParticlePreset eType, _vec3 vPos, LPDIRECT3DTEXTURE9 pTexture)
{
    ParticleDesc desc;
    ZeroMemory(&desc, sizeof(ParticleDesc));
    desc.vEmitPos = vPos;

    switch (eType)
    {
        // ── 발자국: 아래서 위로 살짝 튀기는 먼지/반짝이
    case PARTICLE_FOOTSTEP:
        desc.vEmitDir = D3DXVECTOR3(0.f, 1.f, 0.f);
        desc.fSpreadAngle = D3DX_PI * 0.4f;
        desc.fMinSpeed = 0.1f;     desc.fMaxSpeed = 0.3f;
        desc.fMinLifeTime = 0.5f;     desc.fMaxLifeTime = 0.8f;
        desc.fMinSize = 0.1f;     desc.fMaxSize = 2.f;   // 픽셀 단위
        desc.colorStart = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
        desc.colorEnd = D3DXCOLOR(1.f, 1.f, 1.f, 0.f);  // 서서히 투명
        desc.bUseTextureAsIs = true;  // 원본 이미지 그대로 (알파만 페이드)
        desc.iMaxParticles = 6;
        desc.fEmitRate = 20.f;      // Burst
        desc.bLoop = false;
        desc.fGravity = 4.f;
        break;
        // ── 검 공격 - 초승달 모양 이펙트 공격
    case PARTICLE_ATTACK:
        desc.vEmitDir = D3DXVECTOR3(0.f, 1.f, 0.f);
        desc.fSpreadAngle = D3DX_PI * 0.4f;
        desc.fMinSpeed = 0.1f;     desc.fMaxSpeed = 0.3f;
        desc.fMinLifeTime = 0.5f;     desc.fMaxLifeTime = 0.8f;
        desc.fMinSize = 20.f;     desc.fMaxSize = 60.f;   // 픽셀 단위
        desc.colorStart = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
        desc.colorEnd = D3DXCOLOR(1.f, 1.f, 1.f, 0.f);  // 서서히 투명
        desc.bUseTextureAsIs = true;  // 원본 이미지 그대로 (알파만 페이드)
        desc.iMaxParticles = 10;
        desc.fEmitRate = 1.f;      // Burst
        desc.bLoop = false;
        desc.fGravity = 4.f;
        break;
        // ── 피격: 전방향으로 터지는 불꽃
    case PARTICLE_HIT:
        desc.vEmitDir = D3DXVECTOR3(0.f, 1.f, 0.f);
        desc.fSpreadAngle = D3DX_PI;  // 전방향
        desc.fMinSpeed = 0.1f;      desc.fMaxSpeed = 0.3f;
        desc.fMinLifeTime = 0.1f;     desc.fMaxLifeTime = 3.4f;
        desc.fMinSize = 1.f;     desc.fMaxSize = 3.f;
        desc.colorStart = D3DXCOLOR(1.f, 1.f, 0.6f, 1.f);
        desc.colorEnd = D3DXCOLOR(1.f, 0.7f, 0.f, 0.f);
        desc.bUseTextureAsIs = true;  // 원본 이미지 그대로 (알파만 페이드)
        desc.iMaxParticles = 10;
        desc.fEmitRate = 0.f;      // Burst
        desc.bLoop = false;
        desc.fGravity = 6.f;
        break;

        // ── 폭죽: 위로 솟구쳤다가 사방으로 퍼짐
    case PARTICLE_FIREWORK:
        desc.vEmitDir = D3DXVECTOR3(0.f, 1.f, 0.f);
        desc.fSpreadAngle = D3DX_PI;
        desc.fMinSpeed = 3.f;      desc.fMaxSpeed = 7.f;
        desc.fMinLifeTime = 0.5f;     desc.fMaxLifeTime = 2.2f;
        desc.fMinSize = 2.f;    desc.fMaxSize = 6.f;
        desc.colorStart = D3DXCOLOR(1.f, 0.8f, 0.f, 1.f);
        desc.colorEnd = D3DXCOLOR(1.f, 0.2f, 0.f, 0.f);
        desc.iMaxParticles = 300;
        desc.fEmitRate = 0.f;      // Burst
        desc.bLoop = true;
        desc.fGravity = 3.f;
        break;

        // ── 보스 공격: 지속 방출, Loop
    case PARTICLE_BOSS_ATTACK:
        desc.vEmitDir = D3DXVECTOR3(0.f, 1.f, 0.f);
        desc.fSpreadAngle = D3DX_PI * 0.6f;
        desc.fMinSpeed = 1.f;      desc.fMaxSpeed = 3.f;
        desc.fMinLifeTime = 0.3f;     desc.fMaxLifeTime = 0.8f;
        desc.fMinSize = 0.15f;    desc.fMaxSize = 0.35f;
        desc.colorStart = D3DXCOLOR(0.5f, 0.f, 1.f, 1.f);
        desc.colorEnd = D3DXCOLOR(0.2f, 0.f, 0.5f, 0.f);
        desc.iMaxParticles = 80;
        desc.fEmitRate = 30.f;     // 초당 30개
        desc.bLoop = true;
        desc.fGravity = 0.f;
        break;
        //TNT
    case PARTICLE_DYNAMITE:
        desc.vEmitDir = D3DXVECTOR3(0.f, 1.f, 0.f);
        desc.fSpreadAngle = D3DX_PI * 0.5f;
        desc.fMinSpeed = 2.f;      desc.fMaxSpeed = 5.f;
        desc.fMinLifeTime = 0.4f;  desc.fMaxLifeTime = 0.8f;
        desc.fMinSize = 40.f;      desc.fMaxSize = 80.f;
        desc.colorStart = D3DXCOLOR(1.f, 0.f, 0.f, 0.9f);
        desc.colorEnd = D3DXCOLOR(1.f, 0.5f, 0.f, 0.f);  
        desc.iMaxParticles = 30;
        desc.fEmitRate = 0.f;
        desc.bLoop = false;
        desc.fGravity = 0.5f;
        break;

    default:
        break;
    }

    return Create(pGraphicDev, eType, desc, pTexture);
}
void CParticleEmitter::Free()
{
    Safe_Release(m_pVB);
    Safe_Release(m_pTexture);

    CGameObject::Free();
}