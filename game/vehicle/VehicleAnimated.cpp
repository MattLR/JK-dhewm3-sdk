
// Vehicle.cpp
//
// Copyright 2002-2004 Raven Software
//----------------------------------------------------------------




#include "../Game_local.h"
#include "VehicleAnimated.h"

CLASS_DECLARATION( jkVehicle, jkVehicleAnimated )
END_CLASS

jkVehicleAnimated::jkVehicleAnimated ( void ) {
}

jkVehicleAnimated::~jkVehicleAnimated ( void ) {
	SetPhysics( NULL );
}

/*
================
jkVehicleAnimated::Spawn
================
*/
void jkVehicleAnimated::Spawn( void ) {

	turnRate   = spawnArgs.GetFloat ( "turnRate", "90" );
	viewAngles = GetPhysics()->GetAxis ( ).ToAngles ( );
	viewAxis   = viewAngles.ToMat3();

	// Initialize the physics object
	physicsObj.SetSelf( this );
	physicsObj.SetClipModel( new idClipModel( GetPhysics()->GetClipModel() ), 1.0f );
	physicsObj.SetContents( CONTENTS_BODY );
	physicsObj.SetClipMask( MASK_PLAYERSOLID );
	physicsObj.SetMaxStepHeight( spawnArgs.GetFloat ( "stepheight", "14" ) );

	// Start just a tad above the floor
	physicsObj.SetOrigin( GetPhysics()->GetOrigin() + idVec3( 0, 0, CM_CLIP_EPSILON ) );
	physicsObj.SetMass( spawnArgs.GetFloat( "mass", "100" ) );
	physicsObj.SetDelta( vec3_origin );

	// Gravity
	idVec3 gravity = spawnArgs.GetVector( "gravityDir", "0 0 -1" );
	gravity *= g_gravity.GetFloat ( );
	physicsObj.SetGravity( gravity );
	SetPhysics( &physicsObj );

	animator.RemoveOriginOffset( true );
		
	additionalDelta.Zero();

	BecomeActive( TH_THINK );		
}

/*
================
jkVehicleAnimated::ClientPredictionThink
================
*/
void jkVehicleAnimated::ClientPredictionThink ( void ) {
	Think();
}

/*
================
jkVehicleAnimated::Think
================
*/
void jkVehicleAnimated::Think ( void ) {

	jkVehicle::Think();	

	float rate = 0.0f;
	usercmd_t& cmd = positions[0].mInputCmd;
	idVec3 delta;

	if( positions[0].IsOccupied() && !IsFrozen() && IsMovementEnabled() )	{
		
        if (1)
		//if (( g_vehicleMode.GetInteger() == 0 && !( cmd.buttons & BUTTON_STRAFE )) || 	// If we're in the old driving mode and we aren't strafing or...
		//	( g_vehicleMode.GetInteger() != 0 && ( cmd.buttons & BUTTON_STRAFE )))		// If we're in the new driving mode and we are strafing
		{
			rate = SignZero(cmd.forwardmove) * turnRate;
			//rate *= idMath::MidPointLerp( 0.0f, 0.9f, 1.0f, idMath::Fabs(cmd.rightmove) / 127.0f );
            rate *= (idMath::Fabs(cmd.rightmove) / 127.0f );
			viewAngles.yaw += Sign(cmd.rightmove) * rate * MS2SEC(gameLocal.GetMSec());	
		}
	}

	viewAngles.Normalize360();
	animator.GetDelta( gameLocal.time - gameLocal.GetMSec(), gameLocal.time, delta );
	delta += additionalDelta;

	idStr alignmentJoint;
    if (0) {
	//if ( g_vehicleMode.GetInteger() != 0 && spawnArgs.GetString( "alignment_joint", 0, alignmentJoint ) ) {
		idVec3 offset;
		idMat3 axis;
		GetJointWorldTransform( animator.GetJointHandle( alignmentJoint ), gameLocal.time, offset, axis );
		delta *= axis;
	} else {
		viewAxis = viewAngles.ToMat3() * physicsObj.GetGravityAxis();
		delta *= viewAxis;
	}
			
	physicsObj.SetDelta( delta );
	additionalDelta.Zero();
}

/*
================
jkVehicleAnimated::GetAxis
================
*/
const idMat3& jkVehicleAnimated::GetAxis( int id ) const {
	return viewAxis;
}

/*
================
jkVehicleAnimated::WriteToSnapshot
================
*/
void jkVehicleAnimated::WriteToSnapshot( idBitMsgDelta &msg ) const {
	jkVehicle::WriteToSnapshot ( msg );
	
 	physicsObj.WriteToSnapshot( msg );
	msg.WriteFloat( viewAngles[0] );
	msg.WriteFloat( viewAngles[1] );
	msg.WriteFloat( viewAngles[2] );
}

/*
================
jkVehicleAnimated::ReadFromSnapshot
================
*/
void jkVehicleAnimated::ReadFromSnapshot( const idBitMsgDelta &msg ) {
	jkVehicle::ReadFromSnapshot ( msg );

	physicsObj.ReadFromSnapshot( msg );
	viewAngles[0] = msg.ReadFloat( );
	viewAngles[1] = msg.ReadFloat( );
	viewAngles[2] = msg.ReadFloat( );
}

/*
================
jkVehicleAnimated::Save
================
*/
void jkVehicleAnimated::Save ( idSaveGame *savefile ) const {
	savefile->WriteVec3( storedPosition );

	savefile->WriteStaticObject ( physicsObj );

	savefile->WriteAngles ( viewAngles );
	savefile->WriteFloat ( turnRate );

	// why are we writing out the viewAxis here???
	savefile->WriteMat3 ( viewAxis );

	// TEMP
	animator.Save( savefile );
}

/*
================
jkVehicleAnimated::Restore
================
*/
void jkVehicleAnimated::Restore ( idRestoreGame *savefile ) {
	savefile->ReadVec3( storedPosition );

	physicsObj.SetSelf( this );
	physicsObj.SetClipModel( new idClipModel( GetPhysics()->GetClipModel() ), 1.0f );
	savefile->ReadStaticObject ( physicsObj );
	RestorePhysics ( &physicsObj );
	physicsObj.EnableClip();

	savefile->ReadAngles ( viewAngles );
	savefile->ReadFloat ( turnRate );

	savefile->ReadMat3 ( viewAxis );

	// TEMP - restore animator, because it was cleared when loading the AF
	animator.Restore( savefile );
	animator.GetJoints( &renderEntity.numJoints, &renderEntity.joints );
	animator.GetBounds( gameLocal.time, renderEntity.bounds );
	// TEMP

	additionalDelta.Zero();
}

/*
================
jkVehicleAnimated::GetPhysicsToVisualTransform
================
*/
bool jkVehicleAnimated::GetPhysicsToVisualTransform ( idVec3 &origin, idMat3 &axis ) {
	origin = modelOffset;
	axis = viewAxis;
	return true;
}

/*
================
jkVehicleAnimated::RunPrePhysics
================
*/
void jkVehicleAnimated::RunPrePhysics ( void ) {
	storedPosition = physicsObj.GetOrigin();
}

/*
================
jkVehicleAnimated::RunPostPhysics
================
*/
void jkVehicleAnimated::RunPostPhysics ( void ) {

	if ( autoCorrectionBegin + 3000 > static_cast<unsigned>( gameLocal.time )) {
		autoCorrectionBegin = 0;

		idVec3 delta;
		idVec3 actualDelta = physicsObj.GetOrigin() - storedPosition;

		animator.GetDelta( gameLocal.time - gameLocal.GetMSec(), gameLocal.time, delta );

		if ( !autoCorrectionBegin && delta.LengthSqr() * 0.2f > actualDelta.LengthSqr() ) {
			autoCorrectionBegin = gameLocal.time;
		}
	}
}


