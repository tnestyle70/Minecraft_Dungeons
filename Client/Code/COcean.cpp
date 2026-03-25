#include "pch.h"
#include "COcean.h"
#include "CRenderer.h"

namespace
{
    constexpr int WAVE_FFT_ID = 0;
    constexpr int WAVE_SINE_ID = 1;
    constexpr int WAVE_GERSTNER_ID = 2;

    template <typename T>
    auto ResolvePatchSize(const T& desc, int) -> decltype((void)desc.fPatchSize, float())
    {
        return desc.fPatchSize;
    }

    template <typename T>
    float ResolvePatchSize(const T&, long)
    {
        return 512.f;
    }
}

COcean::COcean(LPDIRECT3DDEVICE9 pGraphicDev)
	:CGameObject(pGraphicDev)
{}

COcean::COcean(const CGameObject & rhs)
	:CGameObject(rhs)
{}

COcean::~COcean()
{}

HRESULT COcean::Ready_GameObject()
{
    if (FAILED(Add_Component()))
        return E_FAIL;

    // N은 VtxCntX 기준 (2의 거듭제곱)
    m_N = m_desc.dwVtxCntX;

    // 출력 배열 초기화
    _ulong total = m_N * m_N;
    m_Heights.assign(total, 0.f);
    m_DisplX.assign(total, 0.f);
    m_DisplZ.assign(total, 0.f);
    m_Normals.assign(total, D3DXVECTOR3(0.f, 1.f, 0.f));

    // 알고리즘별 사전 계산
    switch (static_cast<int>(m_desc.eType))
    {
    case WAVE_FFT_ID:      Init_FFTSpectrum();   break;
    case WAVE_SINE_ID:     Init_SineWaves();     break;
    case WAVE_GERSTNER_ID: Init_GerstnerWaves(); break;
    }

    return S_OK;
}

_int COcean::Update_GameObject(const _float& fTimeDelta)
{
    m_fTime += fTimeDelta * m_fTimeScale;

    // 알고리즘 선택 분기
    switch (static_cast<int>(m_desc.eType))
    {
    case WAVE_FFT_ID:      Tick_FFT(m_fTime);      break;
    case WAVE_SINE_ID:     Tick_Sine(m_fTime);     break;
    case WAVE_GERSTNER_ID: Tick_Gerstner(m_fTime); break;
    }

    // 법선 계산 (공통)
    Compute_Normals();

    // VB 업데이트
    m_pBufferCom->Update_Vertices(
        m_Heights.data(),
        (static_cast<int>(m_desc.eType) != WAVE_SINE_ID) ? m_DisplX.data() : nullptr,
        (static_cast<int>(m_desc.eType) != WAVE_SINE_ID) ? m_DisplZ.data() : nullptr,
        m_Normals.data());

    _int iExit = CGameObject::Update_GameObject(fTimeDelta);
    CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

    return iExit;
}

void COcean::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void COcean::Render_GameObject()
{
    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, TRUE);
    m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransformCom->Get_World());

    if (m_pTextureCom)
        m_pTextureCom->Set_Texture(0);

    if (FAILED(Set_Material()))
        return;

    m_pBufferCom->Render_Buffer();
    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE);
}

HRESULT COcean::Add_Component()
{
    CComponent* pComponent = nullptr;

    // ─── OceanBuffer (동적 VB) ────────────────────────────────
    pComponent = m_pBufferCom = COceanBuffer::Create(m_pGraphicDev, m_desc);
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Buffer", pComponent });

    // ─── Transform ───────────────────────────────────────────
    pComponent = m_pTransformCom = dynamic_cast<CTransform*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

    // ─── Texture (수면 노멀맵/디퓨즈 등) ─────────────────────
    pComponent = m_pTextureCom = dynamic_cast<CTexture*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_OceanTexture"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

    return S_OK;
}

HRESULT COcean::Set_Material()
{
    D3DMATERIAL9 mtrl;
    ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));
    mtrl.Diffuse = D3DXCOLOR(0.05f, 0.25f, 0.45f, 0.85f);
    mtrl.Specular = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
    mtrl.Ambient = D3DXCOLOR(0.02f, 0.08f, 0.15f, 1.f);
    mtrl.Power = 64.f;
    m_pGraphicDev->SetMaterial(&mtrl);
    return S_OK;
}

void COcean::Init_FFTSpectrum()
{
    _ulong N = m_N;
    _ulong total = N * N;

    m_H0.resize(total);
    m_H0Conj.resize(total);
    m_OmegaK.resize(total);

    D3DXVECTOR2 windDir(m_desc.fWindDirX, m_desc.fWindDirZ);
    D3DXVec2Normalize(&windDir, &windDir);

    srand(12345);   // 재현 가능한 난수

    for (_ulong iz = 0; iz < N; ++iz)
    {
        for (_ulong ix = 0; ix < N; ++ix)
        {
            _ulong idx = iz * N + ix;

            // 주파수 인덱스 [-N/2, N/2-1] → 파수 벡터
            int   nx = (int)ix - (int)(N / 2);
            int   nz = (int)iz - (int)(N / 2);
            const float patchSize = ResolvePatchSize(m_desc, 0);
            float kx = (2.f * PI * nx) / patchSize;
            float kz = (2.f * PI * nz) / patchSize;

            D3DXVECTOR2 k(kx, kz);
            float kLen = D3DXVec2Length(&k);

            // Phillips 스펙트럼
            float ph = PhillipsSpectrum(k, m_desc.fWindSpeed,
                windDir, m_desc.fAmplitude);

            // h0(k) = (ξr + iξi) / √2 * √P(k)
            Complex xi = GaussianRandom();
            float   amp = sqrtf(ph * 0.5f);
            m_H0[idx] = xi * amp;

            // conj(h0(-k))
            D3DXVECTOR2 km(-kx, -kz);
            float phm = PhillipsSpectrum(km, m_desc.fWindSpeed,
                windDir, m_desc.fAmplitude);
            Complex xim = GaussianRandom();
            float   ampm = sqrtf(phm * 0.5f);
            m_H0Conj[idx] = std::conj(xim * ampm);

            // ω(k) = √(g|k|) 캐시
            m_OmegaK[idx] = (kLen > 1e-6f) ? Omega(kLen) : 0.f;
        }
    }
}

//FFT 1회 프레임 계산
void COcean::Tick_FFT(float t)
{
    _ulong N = m_N;
    _ulong total = N * N;

    // 시간 스펙트럼: h(k,t) / Dx(k,t) / Dz(k,t)
    std::vector<Complex> Ht(total), Dxt(total), Dzt(total);

    for (_ulong iz = 0; iz < N; ++iz)
    {
        for (_ulong ix = 0; ix < N; ++ix)
        {
            _ulong idx = iz * N + ix;

            int   nx = (int)ix - (int)(N / 2);
            int   nz = (int)iz - (int)(N / 2);
            const float patchSize = ResolvePatchSize(m_desc, 0);
            float kx = (2.f * PI * nx) / patchSize;
            float kz = (2.f * PI * nz) / patchSize;
            float kLen = sqrtf(kx * kx + kz * kz);

            float omega = m_OmegaK[idx];

            // 오일러 회전자 e^{iωt}
            Complex expPos(cosf(omega * t), sinf(omega * t));
            Complex expNeg(cosf(omega * t), -sinf(omega * t));

            // h(k,t) = h0(k)·e^{iωt} + conj(h0(-k))·e^{-iωt}
            Ht[idx] = m_H0[idx] * expPos + m_H0Conj[idx] * expNeg;

            // 수평 변위 스펙트럼
            // Dx(k) = i·(kx/|k|)·h(k,t)
            // Dz(k) = i·(kz/|k|)·h(k,t)
            if (kLen > 1e-6f)
            {
                // i * h = (-imag, real) 즉 복소수 90도 회전
                Complex ih(-Ht[idx].imag(), Ht[idx].real());
                Dxt[idx] = ih * (kx / kLen);
                Dzt[idx] = ih * (kz / kLen);
            }
            else
            {
                Dxt[idx] = Complex(0.f, 0.f);
                Dzt[idx] = Complex(0.f, 0.f);
            }
        }
    }

    // 2D IFFT
    IFFT_2D(Ht, N);
    IFFT_2D(Dxt, N);
    IFFT_2D(Dzt, N);

    // 부호 수정 + 실수부 추출
    // Tessendorf 데이터는 (-1)^(ix+iz) 인수 포함 (DC 중앙화 보정)
    for (_ulong iz = 0; iz < N; ++iz)
    {
        for (_ulong ix = 0; ix < N; ++ix)
        {
            _ulong idx = iz * N + ix;
            float  sign = ((ix + iz) % 2 == 0) ? 1.f : -1.f;

            m_Heights[idx] = Ht[idx].real() * sign;
            m_DisplX[idx] = Dxt[idx].real() * sign * m_desc.fChoppy;
            m_DisplZ[idx] = Dzt[idx].real() * sign * m_desc.fChoppy;
        }
    }
}

void COcean::FFT_1D(std::vector<Complex>& x, bool bInverse)
{
    _ulong N = (_ulong)x.size();

    // 비트반전 순열
    for (_ulong i = 1, j = 0; i < N; ++i)
    {
        _ulong bit = N >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j) std::swap(x[i], x[j]);
    }

    // 나비 연산
    for (_ulong len = 2; len <= N; len <<= 1)
    {
        float   angle = (bInverse ? 2.f : -2.f) * PI / (_float)len;
        Complex wLen(cosf(angle), sinf(angle));

        for (_ulong i = 0; i < N; i += len)
        {
            Complex w(1.f, 0.f);
            for (_ulong k = 0; k < len / 2; ++k)
            {
                Complex u = x[i + k];
                Complex v = x[i + k + len / 2] * w;
                x[i + k] = u + v;
                x[i + k + len / 2] = u - v;
                w *= wLen;
            }
        }
    }

    // IFFT 정규화
    if (bInverse)
    {
        float inv = 1.f / (_float)N;
        for (auto& c : x) c *= inv;
    }
}

void COcean::IFFT_2D(std::vector<Complex>& data, _ulong N)
{
    // 행 방향 IFFT
    std::vector<Complex> row(N);
    for (_ulong iz = 0; iz < N; ++iz)
    {
        for (_ulong ix = 0; ix < N; ++ix)
            row[ix] = data[iz * N + ix];
        FFT_1D(row, true);
        for (_ulong ix = 0; ix < N; ++ix)
            data[iz * N + ix] = row[ix];
    }

    // 열 방향 IFFT
    std::vector<Complex> col(N);
    for (_ulong ix = 0; ix < N; ++ix)
    {
        for (_ulong iz = 0; iz < N; ++iz)
            col[iz] = data[iz * N + ix];
        FFT_1D(col, true);
        for (_ulong iz = 0; iz < N; ++iz)
            data[iz * N + ix] = col[iz];
    }
}

void COcean::Init_SineWaves()
{
    m_SineWaves.clear();

    // 여러 파 성분을 수동으로 설정
    // 실제 게임에서는 파 에디터 UI로 조절
    struct SinePreset { float amp, freq, phase, speed, dx, dz; };
    SinePreset presets[] = {
        { 0.9f, 0.04f, 0.0f,  0.8f,  1.f,  0.0f },
        { 0.6f, 0.07f, 1.2f,  1.2f,  0.7f, 0.3f },
        { 0.3f, 0.12f, 0.7f,  1.8f,  0.5f, 0.5f },
        { 0.2f, 0.18f, 2.1f,  2.5f,  0.3f, 0.7f },
        { 0.3f,0.25f, 3.5f,  3.0f, -0.2f, 0.9f },
        { 0.15f, 0.35f, 0.3f,  4.0f, -0.6f, 0.4f },
    };

    for (auto& p : presets)
    {
        SINE_WAVE w;
        w.fAmplitude = p.amp * 2.f * (m_desc.fWindSpeed * 0.1f);
        w.fFrequency = p.freq;
        w.fPhase = p.phase;
        w.fSpeed = p.speed;
        w.vDir = D3DXVECTOR2(p.dx, p.dz);
        D3DXVec2Normalize(&w.vDir, &w.vDir);
        m_SineWaves.push_back(w);
    }
}

void COcean::Tick_Sine(float t)
{
    _ulong N = m_N;
    const float patchSize = ResolvePatchSize(m_desc, 0);
    float  step = patchSize / (_float)(N - 1);
    float  off = -patchSize * 0.5f;

    for (_ulong iz = 0; iz < N; ++iz)
    {
        for (_ulong ix = 0; ix < N; ++ix)
        {
            _ulong idx = iz * N + ix;
            float  x = off + ix * step;
            float  z = off + iz * step;
            float  h = 0.f;

            for (auto& w : m_SineWaves)
            {
                // 위상 = k·(d·p) + speed·t
                // d·p = 방향 벡터와 위치의 내적
                float dp = w.vDir.x * x + w.vDir.y * z;
                float phase = w.fFrequency * dp + w.fSpeed * t + w.fPhase;
                h += w.fAmplitude * sinf(phase);
            }

            m_Heights[idx] = h;
            m_DisplX[idx] = 0.f;
            m_DisplZ[idx] = 0.f;
        }
    }
}

// ============================================================
//  알고리즘 3: Gerstner Trochoidal 파도
//  참조: GPU Gems ch1 (Finch, 2004)
//
//  각 파 성분에서 정점이 원형 궤도 운동:
//    x'  = x - Q·A·Dx·sin(k·d·P + ω·t)
//    y'  = A·cos(k·d·P + ω·t)
//    z'  = z - Q·A·Dz·sin(k·d·P + ω·t)
//
//  Q = Steepness / (ω · A · numWaves)  → 원형 정도
// ============================================================

void COcean::Init_GerstnerWaves()
{
    m_GerstnerWaves.clear();

    struct GPreset { float amp, wl, speed, steep, dx, dz; };
    GPreset presets[] = {
        { 1.8f,  80.f, 1.0f, 0.8f,  1.0f,  0.0f },
        { 1.2f,  50.f, 1.4f, 0.7f,  0.8f,  0.3f },
        { 0.8f,  30.f, 1.9f, 0.6f,  0.5f,  0.6f },
        { 0.5f,  18.f, 2.5f, 0.5f,  0.2f,  0.9f },
        { 0.35f, 10.f, 3.2f, 0.4f, -0.3f,  0.8f },
        { 0.2f,   6.f, 4.0f, 0.3f, -0.7f,  0.5f },
    };

    float windMul = m_desc.fWindSpeed / 12.f;

    for (auto& p : presets)
    {
        GERSTNER_WAVE w;
        w.fAmplitude = p.amp * windMul;
        w.fWavelength = p.wl;
        w.fSpeed = p.speed;
        w.fSteepness = p.steep;
        w.vDir = D3DXVECTOR2(p.dx, p.dz);
        D3DXVec2Normalize(&w.vDir, &w.vDir);
        m_GerstnerWaves.push_back(w);
    }
}

void COcean::Tick_Gerstner(float t)
{
    _ulong N = m_N;
    const float patchSize = ResolvePatchSize(m_desc, 0);
    float  step = patchSize / (_float)(N - 1);
    float  off = -patchSize * 0.5f;
    _ulong numW = (_ulong)m_GerstnerWaves.size();

    for (_ulong iz = 0; iz < N; ++iz)
    {
        for (_ulong ix = 0; ix < N; ++ix)
        {
            _ulong idx = iz * N + ix;
            float  bx = off + ix * step;
            float  bz = off + iz * step;

            float dX = 0.f, dY = 0.f, dZ = 0.f;

            for (auto& w : m_GerstnerWaves)
            {
                // 파수 k = 2π / λ
                float k = (2.f * PI) / w.fWavelength;
                // 각속도 ω = √(g·k)
                float omega = Omega(k);
                // 위상 속도 조정된 각도
                float phase = k * (w.vDir.x * bx + w.vDir.y * bz)
                    + omega * w.fSpeed * t;

                float cosP = cosf(phase);
                float sinP = sinf(phase);

                // Steepness를 파 수로 나눠 수평 변위 과적용 방지
                float Q = w.fSteepness / (k * w.fAmplitude * (_float)numW + 1e-6f);

                // Gerstner 변위 공식
                dX += -Q * w.fAmplitude * w.vDir.x * sinP;
                dZ += -Q * w.fAmplitude * w.vDir.y * sinP;
                dY += w.fAmplitude * cosP;
            }

            m_Heights[idx] = dY;
            m_DisplX[idx] = dX * m_desc.fChoppy;
            m_DisplZ[idx] = dZ * m_desc.fChoppy;
        }
    }
}

//법선 계산 중앙 차분 : N = normalize((-dH/dx, 1, -dH/dz))
float COcean::Sample_Height(_ulong ix, _ulong iz) const
{
    //래핑(타일링)
    _ulong N = m_N;
    // 음수 인덱스 보정: (a % N + N) % N  (오타로 N%N 만 있으면 항상 0이라 래핑 깨짐)
    int iix = (int)ix;
    int iiz = (int)iz;
    _ulong sx = (_ulong)((iix % (int)N + (int)N) % (int)N);
    _ulong sz = (_ulong)((iiz % (int)N + (int)N) % (int)N);
    return m_Heights[sz * N + sx];
}

void COcean::Compute_Normals()
{
    _ulong N = m_N;
    const float patchSize = ResolvePatchSize(m_desc, 0);
    float  step = patchSize / (_float)(N - 1);
    float  inv2 = 0.5f / step;  // 1 / (2 * step) = 중앙 차분 계수

    for (_ulong iz = 0; iz < N; ++iz)
    {
        for (_ulong ix = 0; ix < N; ++ix)
        {
            _ulong idx = iz * N + ix;

            float hL = Sample_Height(ix - 1, iz);
            float hR = Sample_Height(ix + 1, iz);
            float hD = Sample_Height(ix, iz - 1);
            float hU = Sample_Height(ix, iz + 1);

            D3DXVECTOR3 n(
                -(hR - hL) * inv2,
                1.f,
                -(hU - hD) * inv2);

            D3DXVec3Normalize(&m_Normals[idx], &n);
        }
    }
}

//런타임 파라미터 변경, 알고리즘 전환 시 사전 계산 재실행
void COcean::Set_WaveType(int iType)
{
    if (m_desc.eType == (WAVE_TYPE)iType)
        return;
    m_desc.eType = (WAVE_TYPE)iType;
    m_fTime = 0.f;

    switch (static_cast<int>(iType))
    {
    case WAVE_FFT_ID:
        Init_FFTSpectrum();
        break;
    case WAVE_SINE_ID:
        Init_SineWaves();
        break;
    case WAVE_GERSTNER_ID:
        Init_GerstnerWaves();
        break;
    }
}

void COcean::Change_Wave()
{
    
}

void COcean::Set_WindSpeed(float fSpeed)
{
    m_desc.fWindSpeed = fSpeed;
    if (static_cast<int>(m_desc.eType) == WAVE_FFT_ID)
        Init_FFTSpectrum();
    if (static_cast<int>(m_desc.eType) == WAVE_GERSTNER_ID)
        Init_GerstnerWaves();
}

void COcean::Set_WindDir(float dx, float dz)
{
    m_desc.fWindDirX = dx;
    m_desc.fWindDirZ = dz;
    if (static_cast<int>(m_desc.eType) == WAVE_FFT_ID) Init_FFTSpectrum();
}

void COcean::Set_Amplitude(float fAmp)
{
    m_desc.fAmplitude = fAmp;
    if (static_cast<int>(m_desc.eType) == WAVE_FFT_ID) Init_FFTSpectrum();
}

COcean* COcean::Create(LPDIRECT3DDEVICE9 pGraphicDev, const OCEAN_DESC& desc)
{
    COcean* pOcean = new COcean(pGraphicDev);
    pOcean->m_desc = desc;

    if (FAILED(pOcean->Ready_GameObject()))
    {
        Safe_Release(pOcean);
        MSG_BOX("COcean Create Failed");
        return nullptr;
    }
    return pOcean;
}

void COcean::Free()
{
    CGameObject::Free();
}