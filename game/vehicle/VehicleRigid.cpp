//----------------------------------------------------------------
// Vehicle.cpp
//
// Copyright 2002-2004 Raven Software
//----------------------------------------------------------------




#include "../Game_local.h"
#include "Vehicle.h"

CLASS_DECLARATION( jkVehicle, jkVehicleRigid )
END_CLASS

jkVehicleRigid::jkVehicleRigid ( void ) {
}

jkVehicleRigid::~jkVehicleRigid ( void ) {
	SetPhysics( NULL );
}

/* 
================
jkVehicleRigid::Spawn
================
*/
void jkVehicleRigid::Spawn( void ) {
	physicsObj.SetSelf( this );	

	SetClipModel ( );

	physicsObj.SetOrigin( GetPhysics()->GetOrigin ( ) );
	physicsObj.SetAxis ( GetPhysics()->GetAxis ( ) );
	physicsObj.SetContents( CONTENTS_BODY );
	physicsObj.SetClipMask( MASK_PLAYERSOLID ); //physicsObj.SetClipMask( MASK_PLAYERSOLID|CONTENTS_VEHICLECLIP );
	physicsObj.SetFriction ( spawnArgs.GetFloat ( "friction_linear", "1" ), spawnArgs.GetFloat ( "friction_angular", "1" ), spawnArgs.GetFloat ( "friction_contact", "1" ) );
	physicsObj.SetBouncyness ( spawnArgs.GetFloat ( "bouncyness", "0.6" ) );
	physicsObj.SetGravity( gameLocal.GetGravity() );
	SetPhysics( &physicsObj );
	
	animator.CycleAnim ( ANIMCHANNEL_ALL, animator.GetAnim( spawnArgs.GetString( "anim", "idle" ) ), gameLocal.time, 0 );	

	BecomeActive( TH_THINK );		
}

/*
================
jkVehicleRigid::SetClipModel
================
*/
void jkVehicleRigid::SetClipModel ( void ) {
	idStr			clipModelName;
	idTraceModel	trm;
	float			mass;

	// rebuild clipmodel
	spawnArgs.GetString( "clipmodel", "", clipModelName );

	// load the trace model
	if ( clipModelName.Length() ) {
		if ( !collisionModelManager->TrmFromModel(  clipModelName, trm ) ) {
			gameLocal.Error( "jkVehicleRigid '%s': cannot load collision model %s", name.c_str(), clipModelName.c_str() );
			return;
		}

		physicsObj.SetClipModel( new idClipModel( trm ), spawnArgs.GetFloat ( "density", "1" ) );
	} else {
		physicsObj.SetClipModel( new idClipModel( GetPhysics()->GetClipModel() ), spawnArgs.GetFloat ( "density", "1" ) );
	}

	if ( spawnArgs.GetFloat ( "mass", "0", mass ) && mass > 0 )	{
		physicsObj.SetMass ( mass );
	}
}


/*
================
jkVehicleRigid::RunPrePhysics
================
*/
void jkVehicleRigid::RunPrePhysics ( void ) {
	storedVelocity = physicsObj.GetLinearVelocity();
}

/*
================
jkVehicleRigid::RunPostPhysics
================
*/
void jkVehicleRigid::RunPostPhysics ( void ) {
	
	if ( autoCorrectionBegin + 1250 > static_cast<unsigned>( gameLocal.time )) {
		autoCorrectionBegin = 0;

		float lengthSq = physicsObj.GetLinearVelocity().LengthSqr();
		if ( !autoCorrectionBegin && ( ( storedVelocity * 0.4f ).LengthSqr() >= lengthSq ) || ( lengthSq < 0.01f ) ) {
			autoCorrectionBegin = gameLocal.time;
		}
	}
    

	//if ( g_debugVehicle.GetInteger() == 10 ) {
		//gameLocal.Printf( "Speed: %f\n", physicsObj.GetLinearVelocity().Length() );
	//}
}

/*
================
jkVehicleRigid::WriteToSnapshot
================
*/
void jkVehicleRigid::WriteToSnapshot( idBitMsgDelta &msg ) const {
	jkVehicle::WriteToSnapshot( msg );
	physicsObj.WriteToSnapshot( msg );
}

/*
================
jkVehicleRigid::ReadFromSnapshot
================
*/
void jkVehicleRigid::ReadFromSnapshot( const idBitMsgDelta &msg ) {
	jkVehicle::ReadFromSnapshot( msg );
	physicsObj.ReadFromSnapshot( msg );
}

/*
================
jkVehicleRigid::Save
================
*/
void jkVehicleRigid::Save ( idSaveGame *savefile ) const {

	savefile->WriteVec3 ( storedVelocity ); // cnicholson: Added unsaved var
	savefile->WriteStaticObject ( physicsObj );
}

/*
================
jkVehicleRigid::Restore
================
*/
void jkVehicleRigid::Restore ( idRestoreGame *savefile ) {

	savefile->ReadVec3 ( storedVelocity ); // cnicholson: Added unrestored var

	physicsObj.SetSelf( this );	
	
	SetClipModel ( );

	savefile->ReadStaticObject ( physicsObj );
	RestorePhysics( &physicsObj );
}

/*
=====================
jkVehicleRigid::SkipImpulse
=====================
*/
bool jkVehicleRigid::SkipImpulse( idEntity* ent, int id ) {	
	return false;
}
