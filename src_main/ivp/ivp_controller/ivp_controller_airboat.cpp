//===== Copyright � 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications � 2013, SiPlus, MIT licensed. =============//
//
// Purpose: 4-Wheel Vehicle attempt at a airboat!
//
//===========================================================================//

// Some code.
// Copyright (C) Ipion Software GmbH 1999-2000. All rights reserved.
#ifndef WIN32
#pragma implementation "ivp_controller_airboat.h"
#endif
#include <ivp_physics.hxx>
#include "ivp_controller_airboat.h"

// includes for API
#include <ivp_material.hxx>
#include <ivp_cache_object.hxx>
#include <ivp_actuator.hxx>
#include <ivp_constraint_car.hxx>
#include <ivp_car_system.hxx>
#include <ivp_ray_solver.hxx>
#include <ivp_solver_core_reaction.hxx>

//-----------------------------------------------------------------------------
// Purpose: Main raycast airboat simulation.
//-----------------------------------------------------------------------------
void IVP_Controller_Raycast_Airboat::do_simulation_controller( IVP_Event_Sim *pEventSim, IVP_U_Vector<IVP_Core> * )
{
    IVP_Raycast_Airboat_Pontoon_Temp	pontoonsTemp[IVP_RAYCAST_AIRBOAT_MAX_WHEELS];
    IVP_Ray_Solver_Template				raySolverTemplates[IVP_RAYCAST_AIRBOAT_MAX_WHEELS];
	IVP_Raycast_Airboat_Impact			impacts[IVP_RAYCAST_AIRBOAT_MAX_WHEELS];

	IVP_Core *pAirboatCore = m_pAirboatBody->get_core();
    const IVP_U_Matrix *matWorldFromCore = pAirboatCore->get_m_world_f_core_PSI();
	matWorldFromCore->vimult3(&(pAirboatCore->speed), &m_LocalVelocity);
    
	// Raycasts.
	PreRaycasts( raySolverTemplates, matWorldFromCore, pontoonsTemp );
	do_raycasts_gameside( n_wheels, raySolverTemplates, impacts );
	if ( !PostRaycasts( raySolverTemplates, matWorldFromCore, pontoonsTemp, impacts ) )
		return;

	// Airborne state;
	UpdateAirborneState(impacts, pEventSim);

    // Pontoons.
	DoSimulationPontoons(pontoonsTemp, impacts, pEventSim);

	// Drag (Water)
	DoSimulationDrag(pontoonsTemp, impacts, pEventSim);

	// Turbine (fan).
	DoSimulationTurbine(pEventSim);

	// Steering.
	DoSimulationSteering(pEventSim);

	// Keep upright.
	DoSimulationKeepUprightPitch(impacts, pEventSim);
	DoSimulationKeepUprightRoll(impacts, pEventSim);
}

//-----------------------------------------------------------------------------
// Purpose: Initialize the rays to be cast from the vehicle wheel positions to
//          the "ground."
//-----------------------------------------------------------------------------
void IVP_Controller_Raycast_Airboat::PreRaycasts( IVP_Ray_Solver_Template *pRaySolverTemplates, 
												  const IVP_U_Matrix *matWorldFromCore, 
												  IVP_Raycast_Airboat_Pontoon_Temp *pTempPontoons )
{
	int nPontoonPoints = n_wheels;
	for ( int iPoint = 0; iPoint < nPontoonPoints; ++iPoint )
	{
		IVP_Raycast_Airboat_Wheel *pPontoonPoint = get_wheel( IVP_POS_WHEEL( iPoint ) );
		if ( pPontoonPoint )
		{
			// Fill the in the ray solver template for the current wheel.
			IVP_Ray_Solver_Template &raySolverTemplate = pRaySolverTemplates[iPoint];

			// Transform the wheel "start" position from vehicle core-space to world-space.  This is
			// the raycast starting position.
			matWorldFromCore->vmult4( &pPontoonPoint->raycast_start_cs, &raySolverTemplate.ray_start_point );

			// Transform the shock (spring) direction from vehicle core-space to world-space.  This is
			// the raycast direction.
			matWorldFromCore->vmult3( &pPontoonPoint->raycast_dir_cs, &pTempPontoons[iPoint].raycast_dir_ws );
			raySolverTemplate.ray_normized_direction.set( &pTempPontoons[iPoint].raycast_dir_ws );

			// Set the length of the ray cast.
			raySolverTemplate.ray_length = AIRBOAT_RAYCAST_DIST;	

			// Set the ray solver template flags.  This defines wish objects you wish to
			// collide against in the physics environment.
			raySolverTemplate.ray_flags = IVP_RAY_SOLVER_ALL;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool IVP_Controller_Raycast_Airboat::PostRaycasts( IVP_Ray_Solver_Template *pRaySolverTemplates, const IVP_U_Matrix *matWorldFromCore, IVP_Raycast_Airboat_Pontoon_Temp *pTempPontoons, 
												   IVP_Raycast_Airboat_Impact *pImpacts )
{
	int nPontoonPoints = n_wheels;
	for( int iPoint = 0; iPoint < nPontoonPoints; ++iPoint )
	{
		// Get data at raycast position.
		IVP_Raycast_Airboat_Wheel *pPontoonPoint = get_wheel( IVP_POS_WHEEL( iPoint ) );
		IVP_Raycast_Airboat_Pontoon_Temp *pTempPontoonPoint = &pTempPontoons[iPoint];
		IVP_Raycast_Airboat_Impact *pImpact = &pImpacts[iPoint];
		IVP_Ray_Solver_Template *pRaySolver = &pRaySolverTemplates[iPoint];
		if ( !pPontoonPoint || !pTempPontoonPoint || !pImpact || !pRaySolver )
			continue;

		// Copy the ray length back, it may have changed.
		pPontoonPoint->raycast_length = pRaySolver->ray_length;

		// Test for inverted raycast direction.
		if ( pImpact->bInWater )
		{
			pTempPontoonPoint->raycast_dir_ws.set_multiple( &pTempPontoonPoint->raycast_dir_ws, -1 );
		}

		// Impact.
		if ( pImpact->bImpact )
		{
			// Save impact normal.
			pTempPontoonPoint->ground_normal_ws = pImpact->vecImpactNormalWS;
	
			// Save impact distance.
			IVP_U_Point vecDelta;
			vecDelta.subtract( &pImpact->vecImpactPointWS, &pRaySolver->ray_start_point );
			pPontoonPoint->raycast_dist = vecDelta.real_length();

			// Get the inverse portion of the surface normal in the direction of the ray cast (shock - used in the shock simulation code for the sign
			// and percentage of force applied to the shock).
			pTempPontoonPoint->inv_normal_dot_dir = 1.1f / ( IVP_Inline_Math::fabsd( pTempPontoonPoint->raycast_dir_ws.dot_product( &pTempPontoonPoint->ground_normal_ws ) ) + 0.1f );

			// Set the wheel friciton - ground friction (if any) + wheel friction.
			pTempPontoonPoint->friction_value = pImpact->flFriction * pPontoonPoint->friction_of_wheel;
		}
		// No impact.
		else
		{
			pPontoonPoint->raycast_dist = pPontoonPoint->raycast_length;

			pTempPontoonPoint->inv_normal_dot_dir = 1.0f;	    
			pTempPontoonPoint->moveable_object_hit_by_ray = NULL;
			pTempPontoonPoint->ground_normal_ws.set_multiple( &pTempPontoonPoint->raycast_dir_ws, -1 );
			pTempPontoonPoint->friction_value = 1.0f;
		}

		// Set the new wheel position (the impact point or the full ray distance).  Make this from the wheel not the ray trace position.
		pTempPontoonPoint->ground_hit_ws.add_multiple( &pRaySolver->ray_start_point, &pTempPontoonPoint->raycast_dir_ws, pPontoonPoint->raycast_dist );

		// Get the speed (velocity) at the impact point.
		m_pAirboatBody->get_core()->get_surface_speed_ws( &pTempPontoonPoint->ground_hit_ws, &pTempPontoonPoint->surface_speed_wheel_ws );
		pTempPontoonPoint->projected_surface_speed_wheel_ws.set_orthogonal_part( &pTempPontoonPoint->surface_speed_wheel_ws, &pTempPontoonPoint->ground_normal_ws );

		matWorldFromCore->vmult3( &pPontoonPoint->axis_direction_cs, &pTempPontoonPoint->axis_direction_ws );
		pTempPontoonPoint->projected_axis_direction_ws.set_orthogonal_part( &pTempPontoonPoint->axis_direction_ws, &pTempPontoonPoint->ground_normal_ws );
		if ( pTempPontoonPoint->projected_axis_direction_ws.normize() == IVP_FAULT )
		{
			printf( "IVP_Controller_Raycast_Airboat::do_simulation_controller projected_axis_direction_ws.normize failed\n" );
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void IVP_Controller_Raycast_Airboat::DoSimulationPontoons( IVP_Raycast_Airboat_Pontoon_Temp *pTempPontoons,
													       IVP_Raycast_Airboat_Impact *pImpacts, IVP_Event_Sim *pEventSim )
{
	for ( int iPoint = 0; iPoint < n_wheels; ++iPoint )
	{
		IVP_Raycast_Airboat_Wheel *pPontoonPoint = get_wheel( IVP_POS_WHEEL( iPoint ) );
		if ( !pPontoonPoint )
			continue;

		if ( pImpacts[iPoint].bImpact )
			DoSimulationPontoonsGround( pPontoonPoint, &pTempPontoons[iPoint], &pImpacts[iPoint], pEventSim );
		else if ( pImpacts[iPoint].bInWater )
			DoSimulationPontoonsWater( &pTempPontoons[iPoint], &pImpacts[iPoint], pEventSim );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handle pontoons on ground.
//-----------------------------------------------------------------------------
void IVP_Controller_Raycast_Airboat::DoSimulationPontoonsGround( IVP_Raycast_Airboat_Wheel *pPontoonPoint, 
																 IVP_Raycast_Airboat_Pontoon_Temp *pTempPontoon,
													             IVP_Raycast_Airboat_Impact *pImpact, IVP_Event_Sim *pEventSim )
{
	// Check to see if we hit anything, otherwise the no force on this point.
	IVP_DOUBLE flDiff = pPontoonPoint->raycast_dist - pPontoonPoint->raycast_length;
	if ( flDiff >= 0 ) 
		return;
	
	IVP_FLOAT flSpringConstant, flSpringRelax, flSpringCompress;
	flSpringConstant = pPontoonPoint->spring_constant;
	flSpringRelax = pPontoonPoint->spring_damp_relax;
	flSpringCompress = pPontoonPoint->spring_damp_compress;
	
	IVP_DOUBLE flForce = -flDiff * flSpringConstant;
	IVP_FLOAT flInvNormalDotDir = pTempPontoon->inv_normal_dot_dir;
	if ( flInvNormalDotDir < 0.0f ) { flInvNormalDotDir = 0.0f; }
	if ( flInvNormalDotDir > 3.0f ) { flInvNormalDotDir = 3.0f; }
	flForce *= flInvNormalDotDir;
	
	IVP_U_Float_Point vecSpeedDelta; 
	vecSpeedDelta.subtract( &pTempPontoon->projected_surface_speed_wheel_ws, &pTempPontoon->surface_speed_wheel_ws );
	
	IVP_DOUBLE flSpeed = vecSpeedDelta.dot_product( &pTempPontoon->raycast_dir_ws );
	if ( flSpeed > 0 )
	{
		flForce -= flSpringRelax * flSpeed;
	}
	else
	{
		flForce -= flSpringCompress * flSpeed;
	}
	
	if ( flForce < 0 )
	{
		flForce = 0.0f;
	}
	
	IVP_DOUBLE flImpulse = flForce * pEventSim->delta_time;		
	
	IVP_U_Float_Point vecImpulseWS; 
	vecImpulseWS.set_multiple( &pTempPontoon->ground_normal_ws, flImpulse );
	m_pAirboatBody->get_core()->push_core_ws( &pTempPontoon->ground_hit_ws, &vecImpulseWS );
}

//-----------------------------------------------------------------------------
// Purpose: Handle pontoons on water.
//-----------------------------------------------------------------------------
void IVP_Controller_Raycast_Airboat::DoSimulationPontoonsWater( IVP_Raycast_Airboat_Pontoon_Temp *pTempPontoon,
													            IVP_Raycast_Airboat_Impact *pImpact,
																IVP_Event_Sim *pEventSim )
{
	IVP_FLOAT flForce = pImpact->flWaterDepth;
	if (flForce > 0.41f)
	{
		flForce = 2.8f * 0.41f * 0.0254f;
	}
	else
	{
		if (flForce < 0.0f)
			flForce = 0.0f;
		else
			flForce *= 2.8f * 0.0254f;
	}
	IVP_Core *pAirboatCore = m_pAirboatBody->get_core();
	IVP_U_Float_Point vecImpulseWS(0.0f,
		flForce * (-0.4f * AIRBOAT_WATER_DENSITY) * pAirboatCore->get_mass() * pEventSim->delta_time, 0.0f);
	pAirboatCore->push_core_ws(&(pTempPontoon->ground_hit_ws), &vecImpulseWS);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void IVP_Controller_Raycast_Airboat::DoSimulationTurbine(IVP_Event_Sim *pEventSim)
{
	IVP_Core *pAirboatCore = m_pAirboatBody->get_core();

	IVP_FLOAT thrust = m_flThrust;
	if (m_bAirborneIdle || (m_bAirborne && (thrust < 0.0f)))
		thrust *= 0.5f;

	IVP_U_Float_Point vecImpulse;
	pAirboatCore->get_m_world_f_core_PSI()->get_col(IVP_COORDINATE_INDEX(index_z), &vecImpulse);
	IVP_FLOAT flForwardY = vecImpulse.k[1];

	if ((flForwardY < -0.5f) && (thrust > 0.0f))
		thrust *= 1.0f + flForwardY;
	else if ((flForwardY > 0.5f) && (thrust < 0.0f))
		thrust *= 1.0f - flForwardY;

	
	vecImpulse.mult(pAirboatCore->get_mass() * thrust * pEventSim->delta_time);
	pAirboatCore->center_push_core_multiple_ws(&vecImpulse);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void IVP_Controller_Raycast_Airboat::DoSimulationSteering(IVP_Event_Sim *pEventSim)
{
	IVP_Core *pAirboatCore = m_pAirboatBody->get_core();

	if ((m_SteeringAngle == 0.0f) || (m_flThrust != 0.0f))
	{
		if (m_bAnalogSteering)
		{
			if (m_flThrust < -2.0f)
				m_bReverseSteering = IVP_TRUE;
			else if ((m_flThrust > 2.0f) || (m_LocalVelocity.k[2] > 0.0f))
				m_bReverseSteering = IVP_FALSE;
		}
		else
		{
			if (m_flThrust < 0.0f)
				m_bReverseSteering = IVP_TRUE;
			else if ((m_flThrust > 0.0f) || (m_LocalVelocity.k[2] > 0.0f))
				m_bReverseSteering = IVP_FALSE;
		}
	}

	IVP_FLOAT flForceRotational;
	IVP_FLOAT flSteeringSign;
	IVP_FLOAT flMass = pAirboatCore->get_mass();

	if (fabsf(m_SteeringAngle) > 0.01)
	{
		flSteeringSign = (m_SteeringAngle < 0.0f ? -1.0f : 1.0f);
		if (m_bReverseSteering)
			flSteeringSign = -flSteeringSign;
		IVP_FLOAT flLastSign = (m_LastForwardSteeringAngle < 0.0f ? -1.0f : 1.0f);
		if ((fabsf(m_LastForwardSteeringAngle) < 0.01) || (flSteeringSign != flLastSign))
			m_SteeringSpeed = 0.0f;

		flForceRotational = (m_bAnalogSteering ? fabsf(m_SteeringAngle) : m_SteeringSpeed) * 2.0f;
		if (flForceRotational < 0.0f)
			flForceRotational = 0.0f;
		else if (flForceRotational > 1.0f)
			flForceRotational = 1.0f;
		flForceRotational = flForceRotational * IVP_RAYCAST_AIRBOAT_STEERING_RATE + (IVP_RAYCAST_AIRBOAT_STEERING_RATE * 0.25f);
		flForceRotational *= -(flMass * pEventSim->i_delta_time * flSteeringSign);
		m_SteeringSpeed += pEventSim->delta_time;
	}
	else
	{
		flForceRotational = 0.0f;
	}

	m_LastForwardSteeringAngle = m_SteeringAngle;
	if (m_bReverseSteering)
		m_LastForwardSteeringAngle = -m_LastForwardSteeringAngle;

	IVP_FLOAT flRotSpeed = pAirboatCore->rot_speed.k[1];
	flSteeringSign = (flRotSpeed < 0.0f ? -1.0f : 1.0f);
	flForceRotational += 0.0004 * flMass * flRotSpeed * flRotSpeed * pEventSim->i_delta_time * flSteeringSign;
	flForceRotational += 0.001 * flMass * fabsf(flRotSpeed) * pEventSim->i_delta_time * flSteeringSign;

	IVP_U_Float_Point vecAngularImpulse;
	vecAngularImpulse.set(0.0f, -flForceRotational, 0.0f);
	pAirboatCore->rot_push_core_cs(&vecAngularImpulse);
}

void IVP_Controller_Raycast_Airboat::DoSimulationKeepUprightPitch(IVP_Raycast_Airboat_Impact *pImpacts,
                                                                  IVP_Event_Sim *pEventSim)
{
	if (m_bAirborneIdle)
		return;

	IVP_Core *pAirboatCore = m_pAirboatBody->get_core();

	IVP_U_Float_Point dir, temp, cross;
	temp.set(0.0f, -1.0f, 0.0f);
	pAirboatCore->get_m_world_f_core_PSI()->vimult3(&temp, &dir);
	dir.k[0] = 0.0f;
	dir.normize();
	temp.set(0.0f, -0.9848077f, 0.17364818f); // 0.0, -cos(10deg), sin(10deg)
	cross.calc_cross_product(&temp, &dir);
	IVP_DOUBLE pitch = IVP_Inline_Math::atan2d(cross.real_length_plus_normize(), temp.dot_product(&dir));

	int iPoint;
	for (iPoint = 0; iPoint < n_wheels; ++iPoint, ++pImpacts)
	{
		if (pImpacts->bImpact)
		{
			m_flKeepUprightPitch = pitch;
			return;
		}
	}

	IVP_FLOAT mass = pAirboatCore->get_mass();
	temp.set_multiple(&cross, (pEventSim->i_delta_time * 0.04 * (pitch - m_flKeepUprightPitch) + pitch * 0.1) * mass);
	m_flKeepUprightPitch = pitch;
	mass *= (1.5 * IVP_PI / 180.0);
	IVP_FLOAT length = temp.real_length_plus_normize();
	if (length > mass)
		length = mass;
	temp.mult(length);
	pAirboatCore->rot_push_core_cs(&temp);
}

void IVP_Controller_Raycast_Airboat::DoSimulationKeepUprightRoll(IVP_Raycast_Airboat_Impact *pImpacts,
                                                                 IVP_Event_Sim *pEventSim)
{
	IVP_Core *pAirboatCore = m_pAirboatBody->get_core();

	IVP_U_Float_Point dir, temp, cross;
	temp.set(0.0f, -1.0f, 0.0f);
	pAirboatCore->get_m_world_f_core_PSI()->vimult3(&temp, &dir);
	dir.k[1] = -0.9848077f; // -cos(10deg)
	dir.normize();
	temp.set(0.0f, -0.9848077f, 0.17364818f); // 0.0, -cos(10deg), sin(10deg)
	cross.calc_cross_product(&temp, &dir);
	IVP_DOUBLE roll = IVP_Inline_Math::atan2d(cross.real_length_plus_normize(), temp.dot_product(&dir));

	IVP_BOOL bImpact = IVP_FALSE;
	int iPoint;
	for (iPoint = 0; iPoint < n_wheels; ++iPoint, ++pImpacts)
	{
		if (pImpacts->bImpact)
		{
			bImpact = IVP_TRUE;
			break;
		}
	}
	if (bImpact || (fabs(roll) < (10.0 * IVP_PI / 180.0)))
	{
		m_flKeepUprightRoll = roll;
		return;
	}

	IVP_FLOAT mass = pAirboatCore->get_mass();
	temp.set_multiple(&cross, (pEventSim->i_delta_time * 0.3 * (roll - m_flKeepUprightRoll) + roll * 0.2) * mass);
	m_flKeepUprightRoll = roll;
	mass *= (2.0 * IVP_PI / 180.0);
	IVP_FLOAT length = temp.real_length_plus_normize();
	if (length > mass)
		length = mass;
	temp.mult(length);
	pAirboatCore->rot_push_core_cs(&temp);
}


void IVP_Controller_Raycast_Airboat::UpdateAirborneState(IVP_Raycast_Airboat_Impact *pImpacts,
                                                         IVP_Event_Sim *pEventSim)
{
	int iPoint;
	for (iPoint = 0; iPoint < n_wheels; ++iPoint, ++pImpacts)
	{
		if (!(pImpacts->bImpact))
			continue;
		if (m_bAirborne)
			m_flAirborneTime += pEventSim->delta_time;
		else
		{
			m_bAirborne = IVP_TRUE;
			m_flAirborneTime = 0.0f;
			if (m_pAirboatBody->get_core()->speed.real_length() < IVP_RAYCAST_AIRBOAT_THRUST_MAX)
				m_bAirborneIdle = IVP_TRUE;
		}
		return;
	}
	m_bAirborne = IVP_FALSE;
	m_bAirborneIdle = IVP_FALSE;
}

void IVP_Controller_Raycast_Airboat::do_steering_wheel(IVP_POS_WHEEL wheel_nr, IVP_FLOAT s_angle)
{
    IVP_Raycast_Airboat_Wheel *wheel = get_wheel(wheel_nr);
	
    wheel->axis_direction_cs.set_to_zero();
    wheel->axis_direction_cs.k[ index_x ] = 1.0f;
    wheel->axis_direction_cs.rotate( IVP_COORDINATE_INDEX(index_y), s_angle);
}

void IVP_Controller_Raycast_Airboat::change_spring_constant(IVP_POS_WHEEL pos, IVP_FLOAT spring_constant)
{
    IVP_Raycast_Airboat_Wheel *wheel = get_wheel(pos);
    wheel->spring_constant = spring_constant;
}

void IVP_Controller_Raycast_Airboat::change_spring_dampening(IVP_POS_WHEEL pos, IVP_FLOAT spring_dampening)
{
    IVP_Raycast_Airboat_Wheel *wheel = get_wheel(pos);
    wheel->spring_damp_relax = spring_dampening;
}

void IVP_Controller_Raycast_Airboat::change_spring_dampening_compression(IVP_POS_WHEEL pos, IVP_FLOAT spring_dampening)
{
    IVP_Raycast_Airboat_Wheel *wheel = get_wheel(pos);
    wheel->spring_damp_compress = spring_dampening;
}

void IVP_Controller_Raycast_Airboat::change_spring_pre_tension(IVP_POS_WHEEL pos, IVP_FLOAT pre_tension_length)
{
    IVP_Raycast_Airboat_Wheel *wheel = get_wheel(pos);
    wheel->spring_len = gravity_y_direction * (wheel->distance_orig_hp_to_hp - pre_tension_length);
}

void IVP_Controller_Raycast_Airboat::change_spring_length(IVP_POS_WHEEL pos, IVP_FLOAT spring_length)
{
    IVP_Raycast_Airboat_Wheel *wheel = get_wheel(pos);
    wheel->spring_len = spring_length;
}

void IVP_Controller_Raycast_Airboat::change_wheel_torque(IVP_POS_WHEEL pos, IVP_FLOAT torque)
{
    IVP_Raycast_Airboat_Wheel *wheel = get_wheel(pos);
    wheel->torque = torque;

	// Wake the physics object if need be!
	m_pAirboatBody->get_environment()->get_controller_manager()->ensure_controller_in_simulation( this );
}

//-----------------------------------------------------------------------------
// Purpose:
// Throttle input is -1 to 1.
//-----------------------------------------------------------------------------
void IVP_Controller_Raycast_Airboat::update_throttle( IVP_FLOAT flThrottle )
{
	// Forward
	if ( fabs( flThrottle ) < 0.01f )
	{
		m_flThrust = 0.0f;
	}
	else if ( flThrottle > 0.0f )
	{
		m_flThrust = IVP_RAYCAST_AIRBOAT_THRUST_MAX * flThrottle;
	}
	else if ( flThrottle < 0.0f )
	{
		m_flThrust = IVP_RAYCAST_AIRBOAT_THRUST_MAX_REVERSE * flThrottle;
	}
}

void IVP_Controller_Raycast_Airboat::fix_wheel(IVP_POS_WHEEL pos, IVP_BOOL stop_wheel)
{
    IVP_Raycast_Airboat_Wheel *wheel = get_wheel(pos);
    wheel->wheel_is_fixed = stop_wheel;
}

void IVP_Controller_Raycast_Airboat::change_friction_of_wheel( IVP_POS_WHEEL pos, IVP_FLOAT friction )
{
	IVP_Raycast_Airboat_Wheel *wheel = get_wheel(pos);
	wheel->friction_of_wheel = friction;
}

void IVP_Controller_Raycast_Airboat::change_stabilizer_constant(IVP_POS_AXIS pos, IVP_FLOAT stabi_constant)
{
   IVP_Raycast_Airboat_Axle *pAxle = get_axle( pos );
   pAxle->stabilizer_constant = stabi_constant;
}

void IVP_Controller_Raycast_Airboat::change_fast_turn_factor( IVP_FLOAT fast_turn_factor_ )
{
	//fast_turn_factor = fast_turn_factor_;
}

void IVP_Controller_Raycast_Airboat::change_body_downforce(IVP_FLOAT force)
{
    down_force = force;
}

IVP_CONTROLLER_PRIORITY IVP_Controller_Raycast_Airboat::get_controller_priority()
{
    return IVP_CP_CONSTRAINTS_MAX;
}

void IVP_Controller_Raycast_Airboat::set_booster_acceleration(IVP_FLOAT acceleration)
{
}

void IVP_Controller_Raycast_Airboat::activate_booster(IVP_FLOAT thrust, IVP_FLOAT duration, IVP_FLOAT delay) 
{
}

IVP_FLOAT IVP_Controller_Raycast_Airboat::get_booster_delay() 
{
	return 0.0f; 
}


void IVP_Controller_Raycast_Airboat::do_steering( IVP_FLOAT steering_angle_in, IVP_BOOL analog_steering )
{
	// Check for a change.
    if (m_SteeringAngle == steering_angle_in) 
		return;

	// Set the new steering angle.
    m_SteeringAngle = steering_angle_in;
	m_bAnalogSteering = analog_steering;

	// Make sure the simulation is awake - we just go input.
    m_pAirboatBody->get_environment()->get_controller_manager()->ensure_controller_in_simulation( this );

	// Steer each wheel.
    for ( int iWheel = 0; iWheel < wheels_per_axis; ++iWheel )
	{
		do_steering_wheel( IVP_POS_WHEEL( iWheel ), m_SteeringAngle );
    }
}

IVP_Controller_Raycast_Airboat::~IVP_Controller_Raycast_Airboat()
{
    m_pAirboatBody->get_environment()->get_controller_manager()->remove_controller_from_environment( this, IVP_TRUE );
}


IVP_DOUBLE IVP_Controller_Raycast_Airboat::get_wheel_angular_velocity(IVP_POS_WHEEL pos)
{
    IVP_Raycast_Airboat_Wheel *wheel = get_wheel(pos);
    return wheel->wheel_angular_velocity;
}

IVP_DOUBLE IVP_Controller_Raycast_Airboat::get_body_speed(IVP_COORDINATE_INDEX index)
{
    // return (IVP_FLOAT)car_body->get_geom_center_speed();
    IVP_U_Float_Point *vec_ws = &m_pAirboatBody->get_core()->speed;
    // works well as we do not use merged cores
    const IVP_U_Matrix *mat_ws = m_pAirboatBody->get_core()->get_m_world_f_core_PSI();
    IVP_U_Point orientation;
    mat_ws->get_col(index, &orientation);

    return orientation.dot_product(vec_ws);
};

IVP_DOUBLE IVP_Controller_Raycast_Airboat::get_orig_front_wheel_distance()
{
    IVP_U_Float_Point *left_wheel_cs = &this->get_wheel(IVP_FRONT_LEFT)->hp_cs;
    IVP_U_Float_Point *right_wheel_cs = &this->get_wheel(IVP_FRONT_RIGHT)->hp_cs;

    IVP_DOUBLE dist = left_wheel_cs->k[this->index_x] - right_wheel_cs->k[this->index_x];

    return IVP_Inline_Math::fabsd(dist); // was fabs, which was a sml call
}

IVP_DOUBLE IVP_Controller_Raycast_Airboat::get_orig_axles_distance()
{
    IVP_U_Float_Point *front_wheel_cs = &this->get_wheel(IVP_FRONT_LEFT)->hp_cs;
    IVP_U_Float_Point *rear_wheel_cs = &this->get_wheel(IVP_REAR_LEFT)->hp_cs;

    IVP_DOUBLE dist = front_wheel_cs->k[this->index_z] - rear_wheel_cs->k[this->index_z];

    return IVP_Inline_Math::fabsd(dist); // was fabs, which was a sml call
}

void IVP_Controller_Raycast_Airboat::get_skid_info( IVP_Wheel_Skid_Info *array_of_skid_info_out)
{
	for ( int w = 0; w < n_wheels; w++)
	{
		IVP_Wheel_Skid_Info &info = array_of_skid_info_out[w];
		//IVP_Constraint_Car_Object *wheel = car_constraint_solver->wheel_objects.element_at(w);
		info.last_contact_position_ws.set_to_zero(); // = wheel->last_contact_position_ws;
		info.last_skid_value = 0.0f; // wheel->last_skid_value;
		info.last_skid_time = 0.0f; //wheel->last_skid_time;
	}
}

//-----------------------------------------------------------------------------
// Purpose: (Ipion) Create a raycast car controller.
//   Input: pEnvironment - the physics environment the car is to reside in
//          pCarSystemTemplate - ipion's car system template (filled out with car parameters)
//-----------------------------------------------------------------------------
IVP_Controller_Raycast_Airboat::IVP_Controller_Raycast_Airboat( IVP_Environment *pEnvironment/*environment*/, 
													    const IVP_Template_Car_System *pCarSystemTemplate/*tcs*/ )
{
	InitRaycastCarBody( pCarSystemTemplate );
	InitRaycastCarEnvironment( pEnvironment, pCarSystemTemplate );
	InitRaycastCarWheels( pCarSystemTemplate );
	InitRaycastCarAxes( pCarSystemTemplate );

	m_bAirborne = IVP_FALSE;
	m_flAirborneTime = 0.0f;
	m_bAirborneIdle = IVP_FALSE;

	m_flKeepUprightPitch = 0.0f;
	m_flKeepUprightRoll = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void IVP_Controller_Raycast_Airboat::InitRaycastCarEnvironment( IVP_Environment *pEnvironment, 
													        const IVP_Template_Car_System *pCarSystemTemplate )
{
	// Copies of the car system template component indices and handedness.
    index_x = pCarSystemTemplate->index_x;
    index_y = pCarSystemTemplate->index_y;
    index_z = pCarSystemTemplate->index_z;
    is_left_handed = pCarSystemTemplate->is_left_handed;

	// Remove the old gravity controller and add a different one.
	IVP_Controller *pGravity = pEnvironment->get_gravity_controller();
	if ( pGravity )
	{
		m_pAirboatBody->get_core()->rem_core_controller( pGravity );
	}

	IVP_Standard_Gravity_Controller *pGravityController = new IVP_Standard_Gravity_Controller();
	IVP_U_Point vecGravity( 0.0f, AIRBOAT_GRAVITY, 0.0f );
	pGravityController->grav_vec.set( &vecGravity );

	m_pAirboatBody->get_core()->add_core_controller( pGravityController );
	
	// Add this controller to the physics environment and setup the objects gravity.
    pEnvironment->get_controller_manager()->announce_controller_to_environment( this );
    extra_gravity = pCarSystemTemplate->extra_gravity_force_value;    
//    extra_gravity = -10.0f * m_pAirboatBody->get_core()->get_mass();//pCarSystemTemplate->extra_gravity_force_value;    

	// This works because gravity is still int the same direction, just smaller.
    if ( pEnvironment->get_gravity()->k[index_y] > 0 )
	{
		gravity_y_direction = 1.0f;
    }
	else
	{
		gravity_y_direction = -1.0f;
    }
    normized_gravity_ws.set( pEnvironment->get_gravity() );
    normized_gravity_ws.normize();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void IVP_Controller_Raycast_Airboat::InitRaycastCarBody( const IVP_Template_Car_System *pCarSystemTemplate )
{
	// Car body attributes.
    n_wheels = pCarSystemTemplate->n_wheels;
    n_axis = pCarSystemTemplate->n_axis;
    wheels_per_axis = n_wheels / n_axis;

	// Add the car body "core" to the list of raycast car controller "cores."
    m_pAirboatBody = pCarSystemTemplate->car_body;
    this->vector_of_cores.add( m_pAirboatBody->get_core() );
    
	// Init extra downward force applied to car.    
    down_force_vertical_offset = pCarSystemTemplate->body_down_force_vertical_offset;
    down_force = 0.0f;

	// Initialize.
	for ( int iAxis = 0; iAxis < 3; ++iAxis )
	{
		m_pAirboatBody->get_core()->rot_speed.k[iAxis] = 0.0f;
		m_pAirboatBody->get_core()->speed.k[iAxis] = 0.0f;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void IVP_Controller_Raycast_Airboat::InitRaycastCarWheels( const IVP_Template_Car_System *pCarSystemTemplate )
{
    IVP_U_Matrix m_core_f_object;
    m_pAirboatBody->calc_m_core_f_object( &m_core_f_object );

	// Initialize the car wheel system.
    for ( int iWheel = 0; iWheel < n_wheels; iWheel++ )
	{
		// Get and clear out memory for the current raycast wheel.
		IVP_Raycast_Airboat_Wheel *pRaycastWheel = get_wheel( IVP_POS_WHEEL( iWheel ) );
		P_MEM_CLEAR( pRaycastWheel );

		// Put the wheel in car space.
		m_core_f_object.vmult4( &pCarSystemTemplate->wheel_pos_Bos[iWheel], &pRaycastWheel->hp_cs );
		m_core_f_object.vmult4( &pCarSystemTemplate->trace_pos_Bos[iWheel], &pRaycastWheel->raycast_start_cs );
		
		// Add in the raycast start offset.
		pRaycastWheel->raycast_length = AIRBOAT_RAYCAST_DIST;
		pRaycastWheel->raycast_dir_cs.set_to_zero();
		pRaycastWheel->raycast_dir_cs.k[index_y] = gravity_y_direction;

		// Spring (Shocks) data.
		pRaycastWheel->spring_len = -pCarSystemTemplate->spring_pre_tension[iWheel];

		pRaycastWheel->spring_direction_cs.set_to_zero();
		pRaycastWheel->spring_direction_cs.k[index_y] = gravity_y_direction;
		
		pRaycastWheel->spring_constant = pCarSystemTemplate->spring_constant[iWheel];
		pRaycastWheel->spring_damp_relax = pCarSystemTemplate->spring_dampening[iWheel];
		pRaycastWheel->spring_damp_compress = pCarSystemTemplate->spring_dampening_compression[iWheel];

		// Wheel data.
		pRaycastWheel->friction_of_wheel = 1.0f;//pCarSystemTemplate->friction_of_wheel[iWheel];
		pRaycastWheel->wheel_radius = pCarSystemTemplate->wheel_radius[iWheel];
		pRaycastWheel->inv_wheel_radius = 1.0f / pCarSystemTemplate->wheel_radius[iWheel];

		do_steering_wheel( IVP_POS_WHEEL( iWheel ), 0.0f );
		
		pRaycastWheel->wheel_is_fixed = IVP_FALSE;	
		pRaycastWheel->max_rotation_speed = pCarSystemTemplate->wheel_max_rotation_speed[iWheel>>1];

		pRaycastWheel->wheel_is_fixed = IVP_TRUE;
    }
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void IVP_Controller_Raycast_Airboat::InitRaycastCarAxes( const IVP_Template_Car_System *pCarSystemTemplate )
{
    m_SteeringAngle = -1.0f;		// make sure next call is not optimized
    this->do_steering( 0.0f, IVP_FALSE );			// make sure next call gets through

    for ( int iAxis = 0; iAxis < n_axis; iAxis++ )
	{
		IVP_Raycast_Airboat_Axle *pAxle = get_axle( IVP_POS_AXIS( iAxis ) );
		pAxle->stabilizer_constant = pCarSystemTemplate->stabilizer_constant[iAxis];
    }    
}

//-----------------------------------------------------------------------------
// Purpose: Debug data for use in vphysics and the engine to visualize car data.
//-----------------------------------------------------------------------------
void IVP_Controller_Raycast_Airboat::SetCarSystemDebugData( const IVP_CarSystemDebugData_t &carSystemDebugData )
{
	// Wheels (raycast data only!)
	for ( int iWheel = 0; iWheel < IVP_RAYCAST_AIRBOAT_MAX_WHEELS; ++iWheel )
	{
		m_CarSystemDebugData.wheelRaycasts[iWheel][0] = carSystemDebugData.wheelRaycasts[iWheel][0];
		m_CarSystemDebugData.wheelRaycasts[iWheel][1] = carSystemDebugData.wheelRaycasts[iWheel][1];
		m_CarSystemDebugData.wheelRaycastImpacts[iWheel] = carSystemDebugData.wheelRaycastImpacts[iWheel];
	}
}

//-----------------------------------------------------------------------------
// Purpose: Debug data for use in vphysics and the engine to visualize car data.
//-----------------------------------------------------------------------------
void IVP_Controller_Raycast_Airboat::GetCarSystemDebugData( IVP_CarSystemDebugData_t &carSystemDebugData )
{
	// Wheels (raycast data only!)
	for ( int iWheel = 0; iWheel < IVP_RAYCAST_AIRBOAT_MAX_WHEELS; ++iWheel )
	{
		carSystemDebugData.wheelRaycasts[iWheel][0] = m_CarSystemDebugData.wheelRaycasts[iWheel][0];
		carSystemDebugData.wheelRaycasts[iWheel][1] = m_CarSystemDebugData.wheelRaycasts[iWheel][1];
		carSystemDebugData.wheelRaycastImpacts[iWheel] = m_CarSystemDebugData.wheelRaycastImpacts[iWheel];
	}
}

void IVP_Controller_Raycast_Airboat::update_booster( IVP_FLOAT booster )
{
}

void IVP_Controller_Raycast_Airboat::change_max_body_force(IVP_POS_WHEEL wheel , IVP_FLOAT mforce)
{
}

void IVP_Controller_Raycast_Airboat::update_body_countertorque()
{
}

IVP_U_Vector<IVP_Core> *IVP_Controller_Raycast_Airboat::get_associated_controlled_cores( void )
{ 
	return &vector_of_cores; 
}

void IVP_Controller_Raycast_Airboat::core_is_going_to_be_deleted_event( IVP_Core *core )
{ 
	P_DELETE_THIS(this); 
}

IVP_Raycast_Airboat_Axle *IVP_Controller_Raycast_Airboat::get_axle( IVP_POS_AXIS i )
{ 
	return &m_aAirboatAxles[i]; 
}

IVP_Raycast_Airboat_Wheel *IVP_Controller_Raycast_Airboat::get_wheel( IVP_POS_WHEEL i )
{ 
	return &m_aAirboatWheels[i]; 
}

IVP_Controller_Raycast_Airboat_Vector_of_Cores_1::IVP_Controller_Raycast_Airboat_Vector_of_Cores_1():
							IVP_U_Vector<IVP_Core>( &elem_buffer[0],1 ) 
{
}
