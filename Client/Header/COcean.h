#pragma once
#include "CGameObject.h"
#include "COceanBuffer.h"
#include "COceanTypes.h"

// ============================================================
//  COcean : CGameObject
//
//  파도 알고리즘 3종을 런타임에 전환 가능:
//    WAVE_FFT      : Tessendorf (2001) CPU FFT
//    WAVE_SINE     : 합성 사인파 (단순, 저비용)
//    WAVE_GERSTNER : Gerstner Trochoidal (Choppy 극대화)
//
//  사용법:
//    COcean* pOcean = COcean::Create(pDev, desc);
//    pOcean->Set_WaveType(WAVE_GERSTNER);
//    // 매 프레임
//    pOcean->Update_GameObject(fDelta);
//    pOcean->Render_GameObject();
// ============================================================

class COcean : public CGameObject
{
public:
    explicit COcean(LPDIRECT3DDEVICE9 pGraphicDev);
    COcean(const CGameObject& rhs);
    virtual ~COcean();

public:
    // ─── CGameObject 오버라이드 ──────────────────────────────
    virtual HRESULT Ready_GameObject()       override;
    virtual _int    Update_GameObject(const _float& fTimeDelta) override;
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta) override;
    virtual void    Render_GameObject()      override;

public:
    // ─── 런타임 파라미터 조작 ────────────────────────────────
    void Set_WaveType(int iType);
    void Change_Wave();
    void Set_WindSpeed(float fSpeed);
    void Set_WindDir(float dx, float dz);
    void Set_Amplitude(float fAmp);
    void Set_TimeScale(float fScale) { m_fTimeScale = fScale; }
    void Set_Choppy(float fChoppy) { m_desc.fChoppy = fChoppy; }
    WAVE_TYPE Get_WaveType() const { return m_desc.eType; }
    const OCEAN_DESC& Get_Desc() const { return m_desc; }

public:
    static COcean* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        const OCEAN_DESC& desc);
    virtual void Free() override;

private:
    // ─── 초기화 ──────────────────────────────────────────────
    HRESULT Add_Component();
    HRESULT Set_Material();
    void    Init_FFTSpectrum();   // h0(k) 사전 계산
    void    Init_SineWaves();
    void    Init_GerstnerWaves();

    // ─── 알고리즘별 업데이트 ─────────────────────────────────
    void Tick_FFT(float t);
    void Tick_Sine(float t);
    void Tick_Gerstner(float t);

    // ─── CPU FFT (Cooley-Tukey Iterative) ────────────────────
    //  in-place, 1D, 비트반전 DIT
    void FFT_1D(std::vector<Complex>& x, bool bInverse);
    // 2D IFFT: 행 방향 FFT → 열 방향 FFT
    void IFFT_2D(std::vector<Complex>& data, _ulong N);

    // ─── 노멀 계산 ────────────────────────────────────────────
    void Compute_Normals();
    // 중앙 차분으로 인접 정점 높이로 노멀 추정
    float Sample_Height(_ulong ix, _ulong iz) const;

private:
    // ─── 컴포넌트 참조 ───────────────────────────────────────
    COceanBuffer* m_pBufferCom = nullptr;
    CTransform* m_pTransformCom = nullptr;
    CTexture* m_pTextureCom = nullptr;

    // ─── 파라미터 ─────────────────────────────────────────────
    OCEAN_DESC      m_desc{};
    float           m_fTime = 0.f;
    float           m_fTimeScale = 1.f;

    // ─── FFT 전용 ────────────────────────────────────────────
    _ulong          m_N = 128;  // 격자 크기 (2의 거듭제곱)
    std::vector<Complex> m_H0;      // 초기 스펙트럼 h0(k)
    std::vector<Complex> m_H0Conj;  // conj(h0(-k))
    std::vector<float>   m_OmegaK;  // 각주파수 ω(k) 캐시

    // ─── 출력 배열 (VB 업데이트용) ───────────────────────────
    std::vector<float>       m_Heights;   // 높이
    std::vector<float>       m_DisplX;    // 수평 변위 X
    std::vector<float>       m_DisplZ;    // 수평 변위 Z
    std::vector<D3DXVECTOR3> m_Normals;   // 법선

    // ─── Sine 파 정의 ─────────────────────────────────────────
    std::vector<SINE_WAVE>    m_SineWaves;

    // ─── Gerstner 파 정의 ────────────────────────────────────
    std::vector<GERSTNER_WAVE> m_GerstnerWaves;
};
