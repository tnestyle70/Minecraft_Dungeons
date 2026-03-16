#pragma once
#include "Engine_Define.h"

//파티클 프리셋 종류 
enum eParticlePreset
{
	PARTICLE_FOOTSTEP,
	PARTICLE_ATTACK,
	PARTICLE_HIT,
	PARTICLE_FIREWORK,
	PARTICLE_DYNAMITE,
	PARTICLE_BOSS_ATTACK,
	PARTICLE_END
};

//Emitter 설정 - 파티클 설정
struct ParticleDesc
{
	//생성 위치, 방향
	_vec3 vEmitPos;
	_vec3 vEmitDir; //기준 방항(정규화)
	_float fSpreadAngle;//퍼짐 각도(라디안)
	//속도 범위 - 랜덤
	_float fMinSpeed;
	_float fMaxSpeed;
	//수평 범위(랜덤)
	_float fMinLifeTime;
	_float fMaxLifeTime;
	//크기 (포인트 스프라이트 크기, 단위 월드 스페이스)
	_float fMinSize;
	_float fMaxSize;
	//색상 - 수명 비율에 따른 보간
	D3DXCOLOR colorStart;
	D3DXCOLOR colorEnd;
	// true면 버텍스 색으로 텍스처를 틴트하지 않음 → 원본 이미지 그대로 (알파만 보간)
	_bool bUseTextureAsIs;
	//이미터 동작
	_int iMaxParticles; //물 크기(한 번에 최대 몇 개)
	_float fEmitRate; //초당 생성 개수, 0이면 start시 전부 생성
	_bool bLoop;
	//물리
	_float fGravity;
};

//Particle Runtime Data
struct Particle
{
	_vec3 vPos;
	_vec3 vVelocity;
	D3DXCOLOR color;
	_float fSize;
	_float fLifeTime;
	_float fMaxLifeTime;
	_bool bActive;
};

//동적 VB에 쓸 버텍스 구조체 
#define PARTICLE_FVF (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_PSIZE)
struct ParticleVertex
{
	D3DXVECTOR3 vPos;
	D3DCOLOR color;
	_float fSize;
};