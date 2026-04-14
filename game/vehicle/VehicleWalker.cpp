


#include "../Game_local.h"
#include "VehicleAnimated.h"
#include "../Actor.h"

class jkVehicleWalker : public jkVehicleAnimated {
public:

	CLASS_PROTOTYPE( jkVehicleWalker );

	jkVehicleWalker ( void );
	
	void				Think					( void );
	void				Spawn					( void );
	void				Save					( idSaveGame *savefile ) const;
	void				Restore					( idRestoreGame *savefile );

	virtual void		UpdateState				( void );
	virtual void		SetInput				( int position, const usercmd_t& cmd, const idAngles& newAngles );

	const char*			stopAnimName;

	virtual bool		FindClearExitPoint		( int pos, idVec3& origin, idMat3& axis ) const;

private:
	void				HandleStrafing			( void );


	stateResult_t		Frame_ForwardRight		( int );
	stateResult_t		Frame_ForwardLeft		( int );
	stateResult_t		Frame_BackwardRight		( int );
	stateResult_t		Frame_BackwardLeft		( int );

	stateResult_t		State_Wait_OnlineAnim	( const stateParms_t& parms );

	stateResult_t		State_Idle				( const stateParms_t& parms );
	stateResult_t		State_IdleThink			( const stateParms_t& parms );
	stateResult_t		State_IdleOffline		( const stateParms_t& parms );
	stateResult_t		State_Offline			( const stateParms_t& parms );
	stateResult_t		State_Online			( const stateParms_t& parms );

	stateResult_t		State_ForwardStart		( const stateParms_t& parms );
	stateResult_t		State_Forward			( const stateParms_t& parms );
	stateResult_t		State_BackwardStart		( const stateParms_t& parms );
	stateResult_t		State_Backward			( const stateParms_t& parms );
	stateResult_t		State_Stop				( const stateParms_t& parms );
	stateResult_t		State_Turn				( const stateParms_t& parms );
	stateResult_t		State_TurnThink			( const stateParms_t& parms );
	stateResult_t		State_ScriptedAnim		( const stateParms_t& parms );

	void				Event_ScriptedAnim		( const char* animname, int blendFrames, bool loop, bool endWithIdle );
	void 				Event_ScriptedDone		( void );
	void				Event_ScriptedStop		( void );

	CLASS_STATES_PROTOTYPE ( jkVehicleWalker );
};

CLASS_DECLARATION( jkVehicleAnimated, jkVehicleWalker )
	//EVENT( AI_ScriptedAnim,	jkVehicleWalker::Event_ScriptedAnim )
	//EVENT( AI_ScriptedDone,	jkVehicleWalker::Event_ScriptedDone )
	//EVENT( AI_ScriptedStop,	jkVehicleWalker::Event_ScriptedStop )
END_CLASS

/*
================
jkVehicleWalker::jkVehicleWalker
================
*/
jkVehicleWalker::jkVehicleWalker ( void ) {
	stopAnimName = "";
}

/*
================
jkVehicleWalker::Think
================
*/
void jkVehicleWalker::Think ( void ) {
	jkVehicleAnimated::Think();

	if ( !HasDrivers() || IsStalled() ) {
		return;
	}

	idVec3 delta;
	animator.GetDelta( gameLocal.time - gameLocal.GetMSec(), gameLocal.time, delta );

	if ( delta.LengthSqr() > 0.1f ) {
		gameLocal.RadiusDamage( GetOrigin(), this, this, this, this, spawnArgs.GetString( "def_stompDamage", "damage_Smallexplosion" ) );
	}
}

/*
================
jkVehicleWalker::Spawn
================
*/
void jkVehicleWalker::Spawn	( void ) {
    gameLocal.DPrintf("Spawn\n");
	//SetAnimState ( ANIMCHANNEL_LEGS, "State_IdleOffline", 0 );
    SetCAnimState ( ANIMCHANNEL_LEGS, "State_IdleOffline", 0 );
}

/*
================
jkVehicleWalker::Save
================
*/
void jkVehicleWalker::Save ( idSaveGame *savefile ) const {
	savefile->WriteString( stopAnimName );
}

/*
================
jkVehicleWalker::Restore
================
*/
void jkVehicleWalker::Restore ( idRestoreGame *savefile ) {
	//twhitaker: I just happened to see this, while going through this code which I originally wrote (at 3am or so).
	//TODO: fix this.  Make stopAnimName an idStr?
	idStr str;
	savefile->ReadString( str );
	stopAnimName = str;
}

/*
================
jkVehicleWalker::UpdateState
================
*/
void jkVehicleWalker::UpdateState ( void ) {
	jkVehiclePosition& pos = positions[0];
	usercmd_t& cmd	= pos.mInputCmd;

	vfl.driver		= pos.IsOccupied();
  	vfl.forward		= (vfl.driver && cmd.forwardmove > 0);
  	vfl.backward	= (vfl.driver && cmd.forwardmove < 0);
  	vfl.right		= (vfl.driver && cmd.rightmove < 0);
  	vfl.left		= (vfl.driver && cmd.rightmove > 0);	
	//vfl.strafe		= (vfl.driver && cmd.buttons & BUTTON_STRAFE );

	//if ( g_vehicleMode.GetInteger() != 0 ) {
	//	vfl.strafe = !vfl.strafe;
	//}
}

/*
================
jkVehicleWalker::SetInput
================
*/
void jkVehicleWalker::SetInput ( int position, const usercmd_t& cmd, const idAngles& newAngles ) {
	usercmd_t* pcmd = const_cast<usercmd_t*>( &cmd );
	pcmd->rightmove *= -1;
	GetPosition(position)->SetInput ( cmd, newAngles );
}

/*
================
jkVehicleWalker::HandleStrafing
================
*/
void jkVehicleWalker::HandleStrafing ( void ) {
	if ( vfl.right ) {
		additionalDelta -= spawnArgs.GetVector( "strafe_delta", "0 0 1.2" );
	}
	if ( vfl.left ) {
		additionalDelta += spawnArgs.GetVector( "strafe_delta", "0 0 1.2" );
	}
}

// mekberg: overloaded this because physics bounds code is significantly different
/*
=====================
jkVehicleWalker::FindClearExitPoint
=====================
*/
// FIXME: this whole function could be cleaned up
bool jkVehicleWalker::FindClearExitPoint( int pos, idVec3& origin, idMat3& axis ) const {
	trace_t		trace;
	const jkVehiclePosition*	position = GetPosition( pos );
	idActor*	driver = position->GetDriver();
	idVec3		end;
	idVec3		traceOffsetPoints[4];
	const float error = 1.1f;

	origin.Zero();
	axis.Identity();

	idMat3 driverAxis = driver->viewAxis;
	idVec3 driverOrigin = driver->GetPhysics()->GetOrigin();

	idMat3 vehicleAxis = position->GetEyeAxis();
	idVec3 vehicleOrigin = GetPhysics()->GetOrigin();

	idBounds driverBounds( driver->GetPhysics()->GetBounds() );
	idBounds vehicleBounds( GetPhysics()->GetBounds() );
	idBounds driverAbsBounds;
	idBounds vehicleAbsBounds;

	vehicleAbsBounds.FromTransformedBounds( vehicleBounds, vehicleOrigin, GetPhysics()->GetAxis() );
	if( position->fl.driverVisible ) {
		// May want to do this even if the driver isn't visible
		if( position->mExitPosOffset.LengthSqr() > VECTOR_EPSILON ) {
			axis = GetPhysics()->GetAxis() * position->mExitAxisOffset;
			origin = vehicleOrigin + position->mExitPosOffset * axis;
		} else {
			origin = driverOrigin;
			//axis = (driver->IsBoundTo(this)) ? vehicleAxis : driverAxis; FIXME1
            axis = vehicleAxis;
		}
		return true;
	}

	// Build list
	// FIXME: try and find a cleaner way to do this
    //FIXME1 implement this here too
	//traceOffsetPoints[ 0 ] = vehicleBounds.FindVectorToEdge( vehicleAxis[ 1 ] ) - driverBounds.FindVectorToEdge( -vehicleAxis[ 1 ] );
	//traceOffsetPoints[ 1 ] = vehicleBounds.FindVectorToEdge( -vehicleAxis[ 1 ] ) - driverBounds.FindVectorToEdge( vehicleAxis[ 1 ] );
	//traceOffsetPoints[ 2 ] = vehicleBounds.FindVectorToEdge( vehicleAxis[ 0 ] ) - driverBounds.FindVectorToEdge( -vehicleAxis[ 0 ] );
	//traceOffsetPoints[ 3 ] = vehicleBounds.FindVectorToEdge( -vehicleAxis[ 0 ] ) - driverBounds.FindVectorToEdge( vehicleAxis[ 0 ] );

	for( int ix = 0; ix < 4; ++ix ) {
		//Try all four sides and on top if need be
		end = vehicleOrigin + traceOffsetPoints[ ix ] * error;
// RAVEN BEGIN
// ddynerman: multiple clip worlds
		//gameLocal.Translation( this, trace, vehicleOrigin, end, driver->GetPhysics()->GetClipModel(), driverAxis, driver->GetPhysics()->GetClipMask(), this, driver );
        //FIXME1
// RAVEN END
		driverAbsBounds.FromTransformedBounds( driverBounds, trace.endpos, driverAxis );
		if( trace.fraction > 0.0f && !driverAbsBounds.IntersectsBounds(vehicleAbsBounds) ) {
			origin = trace.endpos;
			axis = vehicleAxis;
			return true;
		}
	}

	return false;
}

/*
===============================================================================

	States 

===============================================================================
*/

CLASS_STATES_DECLARATION ( jkVehicleWalker )
	STATE ( "ForwardLeft",			jkVehicleWalker::Frame_ForwardLeft )
	STATE ( "ForwardRight",			jkVehicleWalker::Frame_ForwardRight )
	STATE ( "BackwardLeft",			jkVehicleWalker::Frame_BackwardLeft )
	STATE ( "BackwardRight",		jkVehicleWalker::Frame_BackwardRight )

	STATE ( "Wait_OnlineAnim",		jkVehicleWalker::State_Wait_OnlineAnim )
	
	STATE ( "State_Idle",			jkVehicleWalker::State_Idle )
	STATE ( "State_IdleThink",		jkVehicleWalker::State_IdleThink )
	STATE ( "State_IdleOffline",	jkVehicleWalker::State_IdleOffline )
	STATE ( "State_Offline",		jkVehicleWalker::State_Offline )
	STATE ( "State_Online",			jkVehicleWalker::State_Online )

	STATE ( "State_ForwardStart",	jkVehicleWalker::State_ForwardStart )
	STATE ( "State_Forward",		jkVehicleWalker::State_Forward )
	STATE ( "State_BackwardStart",	jkVehicleWalker::State_BackwardStart )
	STATE ( "State_Backward",		jkVehicleWalker::State_Backward )
	STATE ( "State_Stop",			jkVehicleWalker::State_Stop )
	STATE ( "State_Turn",			jkVehicleWalker::State_Turn )	
	STATE ( "State_TurnThink",		jkVehicleWalker::State_TurnThink )
	STATE ( "State_ScriptedAnim",	jkVehicleWalker::State_ScriptedAnim )
END_CLASS_STATES

/*
================
jkVehicleWalker::State_IdleOffline
================
*/
stateResult_t jkVehicleWalker::State_IdleOffline ( const stateParms_t& parms ) {
	vfl.frozen = true;
    gameLocal.DPrintf("State_IdleOffline\n");
    //FIXME 1 unevent the playcycle ? and add the last parm when you do
	//GetAnimator()->PlayCycle ( ANIMCHANNEL_LEGS, "idle_offline", parms.blendFrames );
    PostEventMS(&AI_PlayCycle, 1, ANIMCHANNEL_LEGS, "idle_offline");
	PostCAnimState ( ANIMCHANNEL_LEGS, "Wait_Driver", 2 );
	PostCAnimState ( ANIMCHANNEL_LEGS, "State_Online", 2 );
	
	return SRESULT_DONE;
}

/*
================
jkVehicleWalker::State_Online
================
*/
stateResult_t jkVehicleWalker::State_Online ( const stateParms_t& parms ) {	
	vfl.frozen = false;
	gameLocal.DPrintf("State_Online\n");
    //FIXME1 Dynamix
	PlayAnim ( ANIMCHANNEL_LEGS, "start", parms.blendFrames );
    //PostEventMS(&AI_PlayAnim, 1, ANIMCHANNEL_LEGS, "start" );
	PostCAnimState ( ANIMCHANNEL_LEGS, "Wait_OnlineAnim", 4 );
	PostCAnimState ( ANIMCHANNEL_LEGS, "State_Idle", 4 );

	return SRESULT_DONE;
}

/*
================
jkVehicleWalker::State_Offline
================
*/
stateResult_t jkVehicleWalker::State_Offline ( const stateParms_t& parms ) {	
    //FIXME1 Dynamix
	//PlayAnim ( ANIMCHANNEL_LEGS, "stop", parms.blendFrames );
    PostEventMS(&AI_PlayAnim, 1, ANIMCHANNEL_LEGS, "stop");
	PostCAnimState ( ANIMCHANNEL_LEGS, "Wait_TorsoAnim", 4 );
	PostCAnimState ( ANIMCHANNEL_LEGS, "State_IdleOffline", 4 );
	return SRESULT_DONE;
}

/*
================
jkVehicleWalker::State_Idle
================
*/
stateResult_t jkVehicleWalker::State_Idle ( const stateParms_t& parms ) {	
	if ( SRESULT_WAIT != State_IdleThink ( parms ) ) {
		return SRESULT_DONE;
	}
	
    //FIXME1 Dynamix
	//PlayCycle( ANIMCHANNEL_LEGS, "idle", parms.blendFrames );
     PostEventMS(&AI_PlayCycle, 1, ANIMCHANNEL_LEGS, "idle");
	PostCAnimState ( ANIMCHANNEL_LEGS, "State_IdleThink", 2 );
	
	return SRESULT_DONE;
}

/*
================
jkVehicleWalker::State_Idle
================
*/
stateResult_t jkVehicleWalker::State_IdleThink ( const stateParms_t& parms ) { 
	if ( !vfl.driver || vfl.stalled ) {
		PostCAnimState ( ANIMCHANNEL_LEGS, "State_Offline", parms.blendFrames );
		return SRESULT_DONE;
	}

	if ( IsMovementEnabled ( ) ) {
		if ( vfl.forward ) {
			PostCAnimState ( ANIMCHANNEL_LEGS, "State_ForwardStart", 2 );
			return SRESULT_DONE;
		}
		
		if ( vfl.backward ) {
			PostCAnimState ( ANIMCHANNEL_LEGS, "State_BackwardStart", 2 );
			return SRESULT_DONE;
		}
		
		if ( vfl.right || vfl.left ) {
			PostCAnimState ( ANIMCHANNEL_LEGS, "State_Turn", 2 );
			return SRESULT_DONE;
		}
	}
	
	return SRESULT_WAIT;
}

/*
================
jkVehicleWalker::State_ForwardStart
================
*/
stateResult_t jkVehicleWalker::State_ForwardStart ( const stateParms_t& parms ) {

    //FIXME1
	//PlayAnim ( ANIMCHANNEL_LEGS, "forward_start", 2 );
    PostEventMS(&AI_PlayAnim, 1, ANIMCHANNEL_LEGS, "forward_start");
	PostCAnimState ( ANIMCHANNEL_LEGS, "Wait_TorsoAnim", 2 );
	PostCAnimState ( ANIMCHANNEL_LEGS, "State_Forward", 2 );
	return SRESULT_DONE;
}

stateResult_t jkVehicleWalker::State_Forward ( const stateParms_t& parms ) {
	// If not moving anymore by the time we get here just play the stop anim
	if ( !vfl.forward ) {
		stopAnimName = "forward_stop_leftmid";
		SetCAnimState ( ANIMCHANNEL_LEGS, "State_Stop", 4 );
		return SRESULT_DONE;
	}

	if ( !parms.stage ) {
		//PlayCycle( ANIMCHANNEL_LEGS, "forward", 2 );
        PostEventMS(&AI_PlayCycle, 1, ANIMCHANNEL_LEGS, "forward" );
		return SRESULT_STAGE(parms.stage + 1);
	}

	if ( AnimDone( ANIMCHANNEL_LEGS, 2 ) ) {
		return SRESULT_DONE;
	}

	HandleStrafing();
	return SRESULT_WAIT;
}

/*
================
jkVehicleWalker::State_BackwardStart
================
*/
stateResult_t jkVehicleWalker::State_BackwardStart ( const stateParms_t& parms ) {

	//PlayAnim ( ANIMCHANNEL_LEGS, "backward_start", parms.blendFrames );
    PostEventMS(&AI_PlayAnim, 1, ANIMCHANNEL_LEGS, "backward_start" );
	PostCAnimState ( ANIMCHANNEL_LEGS, "Wait_TorsoAnim", 2 );
	PostCAnimState ( ANIMCHANNEL_LEGS, "State_Backward", 2 );
	return SRESULT_DONE;
}

stateResult_t jkVehicleWalker::State_Backward ( const stateParms_t& parms ) {
	// If not moving anymore by the time we get here just play the stop anim
	if ( !vfl.backward ) {
		stopAnimName = "backward_stop_leftmid";
		SetCAnimState ( ANIMCHANNEL_LEGS, "State_Stop", 2 );
		return SRESULT_DONE;
	}
		
	if ( !parms.stage ) {
	//PlayCycle( ANIMCHANNEL_LEGS, "backward", 2 );
    PostEventMS(&AI_PlayCycle, 1, ANIMCHANNEL_LEGS, "backward" );
		return SRESULT_STAGE(parms.stage + 1);
	}

	if ( AnimDone( ANIMCHANNEL_LEGS, 2 ) ) {
		return SRESULT_DONE;
	}

	HandleStrafing();
	return SRESULT_WAIT;
}

/*
================
jkVehicleWalker::State_Stop
================
*/
stateResult_t jkVehicleWalker::State_Stop ( const stateParms_t& parms ) {	
	//PlayAnim ( ANIMCHANNEL_LEGS, stopAnimName, parms.blendFrames );
    PostEventMS(&AI_PlayAnim, 1, ANIMCHANNEL_LEGS, stopAnimName );
	PostCAnimState ( ANIMCHANNEL_LEGS, "Wait_TorsoAnim", 2 );
	PostCAnimState ( ANIMCHANNEL_LEGS, "State_Idle", 2 );
	return SRESULT_DONE;
}

/*
================
jkVehicleWalker::State_Turn
================
*/
stateResult_t jkVehicleWalker::State_Turn ( const stateParms_t& parms ) {
	if ( vfl.left ) {
		PlayAnim ( ANIMCHANNEL_LEGS, "turn_left", parms.blendFrames );
       // PostEventMS(&AI_PlayAnim, 1, ANIMCHANNEL_LEGS, "turn_left");
	} else if ( vfl.right ) {
		PlayAnim ( ANIMCHANNEL_LEGS, "turn_right", parms.blendFrames );
       // PostEventMS(&AI_PlayAnim, 1, ANIMCHANNEL_LEGS, "turn_right" );
	} else {
		PostCAnimState ( ANIMCHANNEL_LEGS, "State_Idle", parms.blendFrames );
		return SRESULT_DONE;
	}
	
	PostCAnimState ( ANIMCHANNEL_LEGS, "State_TurnThink", 16 );
	
	return SRESULT_DONE;
}

/*
================
jkVehicleWalker::State_TurnThink
================
*/
stateResult_t jkVehicleWalker::State_TurnThink ( const stateParms_t& parms ) {
	// If moving again bail on the turn, reguardless of whether its in mid animation
	if ( vfl.forward || vfl.backward ) {
		PostCAnimState ( ANIMCHANNEL_LEGS, "State_Idle", parms.blendFrames );		
		return SRESULT_DONE;
	}
	// If the animation is done then repeat
	if ( AnimDone ( ANIMCHANNEL_LEGS, 8 ) ){
		PostCAnimState ( ANIMCHANNEL_LEGS, "State_Turn", 8 );
		return SRESULT_DONE;
	}

	HandleStrafing();

	return SRESULT_WAIT;
}

/*
================
jkVehicleWalker::Frame_ForwardLeft
================
*/
stateResult_t jkVehicleWalker::Frame_ForwardLeft ( int ) {
	if ( !vfl.forward ) {
		stopAnimName = "forward_stop_left";
		SetCAnimState ( ANIMCHANNEL_LEGS, "State_Stop", 2 );
	}
	return SRESULT_OK;
}

/*
================
jkVehicleWalker::Frame_ForwardRight
================
*/
stateResult_t jkVehicleWalker::Frame_ForwardRight ( int ) {
	if ( !vfl.forward ) {
		stopAnimName = "forward_stop_right";
		SetCAnimState ( ANIMCHANNEL_LEGS, "State_Stop", 2 );
	}
	return SRESULT_OK;
}


/*
================
jkVehicleWalker::Frame_BackwardLeft
================
*/
stateResult_t jkVehicleWalker::Frame_BackwardLeft ( int ) {
	if ( !vfl.backward ) {
		stopAnimName = "backward_stop_left";
		SetCAnimState ( ANIMCHANNEL_LEGS, "State_Stop", 2 );
	}
	return SRESULT_OK;
}

/*
================
jkVehicleWalker::Frame_BackwardRight
================
*/
stateResult_t jkVehicleWalker::Frame_BackwardRight ( int ) {
	if ( !vfl.backward ) {
		stopAnimName = "backward_stop_right";
		SetCAnimState ( ANIMCHANNEL_LEGS, "State_Stop", 2 );
	}
	return SRESULT_OK;
}

/*
================
jkVehicleWalker::State_Wait_OnlineAnim
================
*/
stateResult_t jkVehicleWalker::State_Wait_OnlineAnim ( const stateParms_t& parms ) {	
	if ( !AnimDone ( ANIMCHANNEL_LEGS, parms.blendFrames ) && vfl.driver ) {
		return SRESULT_WAIT;
	}
	return SRESULT_DONE;
}	

/*
================
jkVehicleWalker::State_ScriptedAnim
================
*/
stateResult_t jkVehicleWalker::State_ScriptedAnim ( const stateParms_t& parms ) {
	if ( !AnimDone ( ANIMCHANNEL_LEGS, parms.blendFrames ) ) {
		return SRESULT_WAIT;
	}
	Event_ScriptedStop();
	return SRESULT_DONE;
}

/*
================
jkVehicleWalker::Event_ScriptedAnim
================
*/
void jkVehicleWalker::Event_ScriptedAnim( const char* animname, int blendFrames, bool loop, bool endWithIdle ) {
	vfl.endWithIdle = endWithIdle;
	if ( loop ) {
		//PlayCycle ( ANIMCHANNEL_LEGS, animname, blendFrames );
        PostEventMS(&AI_PlayCycle, 1, ANIMCHANNEL_LEGS, animname );
	} else {
		//PlayAnim ( ANIMCHANNEL_LEGS, animname, blendFrames );
        PostEventMS(&AI_PlayAnim, 1, ANIMCHANNEL_LEGS, animname );
	}
	SetCAnimState ( ANIMCHANNEL_LEGS, "State_ScriptedAnim", blendFrames );
	vfl.scripted = true;
}

/*
================
jkVehicleWalker::Event_ScriptedDone
================
*/
void jkVehicleWalker::Event_ScriptedDone( void ) {
	idThread::ReturnInt( !vfl.scripted );
}

/*
================
jkVehicleWalker::Event_ScriptedStop
================
*/
void jkVehicleWalker::Event_ScriptedStop( void ) {
	vfl.scripted = false;

	if ( vfl.endWithIdle ) {
		SetCAnimState ( ANIMCHANNEL_LEGS, "State_Idle", 2 );
	}
}
