#pragma once
#include "CCamera.h"
#include <deque>
#include "CTransform.h"

//Idea - deque에 waypoint를 저장하고 스테이지에서 위치값 뽑아내면서
//쭉 훑고 deque가 비면 캠 전환
struct CamWayPoint
{
	_vec3 vEye;
	_vec3 vUp;
	_float fDuration;
};

class CDynamicCamera :  public CCamera
{
private:
	explicit CDynamicCamera(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CDynamicCamera(const CDynamicCamera& rhs);
	virtual ~CDynamicCamera();

public:
	HRESULT		Ready_GameObject(const _vec3* pEye,
								 const _vec3* pAt,
								 const _vec3* pUp,
								 const _float& fFov,
								 const _float & fAspect,
								 const _float & fNear,
								 const _float & fFar);

	virtual _int	Update_GameObject(const _float& fTimeDelta);
	virtual void	LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void	Render_GameObject() {}

private:
	_int Update_ActionCam(const _float& fTimeDelta);

private:
	void		Key_Input(const _float& fTimeDelta);
	void		Mouse_Move();
	void		Mouse_Fix();
public:
	void SetActionCam();
	bool IsActionCamFinished() { return m_bActionCam; }
private:
	_float		m_fSpeed;
	_bool		m_bFix;
	_bool		m_bCheck;
private: //Cam Way Point
	deque<CamWayPoint> m_deqWayPoints;
	CamWayPoint m_wpStart;
	CamWayPoint m_wpTarget;
	CamWayPoint m_wpEnd;
	bool m_bActionCam = false;
public:
	static CDynamicCamera* Create(LPDIRECT3DDEVICE9 pGraphicDev,
									const _vec3* pEye,
									const _vec3* pAt,
									const _vec3* pUp,
									const _float& fFov = D3DXToRadian(60.f),
									const _float& fAspect = (_float)WINCX / WINCY,
									const _float& fNear = 0.1f,
									const _float& fFar = 1000.f);

private:
	virtual void		Free();



//플레이어에게 카메라 고정
public:
	void SetFollowTarget(Engine::CTransform* pTransform) { m_pTargetTransform = pTransform; }

private:
	Engine::CTransform* m_pTargetTransform = nullptr;
	_vec3 m_vFollowOffset = { 0.f, 15.f, -12.f };  // 등뒤 위치 오프셋

	//자유카메라 <-> 고정카메라
	bool m_bFollowMode = true;
	bool m_bF2Check = false;
};

