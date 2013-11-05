//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose:
//
//===========================================================================//

#ifndef PHYSICS_AIRBOAT_H
#define PHYSICS_AIRBOAT_H
#ifdef _WIN32
#pragma once
#endif

#include "vphysics_interface.h"
#include "vector.h"
#include "ivp_physics.hxx"
#include "ivp_controller.hxx"
#include "ivp_car_system.hxx"
#include "ivp_controller_airboat.h"

class IPhysicsObject;

//=============================================================================
//
// Raycast Airboat System
//
class CPhysics_Airboat : public IVP_Controller_Raycast_Airboat 
{

public:

    CPhysics_Airboat( IVP_Environment *env, const IVP_Template_Car_System *t, IPhysicsGameTrace *pGameTrace );
    virtual ~CPhysics_Airboat();

	virtual void do_raycasts_gameside( int nRaycastCount, IVP_Ray_Solver_Template *pRays, IVP_Raycast_Airboat_Impact *pImpacts );

    void update_wheel_positions( void );
	void SetWheelFriction( int iWheel, float flFriction );

	IPhysicsObject *GetWheel( int index );
	float GetWaterDepth( Ray_t *pGameRay, IPhysicsObject *pObject );

	virtual void DoSimulationDrag( IVP_Raycast_Airboat_Pontoon_Temp *pTempPontoons, IVP_Raycast_Airboat_Impact *pImpacts, IVP_Event_Sim *pEventSim );
	float ComputeFrontPontoonWaveNoise( int iPoint, float flSpeed );
	void PerformFrictionNotification( float flEnergy, float flDeltaTime, int surfacePropsHit, IPhysicsCollisionData *pData );

protected:

	void InitAirboat( const IVP_Template_Car_System *pCarSystem );

	void pre_raycasts_gameside( int nRaycastCount, IVP_Ray_Solver_Template *pRays, Ray_t *pGameRays, IVP_Raycast_Airboat_Impact *pImpacts );

    IVP_Real_Object		*m_pWheels[IVP_RAYCAST_AIRBOAT_MAX_WHEELS];
	IPhysicsGameTrace	*m_pGameTrace;
};

class CAirboatFrictionData : public IPhysicsCollisionData
{
	friend class CPhysics_Airboat;
public:
	CAirboatFrictionData();
	void GetSurfaceNormal(Vector &out) { out = m_SurfaceNormal; }
	void GetContactPoint(Vector &out) { out = m_ContactPoint; }
	void GetContactSpeed(Vector &out) { out = m_ContactSpeed; }
protected:
	Vector m_SurfaceNormal;
	Vector m_ContactPoint;
	Vector m_ContactSpeed;
};

#endif // PHYSICS_AIRBOAT_H
