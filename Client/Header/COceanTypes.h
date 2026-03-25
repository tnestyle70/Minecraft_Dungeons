#pragma once
// _float / _ulong / D3DXVECTOR2 등 — include 순서와 무관하게 OCEAN_DESC가 항상 올바르게 파싱되도록
#include "Engine_Define.h"

#include <complex>
#include <vector>
#include <cmath>

// ============================================================
//  OceanTypes.h
//  파도 시스템 공용 타입 / 수식 정의
//  DX9 기반, Engine/Client 구조에 맞춤
// ============================================================

#ifndef PI
#define PI  3.14159265358979f
#endif
#define GRAVITY 9.81f

// ─── 파도 알고리즘 선택 ──────────────────────────────────────
enum WAVE_TYPE
{
    WAVE_FFT,       // Tessendorf FFT (Phillips 스펙트럼)
    WAVE_SINE,      // 단순 합성 사인파
    WAVE_GERSTNER,  // Gerstner Trochoidal 파도
    WAVE_END
};

// ─── 파도 파라미터 (런타임 조절 가능) ────────────────────────
struct OCEAN_DESC
{
    _ulong  dwVtxCntX = 128;      // 격자 X 해상도 (2의 거듭제곱 권장)
    _ulong  dwVtxCntZ = 128;      // 격자 Z 해상도
    _float  fPatchSize = 512.f;    // 월드 패치 크기 (m)
    _float  fAmplitude = 2.5e-4f;  // Phillips 스펙트럼 A
    _float  fWindSpeed = 12.f;     // 풍속 (m/s)
    _float  fWindDirX = 1.f;      // 풍향 X (정규화)
    _float  fWindDirZ = 0.f;      // 풍향 Z
    _float  fChoppy = 1.2f;     // 수평 변위 스케일 (Choppy)
    _float  fTimeScale = 1.f;      // 시뮬 속도 배율
    WAVE_TYPE eType = WAVE_FFT;
};

// ─── 복소수 타입 ─────────────────────────────────────────────
using Complex = std::complex<float>;

// ─── Gerstner 파 단일 성분 ────────────────────────────────────
struct GERSTNER_WAVE
{
    _float fAmplitude;   // 파고
    _float fWavelength;  // 파장
    _float fSpeed;       // 위상 속도
    _float fSteepness;   // 가파름 Q (0=사인파, 1=최대 Gerstner)
    D3DXVECTOR2 vDir;    // 파 진행 방향 (정규화)
};

// ─── 단순 사인파 성분 ─────────────────────────────────────────
struct SINE_WAVE
{
    _float fAmplitude;
    _float fFrequency;
    _float fPhase;
    _float fSpeed;
    D3DXVECTOR2 vDir;
};

// ─── 수학 유틸 (인라인) ──────────────────────────────────────

// Box-Muller 가우시안 난수
inline Complex GaussianRandom()
{
    static float u1, u2;
    u1 = ((float)rand() / RAND_MAX);
    u2 = ((float)rand() / RAND_MAX);
    if (u1 < 1e-6f) u1 = 1e-6f;
    float r = sqrtf(-2.f * logf(u1));
    return Complex(r * cosf(2.f * PI * u2),
        r * sinf(2.f * PI * u2));
}

// Phillips 스펙트럼
// P(k) = A * exp(-1/(kL)^2) / k^4 * |k^.w^|^2
inline float PhillipsSpectrum(D3DXVECTOR2 k,
    float windSpeed,
    D3DXVECTOR2 windDir,
    float amplitude)
{
    float kLen = D3DXVec2Length(&k);
    if (kLen < 1e-6f) return 0.f;

    float L = windSpeed * windSpeed / GRAVITY;
    float kL = kLen * L;
    float kLen2 = kLen * kLen;
    float kLen4 = kLen2 * kLen2;

    D3DXVECTOR2 kNorm = k / kLen;
    float cosTheta = D3DXVec2Dot(&kNorm, &windDir);

    float ph = amplitude
        * expf(-1.f / (kL * kL))
        / kLen4
        * cosTheta * cosTheta;

    // 역풍 감쇠
    if (cosTheta < 0.f) ph *= 0.125f;

    // 짧은 파 억제 (l = 0.001L)
    float l = L * 0.001f;
    ph *= expf(-kLen2 * l * l);

    return ph;
}

// 분산 관계식
inline float Omega(float k) { return sqrtf(GRAVITY * k); }
