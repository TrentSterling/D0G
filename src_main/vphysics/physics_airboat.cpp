//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose:
//
//===========================================================================//

#include <cstd/string.h>
#include "tier0/dbg.h"
#include "physics_airboat.h"
#include "physics_environment.h"
#include "physics_material.h"
#include "physics_object.h"
#include "ivp_material.hxx"
#include "ivp_ray_solver.hxx"
#include "ivp_cache_object.hxx"
#include "cmodel.h"
#include "convert.h"

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CPhysics_Airboat::CPhysics_Airboat( IVP_Environment *pEnv, const IVP_Template_Car_System *pCarSystem,
								    IPhysicsGameTrace *pGameTrace )
									: IVP_Controller_Raycast_Airboat( pEnv, pCarSystem )
{
	InitAirboat( pCarSystem );
	m_pGameTrace = pGameTrace;
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
CPhysics_Airboat::~CPhysics_Airboat()
{
}

//-----------------------------------------------------------------------------
// Purpose: Setup the car system wheels.
//-----------------------------------------------------------------------------
void CPhysics_Airboat::InitAirboat( const IVP_Template_Car_System *pCarSystem )
{
	for ( int iWheel = 0; iWheel < pCarSystem->n_wheels; ++iWheel )
	{
		m_pWheels[iWheel] = pCarSystem->car_wheel[iWheel];
		m_pWheels[iWheel]->enable_collision_detection( IVP_FALSE );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get the raycast wheel.
//-----------------------------------------------------------------------------
IPhysicsObject *CPhysics_Airboat::GetWheel( int index )
{
	Assert( index >= 0 );
	Assert( index < n_wheels );

	return ( IPhysicsObject* )m_pWheels[index]->client_data;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPhysics_Airboat::SetWheelFriction( int iWheel, float flFriction )
{
	change_friction_of_wheel( IVP_POS_WHEEL( iWheel ), flFriction );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CPhysics_Airboat::GetWaterDepth(Ray_t *pGameRay, IPhysicsObject *pObject)
{
	Ray_t ray;
	Vector end;
	VectorCopy(pGameRay->m_Start, end);
	end[2] -= 1000.0f;
	ray.Init(pGameRay->m_Start, end);
	trace_t trace;
	m_pGameTrace->VehicleTraceRayWithWater(ray, pObject->GetGameData(), &trace);
	return trace.fraction * 1000.0f;
}

//-----------------------------------------------------------------------------
// Purpose:: Convert data to HL2 measurements, and test direction of raycast.
//-----------------------------------------------------------------------------
void CPhysics_Airboat::pre_raycasts_gameside( int nRaycastCount, IVP_Ray_Solver_Template *pRays,
											  Ray_t *pGameRays, IVP_Raycast_Airboat_Impact *pImpacts )
{
	IVP_Core *pAirboatCore = m_pAirboatBody->get_core();

	float flVerticalSpeed = m_LocalVelocity.k[2] * 0.1f;
	if (flVerticalSpeed < 0.0f)
		flVerticalSpeed = 0.0f;
	else if (flVerticalSpeed > 1.0f)
		flVerticalSpeed = 1.0f;

	float flSpeed = pAirboatCore->speed.real_length() * (1.0f / 15.0f);
	if (flSpeed < 0.0f)
		flSpeed = 0.0f;
	if (flSpeed > 1.0f)
		flSpeed = 1.0f;

	if (m_flThrust != 0.0f)
		flVerticalSpeed *= 0.5f;

	int iRaycast;
	IVP_Ray_Solver_Template *pRay = pRays;
	Ray_t *pGameRay = pGameRays;
	Vector vecStart[IVP_RAYCAST_AIRBOAT_MAX_WHEELS], vecDirection[IVP_RAYCAST_AIRBOAT_MAX_WHEELS], vecEnd;
	Vector *pVecStart = vecStart, *pVecDirection = vecDirection;
	int iFrontInWater = 0;

	for (iRaycast = 0; iRaycast < nRaycastCount; ++iRaycast, ++pRay, ++pGameRay, ++pImpacts, ++pVecStart, ++pVecDirection)
	{
		// Setup the ray.
		ConvertPositionToHL(pRay->ray_start_point, *pVecStart);
		ConvertDirectionToHL(pRay->ray_normized_direction, *pVecDirection);

		// Check to see if that point is in water.
		if (m_pGameTrace->VehiclePointInWater(*pVecStart))
		{
			pVecDirection->Negate();
			pImpacts->bInWater = IVP_TRUE;
		}
		else
		{
			pImpacts->bInWater = IVP_FALSE;
		}

		vecEnd = (*pVecStart) + (*pVecDirection) * IVP2HL(pRay->ray_length);

		// Shorten the trace.
		if (m_pGameTrace->VehiclePointInWater(vecEnd))
		{
			if (iRaycast <= 1)
			{
				++iFrontInWater;
				pRay->ray_length = ComputeFrontPontoonWaveNoise(iRaycast, flSpeed) + AIRBOAT_RAYCAST_DIST_WATER;
			}
			else
			{
				pRay->ray_length = AIRBOAT_RAYCAST_DIST_WATER;
				vecEnd = (*pVecStart) + (*pVecDirection) * IVP2HL(AIRBOAT_RAYCAST_DIST_WATER);
			}
		}

		pGameRays->Init(*pVecStart, vecEnd);
	}

	if (iFrontInWater == 2)
	{
		float flRayLength;
		float flRayOffset = flVerticalSpeed * 0.25f + AIRBOAT_RAYCAST_DIST_WATER;

		flRayLength = ComputeFrontPontoonWaveNoise(0, flSpeed) + flRayOffset;
		pRays[0].ray_length = flRayLength;
		pGameRays[0].Init(vecStart[0], vecStart[0] + vecDirection[0] * IVP2HL(flRayLength));

		flRayLength = ComputeFrontPontoonWaveNoise(1, flSpeed) + flRayOffset;
		pRays[1].ray_length = flRayLength;
		pGameRays[1].Init(vecStart[1], vecStart[1] + vecDirection[1] * IVP2HL(flRayLength));
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPhysics_Airboat::do_raycasts_gameside( int nRaycastCount, IVP_Ray_Solver_Template *pRays,
										     IVP_Raycast_Airboat_Impact *pImpacts )
{
	Assert( nRaycastCount >= 0 );
	Assert( nRaycastCount <= IVP_RAYCAST_AIRBOAT_MAX_WHEELS );

	Ray_t gameRays[IVP_RAYCAST_AIRBOAT_MAX_WHEELS];
	pre_raycasts_gameside( nRaycastCount, pRays, gameRays, pImpacts );

	// Do the raycasts and set impact data.
	IVP_Raycast_Airboat_Impact *pImpact = pImpacts;
	trace_t trace;
	for ( int iRaycast = 0; iRaycast < nRaycastCount; ++iRaycast, ++pImpact )
	{
		// Trace.
		if ( pImpact->bInWater )
		{
			IPhysicsObject *pPhysAirboat = static_cast<IPhysicsObject*>( m_pAirboatBody->client_data );
			m_pGameTrace->VehicleTraceRay( gameRays[iRaycast], pPhysAirboat->GetGameData(), &trace ); 
		}
		else
		{
			IPhysicsObject *pPhysAirboat = static_cast<IPhysicsObject*>( m_pAirboatBody->client_data );
			m_pGameTrace->VehicleTraceRayWithWater( gameRays[iRaycast], pPhysAirboat->GetGameData(), &trace );
			pImpact->flWaterDepth = GetWaterDepth(gameRays + iRaycast, pPhysAirboat);
		}
		
		// Set impact data.
		if ( trace.fraction == 1.0f )
		{
			pImpact->bImpact = IVP_FALSE;
		}
		else
		{
			pImpact->bImpact = IVP_TRUE;

			// Set water surface flag.
			if ( trace.contents & MASK_WATER )
			{
				pImpact->bImpactWater = IVP_TRUE;
			}
			else
			{
				pImpact->bImpactWater = IVP_FALSE;
				pImpact->flWaterDepth = 0.0f;
			}

			// Save impact surface data.
			ConvertPositionToIVP( trace.endpos, pImpact->vecImpactPointWS );
			ConvertDirectionToIVP( trace.plane.normal, pImpact->vecImpactNormalWS );

			// Save surface properties.
			surfacedata_t *pSurfaceData = physprops->GetSurfaceData( trace.surface.surfaceProps );
			pImpact->surfaceProps = trace.surface.surfaceProps;
			pImpact->flDampening = pSurfaceData->physics.dampening;
			pImpact->flFriction = pSurfaceData->physics.friction;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPhysics_Airboat::update_wheel_positions( void )
{
	return;
}

//-----------------------------------------------------------------------------
// Purpose: Water drag.
//-----------------------------------------------------------------------------
void CPhysics_Airboat::DoSimulationDrag(IVP_Raycast_Airboat_Pontoon_Temp *pTempPontoons,
                                        IVP_Raycast_Airboat_Impact *pImpacts, IVP_Event_Sim *pEventSim)
{
	IVP_Core *pAirboatCore = m_pAirboatBody->get_core();
	CAirboatFrictionData frictionData;

	IVP_U_Float_Point *vecSpeed = &(pAirboatCore->speed);
	ConvertDirectionToHL(*vecSpeed, frictionData.m_ContactSpeed);

	int nFrictions = 0;
	Vector vecTemp;
	float flFriction = 0.0f;
	bool bImpactWater = false;

	int surfaceProps[IVP_RAYCAST_AIRBOAT_MAX_WHEELS];
	int surfacePropsSet[IVP_RAYCAST_AIRBOAT_MAX_WHEELS];
	int iSurfaceProps = 0;
	int iFrictionSurfaceProps = 0;
	int i;
	memset(surfaceProps, 0xff, sizeof(surfaceProps));
	memset(surfacePropsSet, 0, sizeof(surfacePropsSet));

	int iPoint;
	for (iPoint = 0; iPoint < n_wheels; ++iPoint, ++pTempPontoons, ++pImpacts)
	{
		if (!(pImpacts->bImpact))
			continue;
		if (pImpacts->bImpactWater)
		{
			bImpactWater = true;
			continue;
		}
		++nFrictions;
		flFriction += pImpacts->flFriction;

		for (i = 0; i < iSurfaceProps; ++i)
		{
			if (surfaceProps[i] == pImpacts->surfaceProps)
				break;
		}
		if (i >= iSurfaceProps)
			++iSurfaceProps;
		surfaceProps[i] = pImpacts->surfaceProps;
		++(surfacePropsSet[i]);
		if (surfacePropsSet[i] > surfacePropsSet[iFrictionSurfaceProps])
			iFrictionSurfaceProps = i;

		ConvertPositionToHL(pTempPontoons->ground_hit_ws, vecTemp);
		frictionData.m_ContactPoint += vecTemp;
		ConvertDirectionToHL(pTempPontoons->ground_normal_ws, vecTemp);
		frictionData.m_SurfaceNormal += vecTemp;
	}

	if (nFrictions)
	{
		float flInvFrictions = 1.0f / (float)nFrictions;
		frictionData.m_ContactPoint *= flInvFrictions;
		frictionData.m_SurfaceNormal *= flInvFrictions;
		VectorNormalize(frictionData.m_SurfaceNormal);
	}

	IVP_U_Float_Point vecImpulse;
	IVP_U_Float_Point vecLocalDrag;
	float flMass = pAirboatCore->get_mass();
	float flSpeed = vecSpeed->real_length();
	const IVP_U_Matrix *matWorldFromCore = pAirboatCore->get_m_world_f_core_PSI();

	if (bImpactWater)
	{
		vecLocalDrag.k[0] = m_LocalVelocity.k[0] * -0.6f;
		vecLocalDrag.k[1] = m_LocalVelocity.k[1] * -0.0025f;
		vecLocalDrag.k[2] = m_LocalVelocity.k[2] * -0.005f;
		vecLocalDrag.mult(flMass * flSpeed * pEventSim->delta_time);
		matWorldFromCore->vmult3(&vecLocalDrag, &vecImpulse);
		pAirboatCore->center_push_core_multiple_ws(&vecImpulse);
	}

	if (nFrictions && (flSpeed > 0.0f))
	{
		IPhysicsObject *pPhysAirboat = static_cast<IPhysicsObject *>(m_pAirboatBody->client_data);
		float flEnergy = pPhysAirboat->GetEnergy();
		vecLocalDrag.set_multiple(&m_LocalVelocity,
			(((flFriction / (float)nFrictions) * flMass * (-0.6f * AIRBOAT_GRAVITY)) / flSpeed) * pEventSim->delta_time);
		vecLocalDrag.k[0] *= 2.0f;
		vecLocalDrag.k[1] *= 0.8f;
		matWorldFromCore->vmult3(&vecLocalDrag, &vecImpulse);
		pAirboatCore->center_push_core_multiple_ws(&vecImpulse);
		PerformFrictionNotification(flEnergy - pPhysAirboat->GetEnergy(), pEventSim->delta_time,
			surfaceProps[iFrictionSurfaceProps], &frictionData);
	}
}

float CPhysics_Airboat::ComputeFrontPontoonWaveNoise(int iPoint, float flSpeed)
{
	float flNoise = 1.0f - flSpeed;
	if (flNoise < 0.0f)
		flNoise = 0.0f;
	else if (flNoise > 1.0f)
		flNoise = 1.0f;

	float x = (float)(m_pAirboatBody->get_core()->environment->get_current_time().get_seconds());
	if (flSpeed < 0.3f)
		x += (float)iPoint * 1.5f;

	return (flNoise * 0.02f + 0.01f) * sinf(x * 1.5f);
}

void CPhysics_Airboat::PerformFrictionNotification(float flEnergy, float flDeltaTime, int surfacePropsHit,
                                                   IPhysicsCollisionData *pData)
{
	CPhysicsObject *pPhysAirboat = static_cast<CPhysicsObject *>(m_pAirboatBody->client_data);
	if (!(pPhysAirboat->GetCallbackFlags() & CALLBACK_GLOBAL_FRICTION))
		return;
	IPhysicsCollisionEvent *pEvent = pPhysAirboat->GetVPhysicsEnvironment()->GetCollisionEventHandler();
	if (!pEvent)
		return;
	flEnergy *= flDeltaTime / pPhysAirboat->GetMass();
	if (flEnergy > 0.05f)
		pEvent->Friction(pPhysAirboat, flEnergy, pPhysAirboat->GetMaterialIndex(), surfacePropsHit, pData);
}

CAirboatFrictionData::CAirboatFrictionData()
{
	m_SurfaceNormal.Zero();
	m_ContactPoint.Zero();
	m_ContactSpeed.Zero();
}