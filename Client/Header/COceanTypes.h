#pragma once
#include <complex>
#include <vector>
#include <cmath>
#include "Engine_Define.h"

static constexpr float PI = 3.141592f;
static constexpr float GRAVITY = 9.8f;

//파도 시스템 공용 타입

//파도 알고리즘
enum WAVE_TYPE
{
	WAVE_FFT, //FFT
	WAVE_SINE, //합성 사인파
	WAVE_GERSTNER, //거스트너
	WAVE_END
};

//파도 파라미터
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

//복소수 타입 - 2D의 회전, 쿼터니언은 3D의 회전 e^(i ceta) = cos ceta * i sin ceta
using Complex = std::complex<float>;

//사인파
struct SINE_WAVE
{
    float fAmplitude; //진폭?
    float fFrequency; //주기?
    float fPhase; //위상
    float fSpeed; //속도 
};

//Box_Muller 가우시안 난수
inline Complex GaussianRandom()
{
    static float u1, u2;
    u1 = (float)rand() / RAND_MAX;
    u2 = (float)rand() / RAND_MAX;
    if (u1 < 1e-6f) u1 = 1e-6f;
    float r = sqrtf(-2.f * logf(u1));
    return Complex(r * cosf(2.f * PI * u2),
        r * sinf(2.f * PI * u2));
}

//Philips 스펙트럼
//p(K) = A * exp(-1/(kL)^2 / k^4 * |k^/w^|^2)
inline float PhilipsSpectrum(_vec2 k, float windSpeed, _vec2 windDir, float amplitude)
{
    float kLen = D3DXVec2Length(&k);
    if (kLen < 1e-6f) return 0.f;
    
    float L = windSpeed * windSpeed / GRAVITY;
    float kL = kLen * L;
    float kLen2 = kLen * kLen;
    float kLen4 = kLen2 * kLen2;

    _vec2 kNorm = k / kLen;
  
}

