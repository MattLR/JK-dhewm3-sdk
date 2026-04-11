//----------------------------------------------------------------
// Vehicle.cpp
//
// Copyright 2002-2004 Raven Software
//----------------------------------------------------------------




#include "../Game_local.h"
#include "../Projectile.h"
#include "Vehicle.h"
#include "../Actor.h"
#include "../Moveable.h"

#define VEHICLE_CRASH_DELAY		500
#define VEHICLE_HAZARD_TIMEOUT	5000
#define VEHICLE_LOCK_TIMEOUT	5000

//const idEventDef EV_LaunchProjectiles( "launchProjectiles", "d" );
const idEventDef EV_HUDShockWarningOff( "<HUDShockWarningOff>");
const idEventDef EV_StalledRestart( "<StalledRestart>", "dd" );
const idEventDef EV_GetViewAngles(  "getViewAngles", NULL, 'v' );

CLASS_DECLARATION( idActor, jkVehicle )
	//EVENT( EV_Door_Lock,				jkVehicle::Event_Lock )
	//EVENT( EV_Door_IsLocked,			jkVehicle::Event_IsLocked )
	//EVENT( AI_EnableMovement,			jkVehicle::Event_EnableMovement )
	//EVENT( AI_DisableMovement,			jkVehicle::Event_DisableMovement )
	EVENT( EV_Player_EnableWeapon,		jkVehicle::Event_EnableWeapon )
	EVENT( EV_Player_DisableWeapon,		jkVehicle::Event_DisableWeapon )
	//EVENT( AI_EnableClip,				jkVehicle::Event_EnableClip )
	//EVENT( AI_DisableClip,				jkVehicle::Event_DisableClip )
	EVENT( EV_Activate,					jkVehicle::Event_Activate )
	//EVENT( EV_LaunchProjectiles,		jkVehicle::Event_LaunchProjectiles )
	//EVENT( AI_SetScript,				jkVehicle::Event_SetScript )
	//EVENT( AI_SetHealth,				jkVehicle::Event_SetHealth )
	EVENT( EV_HUDShockWarningOff,		jkVehicle::Event_HUDShockWarningOff )
	EVENT( EV_StalledRestart,			jkVehicle::Event_StalledRestart )
	EVENT( EV_GetViewAngles,			jkVehicle::Event_GetViewAngles )
END_CLASS

/*
=====================
jkVehicle::jkVehicle
=====================
*/
jkVehicle::jkVehicle ( void ) {
	autoRight			= false;
	hud					= NULL;
	shieldModel			= NULL;
	shieldMaxHealth		= 0;
	hazardWarningTime	= 0;
	lockWarningTime		= 0;
	godModeDamage		= 0;
	drivers				= 0;

	autoCorrectionBegin	= 0;

	//crashEffect			= 0;
	crashTime			= 0;
	crashNextSound		= 0;

	fl.networkSync		= true;
}

jkVehicle::~jkVehicle ( void ) {
	int i;
	
	// Force all the drivers out
	for ( i = 0; i < positions.Num(); i ++ ) {
		idActor* driver = positions[i].GetDriver ( );
		if ( !driver ) {
			continue;
		}
		driver->ProcessEvent ( &AI_ExitVehicle, true );
	}

	if ( shieldModel ) {
		shieldModel->Unlink();
		delete shieldModel;
	}

	positions.Clear ( );
}

/*
================
jkVehicle::Spawn
================
*/
void jkVehicle::Spawn( void ) {
	const char* temp;

	memset ( &vfl, 0, sizeof(vfl) );

	SetPositions ( );

	healthMax		   = health;
	healthLow		   = spawnArgs.GetInt ( "lowhealth", va("%d", health / 10 ) );	
	damageStaticChance = spawnArgs.GetFloat ( "damageStaticChance", "0" );				
	crashSpeedSmall	   = spawnArgs.GetFloat ( "crashSpeedSmall",  "50" );
	crashSpeedMedium   = spawnArgs.GetFloat ( "crashSpeedMedium", "125" );
	crashSpeedLarge	   = spawnArgs.GetFloat ( "crashSpeedLarge",  "200" );
	crashDamage        = spawnArgs.GetString ( "def_crashDamage" );

	healthRegenDelay	= SEC2MS(spawnArgs.GetFloat( "healthRegenDelay", "0" ));
	healthRegenRate		= spawnArgs.GetInt( "healthRegenRate", "0" );
	healthRegenAmount.Init( gameLocal.time, 0, healthMax, healthMax );

	vfl.disableMovement = spawnArgs.GetBool ( "disableMovement", "0" );
	vfl.disableWeapons  = spawnArgs.GetBool ( "disableWeapon", "0" );
	vfl.scripted		= 0;
	vfl.flipEject		= spawnArgs.GetBool( "allowFlipEject", "1" );

	health = spawnArgs.GetInt ( "health", "100" );
	fl.takedamage = ( health > 0 );

	// Load the HUD
	/*
	if ( NULL != ( temp = spawnArgs.GetString( "gui_hud", "" ) ) ) {
		hud = uiManager->FindGui( temp, true, false, false );
		if ( hud ) {
			hud->SetStateInt ( "vehicle_id", spawnArgs.GetInt ( "hudid" ) );
			hud->Activate( true, gameLocal.time );
		}
	}
	*/

	// Get shield parameters
	shieldMaxHealth  = spawnArgs.GetInt ( "shieldHealth", "0" );	
	shieldRegenTime  = SEC2MS ( spawnArgs.GetFloat ( "shieldRegenTime", "0" ) );
	shieldRegenDelay = SEC2MS ( spawnArgs.GetFloat ( "shieldRegenDelay", "0" ) );
	shieldHitTime    = 0;	
	shieldHealth.Init ( gameLocal.time, 0, shieldMaxHealth, shieldMaxHealth );
	
	SetCombatModel();
		
	cachedContents = GetPhysics()->GetContents();

	//funcs.enter.Init( spawnArgs.GetString( "enter_script" ) );
	//funcs.exit.Init( spawnArgs.GetString( "exit_script" ) );

	crashVelocitySmall	= spawnArgs.GetFloat( "crashVelocitySmall", "0" );
	crashVelocityMedium	= spawnArgs.GetFloat( "crashVelocityMedium", "0" );
	crashVelocityLarge	= spawnArgs.GetFloat( "crashVelocityLarge", "0" );

	alwaysImpactDamage	= spawnArgs.GetBool( "alwaysImpactDamage", "0" );

	// precache hard-coded entitydefs
	declManager->FindType( DECL_ENTITYDEF, "damage_gev_collision_self", false );
	declManager->FindType( DECL_ENTITYDEF, "damage_gev_collision", false );
}

/*
================
jkVehicle::Save
================
*/
void jkVehicle::Save ( idSaveGame *savefile ) const {
	int i;

	savefile->WriteInt ( positions.Num ( ) );	
	for ( i = 0; i < positions.Num(); i ++ ) {
		positions[i].Save ( savefile );
	}
	savefile->WriteInt ( drivers );
	
	savefile->WriteUserInterface( hud, false );

	savefile->WriteFloat ( crashSpeedSmall );
	savefile->WriteFloat ( crashSpeedMedium );
	savefile->WriteFloat ( crashSpeedLarge );
	savefile->WriteString ( crashDamage );
	//crashEffect.Save ( savefile );
	savefile->WriteInt ( crashNextSound );
	savefile->WriteInt ( crashTime );

	savefile->WriteFloat ( autoRightDir );
	savefile->WriteBool ( autoRight );
	
	savefile->WriteInt( autoCorrectionBegin );

	savefile->Write( &vfl, sizeof( vfl ) );

	savefile->WriteFloat ( damageStaticChance );
	
	savefile->WriteFloat ( shieldMaxHealth );
	//savefile->WriteInterpolate( shieldHealth );
	savefile->WriteInt ( shieldHitTime );
	savefile->WriteFloat ( shieldRegenTime );
	savefile->WriteInt ( shieldRegenDelay );
// TOSAVE: idClipModel*				shieldModel;

	savefile->WriteInt( healthRegenDelay );
	savefile->WriteInt( healthRegenRate );
	//savefile->WriteInterpolate( healthRegenAmount );

	savefile->WriteInt ( hazardWarningTime );
	savefile->WriteInt ( lockWarningTime );
	savefile->WriteInt ( healthMax );
	savefile->WriteInt ( healthLow );
	savefile->WriteInt ( godModeDamage );

	savefile->WriteInt ( cachedContents );

	//funcs.enter.Save( savefile );
	//funcs.exit.Save ( savefile );

// cnicholson: Don't save crash Velocities, they are assigned in Restore
//	savefile->WriteFloat( crashVelocitySmall );	// cnicholson: Added unsaved var
//	savefile->WriteFloat( crashVelocityMedium );// cnicholson: Added unsaved var
//	savefile->WriteFloat( crashVelocityLarge );	// cnicholson: Added unsaved var

// TOSAVE: idList< idEntityPtr< idGuidedProjectile > >	incomingProjectiles;

}

/*
================
jkVehicle::Restore
================
*/
void jkVehicle::Restore ( idRestoreGame *savefile ) {
	int		i;
	int		num;

	savefile->ReadInt ( num );
	positions.Clear();
	positions.SetNum ( num );
	for ( i = 0; i < num; i ++ ) {
		positions[i].Restore ( savefile );
	}
	savefile->ReadInt ( drivers );
	
	//savefile->ReadUserInterface( hud, &spawnArgs );

	savefile->ReadFloat ( crashSpeedSmall );
	savefile->ReadFloat ( crashSpeedMedium );
	savefile->ReadFloat ( crashSpeedLarge );
	savefile->ReadString ( crashDamage );
	//crashEffect.Restore ( savefile );
	savefile->ReadInt ( crashNextSound );
	savefile->ReadInt ( crashTime );
	
	savefile->ReadFloat ( autoRightDir );
	savefile->ReadBool ( autoRight );
	
	savefile->ReadInt( (int&)autoCorrectionBegin );

	savefile->Read( &vfl, sizeof( vfl ) );

	savefile->ReadFloat ( damageStaticChance );
	
	savefile->ReadFloat ( shieldMaxHealth );
	//savefile->ReadInterpolate( shieldHealth );
	savefile->ReadInt ( shieldHitTime );
	savefile->ReadFloat ( shieldRegenTime );
	savefile->ReadInt ( shieldRegenDelay );
// TORESTORE: idClipModel*				shieldModel;

	savefile->ReadInt( healthRegenDelay );
	savefile->ReadInt( healthRegenRate );
	//savefile->ReadInterpolate( healthRegenAmount );

	savefile->ReadInt ( hazardWarningTime );
	savefile->ReadInt ( lockWarningTime );
	savefile->ReadInt ( healthMax );
	savefile->ReadInt ( healthLow );
	savefile->ReadInt ( godModeDamage );

	savefile->ReadInt ( cachedContents );

	//funcs.enter.Restore ( savefile );
	//funcs.exit.Restore ( savefile );

	SetCombatModel ( );

	crashVelocitySmall	= spawnArgs.GetFloat( "crashVelocitySmall", "0" );
	crashVelocityMedium	= spawnArgs.GetFloat( "crashVelocityMedium", "0" );
	crashVelocityLarge	= spawnArgs.GetFloat( "crashVelocityLarge", "0" );

	alwaysImpactDamage	= spawnArgs.GetBool( "alwaysImpactDamage", "0" );

	// precache hard-coded entitydefs
	declManager->FindType( DECL_ENTITYDEF, "damage_gev_collision_self", false );
	declManager->FindType( DECL_ENTITYDEF, "damage_gev_collision", false );
}

/*
================
jkVehicle::SetPositions
================
*/
void jkVehicle::SetPositions ( void ) {
	int					positionCount;
	int					index;
	const idKeyValue*	kv;
	
	// Count the positions first so we can allocate them all at once
	positionCount = 0;
	kv = spawnArgs.MatchPrefix( "def_position", NULL );
	while ( kv ) {
		positionCount++;
		kv = spawnArgs.MatchPrefix( "def_position", kv );
	}
	
	// Every vehicle needs a def_position in it's def file
	if ( positionCount == 0)	{
		//gameLocal.Error ( "Vehicle '%s' has no def_position entries.", name.c_str() );
		gameLocal.Warning ( "Vehicle '%s' has no def_position entries.", name.c_str() );
		positionCount = 1;
		spawnArgs.Set( "def_position", "vehicle_ai_null_position" );
	}

	// Initialize the positions
	positions.SetNum ( positionCount );

	// Initialize all of the positions
	index = 0;
	kv = spawnArgs.MatchPrefix( "def_position", NULL );
	while ( kv ) {	
		const idDict* dict;
		
		// Get the position dictionary
		dict = gameLocal.FindEntityDefDict ( kv->GetValue(), false );
		if ( !dict ) {
			gameLocal.Error ( "Invalid vehicle part definition '%'", kv->GetValue().c_str() );
		}
		
		// Initialize the position
		positions[index++].Init ( this, *dict );					
				
		kv = spawnArgs.MatchPrefix( "def_position", kv );
	}		
		
}

/*
================
jkVehicle::SetCombatModel
================
*/
void jkVehicle::SetCombatModel ( void ) {
	idActor::SetCombatModel ( );
		
	if ( shieldMaxHealth ) {
		idBounds bounds;
		bounds.Clear ( );
		bounds.AddPoint ( spawnArgs.GetVector ( "shieldMins", "0 0 0" ) );
		bounds.AddPoint ( spawnArgs.GetVector ( "shieldMaxs", "0 0 0" ) );

		if ( shieldModel ) {
			shieldModel->Unlink();
			delete shieldModel;
			shieldModel = NULL;
		}

		//twhitaker: dodecahedron support
		idStr shieldModelName;
		if ( spawnArgs.GetString( "shieldModel", "", shieldModelName ) ) {
			if ( shieldModelName.Length() && !shieldModelName.Icmp( "dodecahedron" ) ) {
				idTraceModel trm;
				trm.SetupDodecahedron ( GetPhysics()->GetBounds() );
				shieldModel = new idClipModel ( trm );
			}
		}
		//twhitaker: end

		if ( !shieldModel ) {
			shieldModel = new idClipModel ( idTraceModel ( bounds, spawnArgs.GetInt ( "shieldSides", "6" ) ) );
		}

		shieldModel->SetOwner ( this );
		shieldModel->SetContents ( CONTENTS_SOLID );
	} else {
		shieldModel = NULL;
	}
}

/*
================
jkVehicle::LinkCombat
================
*/
void jkVehicle::LinkCombat ( void ) {
	if ( fl.hidden ) {
		return;
	}

    
	if ( g_debugVehicle.GetInteger() == 1 && shieldModel ) {
		//collisionModelManager->DrawModel( shieldModel->GetCollisionModel(), renderEntity.origin, renderEntity.axis, vec3_origin, mat3_identity, 0.0f );
		collisionModelManager->DrawModel( shieldModel->Handle(), renderEntity.origin, renderEntity.axis, vec3_origin, 0 );
	}
    

	if ( shieldHealth.GetCurrentValue ( gameLocal.time ) > 0 && HasDrivers ( ) ) {
		// RAVEN BEGIN
		// ddynerman: multiple clip worlds
		//shieldModel->Link( this, 0, renderEntity.origin, renderEntity.axis );gameLocal.clip
        shieldModel->Link(gameLocal.clip, this, 0, renderEntity.origin, renderEntity.axis );

		// RAVEN END
		if ( combatModel ) {
			combatModel->Unlink ( );
		}
	} else {
		// RAVEN BEGIN
		// ddynerman: multiple clip worlds
		//combatModel->Link( this, 0, renderEntity.origin, renderEntity.axis, modelDefHandle );	
        combatModel->Link(gameLocal.clip, this, 0, renderEntity.origin, renderEntity.axis );
		// RAVEN END
		
		if ( shieldModel ) {
			shieldModel->Unlink ( );
		}
	}
}

/*
================
jkVehicle::ClientPredictionThink
================
*/
void jkVehicle::ClientPredictionThink ( void ) {
	Think ( );
}

/*
================
jkVehicle::WriteToSnapshot
================
*/
void jkVehicle::WriteToSnapshot ( idBitMsgDelta &msg ) const {
	int i;
	// TODO: Check that this conditional write to delta message is OK
	for ( i = 0; i < positions.Num(); i ++ ) {
		positions[i].WriteToSnapshot ( msg );
	}
}

/*
================
jkVehicle::ReadFromSnapshot
================
*/
void jkVehicle::ReadFromSnapshot ( const idBitMsgDelta &msg ) {
	int i;
	for ( i = 0; i < positions.Num(); i ++ ) {
		positions[i].ReadFromSnapshot ( msg );
	}
}

/*
================
jkVehicle::UpdateState
================
*/
void jkVehicle::UpdateState ( void ) {
	jkVehiclePosition* pos;
  	pos = &positions[0];
  	
  	vfl.forward  = (pos->IsOccupied() && pos->mInputCmd.forwardmove > 0);
  	vfl.backward = (pos->IsOccupied() && pos->mInputCmd.forwardmove < 0);
  	vfl.right    = (pos->IsOccupied() && pos->mInputCmd.rightmove > 0);
  	vfl.left     = (pos->IsOccupied() && pos->mInputCmd.rightmove < 0);	
	vfl.driver   = pos->IsOccupied();
	vfl.strafe	 = (pos->IsOccupied() && pos->mInputCmd.buttons & BUTTON_7 );
}


/*
================
jkVehicle::Think
================
*/
void jkVehicle::Think ( void ) {	
	
	UpdateState();
	UpdateAnimState ( );
	UpdateIncomingProjectiles();

	// If we are the current debug entity then output some info to the hud
    /*
	if ( gameDebug.IsHudActive ( DBGHUD_VEHICLE, this ) ) {
		gameDebug.SetInt ( "vehicle", 1 );
		gameDebug.SetInt ( "shields", shieldHealth.GetCurrentValue(gameLocal.time) );
		gameDebug.SetInt ( "positions", positions.Num() );
		gameDebug.SetInt ( "drivers", drivers );
	}
	
	if ( g_debugVehicle.GetInteger() == 1 ) {
		idMat3 flatAxis = idAngles(0,GetPhysics()->GetAxis().ToAngles().yaw,0).ToMat3();
		gameRenderWorld->DebugLine ( colorOrange, GetPhysics()->GetCenterMass(), GetPhysics()->GetCenterMass() + 50.0f * flatAxis[0] );
		gameRenderWorld->DebugLine ( colorYellow, GetPhysics()->GetCenterMass(), GetPhysics()->GetCenterMass() + 50.0f * flatAxis[1] );
		gameRenderWorld->DebugLine ( colorCyan, GetPhysics()->GetCenterMass(), GetPhysics()->GetCenterMass() - 50.0f *  flatAxis[2] );

		gameRenderWorld->DebugLine ( colorRed, GetPhysics()->GetCenterMass(), GetPhysics()->GetCenterMass() + 50.0f * GetPhysics()->GetAxis()[0] );
		gameRenderWorld->DebugLine ( colorGreen, GetPhysics()->GetCenterMass(), GetPhysics()->GetCenterMass() + 50.0f * GetPhysics()->GetAxis()[1] );
		gameRenderWorld->DebugLine ( colorBlue, GetPhysics()->GetCenterMass(), GetPhysics()->GetCenterMass() + 50.0f * GetPhysics()->GetAxis()[2] );
	}
	*/
    

/* VEH_FIXME: where to put this?
	// For engine glow
	renderEntity.shaderParms[7] = mEngineStatus.GetCurrentValue ( gameLocal.time );
*/

	if( thinkFlags & TH_THINK ) {
		int	i;

		for( i = 0; i < positions.Num(); i++ ) {
			positions[i].RunPrePhysics( );
		}

		if ( autoRight ) {
			if( idMath::Fabs( idMath::AngleNormalize180( renderEntity.axis.ToAngles().roll ) ) < 30.0f ) {
				GetPhysics()->SetLinearVelocity( vec3_origin );
				autoRight = false;
			} else {
				idVec3  upSpeed = -GetPhysics()->GetGravityNormal() * 72.0f;
				idMat3	axis    = GetPhysics()->GetAxis();
				idAngles angles = axis.ToAngles();
				idAngles newAngles;
				
				//FIXME1 Dynamix fix for time not ticks
				if (angles.roll < 0) {
					newAngles = axis.ToAngles() + idAngles(0, 0, (angles.roll+180)/30);
				} else {
					newAngles = axis.ToAngles() + idAngles(0, 0, (angles.roll-180)/30);
				}

				idMat3 newAxis = newAngles.ToMat3();
				idMat3 flatAxis = idAngles(0, 0, 0).ToMat3();
				
				// Rotate around the forward axis
                //Add rotaterelative I guess FIXME1 Dynamix
				//axis.RotateRelative ( 0, (renderEntity.axis.ToAngles().roll<0?180:-180) * MS2SEC(gameLocal.GetMSec()) * 1.5f );					
				//GetPhysics()->SetAxis ( axis2 );
				//axis = renderEntity.axis;
				GetPhysics()->SetAxis(newAxis);
				//renderEntity.axis = flatAxis;
	
				// Move up to make room for the rotation					
				GetPhysics()->SetLinearVelocity( upSpeed );
			}
		} 
		
		

		// twhitaker: nothing special here
		RunPrePhysics();

		// Run the physics
		RunPhysics();
		
		// twhitaker: nothing special here
		RunPostPhysics();
		
		// Give each position a chance to think after physics has been run
		vfl.godmode = false;
		fl.notarget = false;
		
		for( i = 0; i < positions.Num(); i++ ) {
			
			positions[i].RunPostPhysics( );
			
			// Include the eye origin into the bounds to ensure the driver
			// is inside the render bounds when the vehicle is rendered.
			if ( positions[i].IsOccupied ( ) ) {
				AddToBounds ( positions[i].GetEyeOrigin ( ) );

				// Transfer godmode from driving players
				if ( positions[i].mDriver && positions[i].mDriver->IsType ( idPlayer::Type ) ) {
					//if ( static_cast<idPlayer*>(positions[i].mDriver.GetEntity())->godmode ) {
					//Temp for model alignment, think it should be renderentity or something FIXME1 Dynamix
					//positions[i].GetDriver()->GetPhysics()->SetAxis(GetPhysics()->GetAxis());
					//	vfl.godmode = true;
					//}
				}
				
				// Inherit notarget if our driver has it on
				if ( positions[i].mDriver && positions[i].mDriver->fl.notarget ) {
					fl.notarget = true;
				}
			//Temp for model aligntment, think it should be done with renderenttiy or something, check the QW code FIXME1 Dynamix
			//GetPosition(0)->GetDriver()->GetPhysics()->SetAxis(GetPhysics()->GetAxis());
			}
		}
			
		
		// If the vehicle is flipped then kick out all drivers
		//FIXME1 crashing currently, locked_flip_death was working I think but will check again
		//Can exit vehicle when it's flipped so not that, foce bool works fine also - getphysics check breaking upside down maybe?, although
		//I if flip death works don't know how that could be possible
		//Outside if works fine, one of the inners
		
		if ( HasDrivers() && IsFlipped ( ) && GetPhysics()->GetLinearVelocity ( ).LengthSqr() < (100.0f * 100.0f)) {
			/*
			if ( spawnArgs.GetBool( "locked_flip_death", false ) ) {
				Killed( this, this, 999999999, GetPhysics()->GetLinearVelocity(), 0 );
			} else if ( vfl.flipEject ) {
				for ( i = 0; i < positions.Num(); i ++ ) {
					idActor* driver = positions[i].GetDriver ();
					if ( driver ) {
						driver->ProcessEvent ( &AI_ExitVehicle, true );
					}
				}
			}
			*/
		}
		
	}
	
	// Regenerate the shield
	if ( shieldMaxHealth && shieldHealth.GetCurrentValue(gameLocal.time) < shieldMaxHealth ) {		
		if ( gameLocal.time > shieldHitTime + shieldRegenDelay && shieldHealth.IsDone ( gameLocal.time ) ) {
			StopSound ( SND_CHANNEL_BODY2, false );
			StartSound ( "snd_shieldRecharge", SND_CHANNEL_BODY2, 0, false, NULL );

			float regenTime = shieldHealth.GetCurrentValue(gameLocal.time);
			regenTime /= (float)shieldMaxHealth;
			regenTime = 1.0f - regenTime;
			regenTime *= (float)shieldRegenTime;
			shieldHealth.Init ( gameLocal.time, regenTime, shieldHealth.GetCurrentValue(gameLocal.time), shieldMaxHealth );
		}
	}
	

	// Regenerate health, if needed
	// do nothing if the vehicle is at full hit points
	if (health < healthMax) 
	{	
		// also do nothing if we've been damaged less than healthRegenDelay seconds ago
		if ( healthRegenRate && shieldHitTime + healthRegenDelay <= gameLocal.time )
		{
			// do we need to start the interpolation?
			if ( healthRegenAmount.IsDone( gameLocal.time ))
			{
				healthRegenAmount.Init( gameLocal.time, SEC2MS((float)(healthMax - health)/healthRegenRate), 
										health, healthMax );
			}
			else // get our interpolated health value for the current time.
			{
				health = healthRegenAmount.GetCurrentValue( gameLocal.time );
			}
		}
	}

	// Check hazards
	if ( hazardWarningTime && gameLocal.time > hazardWarningTime + VEHICLE_HAZARD_TIMEOUT ) {
		hazardWarningTime = 0;
		StartSound ( "snd_voiceSafe", SND_CHANNEL_VOICE, 0, false, NULL );

		if ( renderEntity.gui[0] ) {
			renderEntity.gui[0]->HandleNamedEvent ( "info_safe" );
		}		
	}
	

	// Stop the crash effect if no collide happened this frame
	//if ( crashEffect && crashTime != gameLocal.time ) {
	//	crashEffect->Stop ( );
	//	crashEffect = NULL;
	//}		

	UpdateAnimation();
	
	
	if ( thinkFlags & TH_UPDATEVISUALS ) {
		Present();
		LinkCombat();
	}
	

	if (spawnArgs.GetBool("touchtriggers")) {
		TouchTriggers();
	}
}

/*
================
jkVehicle::UpdateDrivers
================
*/
void jkVehicle::UpdateDrivers ( int delta ) {
	int oldDrivers = drivers;

	drivers = idMath::ClampInt ( 0, positions.Num(), drivers + delta );
	
	if ( drivers && !oldDrivers ) {	
		if ( fl.takedamage ) {	
			//aiManager.AddTeammate ( this );
		}
		
		// script function for spawning guys
		const char* temp;
		if( spawnArgs.GetString( "call_enter", "", &temp ) && *temp ) {
			//gameLocal.CallFrameCommand ( this, temp );
		}		
	} else if ( !drivers ) {
		//aiManager.RemoveTeammate ( this );

		refSound.listenerId = entityNumber + 1;		

		// script function for spawning guys
		const char* temp;
		if( spawnArgs.GetString( "call_exit", "", &temp ) && *temp ) {
			//gameLocal.CallFrameCommand ( this, temp );
		}		
	}
}

/*
================
jkVehicle::GetAxis
================
*/
const idMat3& jkVehicle::GetAxis( int id ) const {
	return GetPhysics()->GetAxis( id );
}

/*
================
jkVehicle::GetOrigin
================
*/
const idVec3& jkVehicle::GetOrigin( int id ) const {
	return GetPhysics()->GetOrigin();
}

/*
================
jkVehicle::GetEyePosition
================
*/
void jkVehicle::GetEyePosition ( int pos, idVec3& origin, idMat3& axis ) {
	axis   = positions[pos].GetEyeAxis ( );
	origin = positions[pos].GetEyeOrigin ( );
}

/*
================
jkVehicle::GetDriverPosition
================
*/
void jkVehicle::GetDriverPosition ( int pos, idVec3& origin, idMat3& axis ) {
	const jkVehiclePosition *position = GetPosition( pos );
	
	position->GetDriverPosition( origin, axis );
}

/*
=====================
jkVehicle::FindClearExitPoint
=====================
*/
// FIXME: this whole function could be cleaned up
bool jkVehicle::FindClearExitPoint( int pos, idVec3& origin, idMat3& axis ) const {
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
	idMat3 identity;
	identity.Identity( );

	if( position->fl.driverVisible ) {
		// May want to do this even if the driver isn't visible
		if( position->mExitPosOffset.LengthSqr() > VECTOR_EPSILON ) {
			axis = GetPhysics()->GetAxis() * position->mExitAxisOffset;
			origin = vehicleOrigin + position->mExitPosOffset * axis;
		} else {
			origin = driverOrigin;
			//axis = (driver->IsBoundTo(this)) ? vehicleAxis : driverAxis;
		}
		return true;
	}

	// Build list
	// FIXME: try and find a cleaner way to do this
    //Add this function FIXME1 Dynamix
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

		// mekberg: vehicle bounds are resized based on rotation but not rotated with the axis. Get transformed bounds.
		vehicleAbsBounds.FromTransformedBounds( vehicleBounds, vehicleOrigin, identity );
		if( trace.fraction > 0.0f && !driverAbsBounds.IntersectsBounds(vehicleAbsBounds) ) {
			origin = trace.endpos;
			axis = vehicleAxis;
			return true;
		}
	}

	return false;
}

/*
================
jkVehicle::AddDriver

Add a driver to the vehicle at the given position.  Its possible that the given position is 
occupied, in that case the driver will be assigned the closest valid position.  If no position 
can be found then (-1) will be returned, otherwise the position the driver is now driving will
be returned.
================
*/
int jkVehicle::AddDriver ( int position, idActor* driver ) {
	int wraparound;
	
	// Ensure the given position is valid
	if ( position < 0 || position >= positions.Num() ) {
		gameLocal.Warning ( "position %d is invalid for vehicle '%s'", position, name.c_str() );
		return -1;
	}
	
	wraparound = position;
	
	do {
		// If the position isnt occupied then set the driver
		if ( !positions[position].IsOccupied ( ) ) {
			positions[position].SetDriver ( driver );
			
			// Vehicle is on same team as driver
			team = driver->team;
			
			UpdateDrivers ( 1 );
			
			// The local player will hear all private sounds of the vehicle			
			if ( driver == gameLocal.GetLocalPlayer ( ) ) {
				refSound.listenerId = driver->GetListenerId ( );
			}
						
			return position;
		}
	
		// Check the next position, wrap around if necessary
		position = (position + 1) % positions.Num();
	
	} while ( wraparound != position );
	
	return -1;
}

/*
================
jkVehicle::RemoveDriver

Removes the given driver from the vehicle.  This includes physically taking the driver out 
of the vehicle and placing them back into the world and will follow all constraints that limit
the driver from exiting the vehicle.
================
*/
bool jkVehicle::RemoveDriver ( int position, bool force ) {
	if ( !positions[position].EjectDriver ( force ) ) {
		return false;
	}
	
	UpdateDrivers ( -1 );
	return true;
}

/*
================
jkVehicle::EjectAllDrivers
================
*/
void jkVehicle::EjectAllDrivers( bool force ) {
	for( int ix = positions.Num() - 1; ix >= 0; --ix ) {
		if( !GetPosition(ix)->GetDriver() ) {
			continue;
		}

		GetPosition(ix)->GetDriver()->ProcessEvent( &AI_ExitVehicle, force );
	}
}

/*
============
jkVehicle::Damage
============
*/
void jkVehicle::Damage ( idEntity *inflictor, idEntity *attacker, const idVec3 &dir, const char *damageDefName, const float damageScale, int location ) {
	int damage;

	if ( !fl.takedamage ) {
		return;
	}

	const idDict *damageDef = gameLocal.FindEntityDefDict( damageDefName, false );
	if ( !damageDef ) {
		gameLocal.Warning( "Unknown damageDef '%s'", damageDefName );
		return;
	}

	if ( damageDef->GetBool( "ignore_vehicles", "0" ) ) {
		return;
	}

	damageDef->GetInt( "damage", "20", damage );
	damage *= damageScale;

	int save = HasDrivers() ? shieldHealth.GetCurrentValue ( gameLocal.time ) : 0;

	// If one of the drivers is the player, do the player HUD effects
	for ( int i = 0; i < positions.Num(); i++ ) {
		jkVehiclePosition & pos = positions[ i ];
        //static cast or something?

        /* FIXME1
		if ( pos.mDriver.IsValid() && pos.mDriver->IsType( idPlayer::Type ) ) {
			idPlayer & driver = static_cast< idPlayer & >( *pos.mDriver.GetEntity() );

			if ( driver.GetHud() ) {
				driver.GetHud()->HandleNamedEvent( "vehicleHit" );
			}

			if ( !stricmp( damageDef->GetString( "filter" ), "electrical" ) ) {
				driver.GetHud()->HandleNamedEvent( "electricWarningOn" );
				PostEventMS( &EV_HUDShockWarningOff, spawnArgs.GetInt( "hud_shock_warning_time", "2500" ) );

				if ( damage >= spawnArgs.GetInt( "electric_stall_damage", "20" ) ) {
					//rvClientCrawlEffect* effect;
					//effect = new rvClientCrawlEffect ( gameLocal.GetEffect( spawnArgs , "fx_electrical_stall" ), this, SEC2MS( spawnArgs.GetFloat ( "hud_shock_warning_time", "2.5" ) ) );
					//effect->Play ( gameLocal.time, false );

					if ( renderEntity.gui[ 0 ] ) {
						renderEntity.gui[ 0 ]->HandleNamedEvent( "shock_stall" );
					}

					if ( renderEntity.gui[ 1 ] ) {
						renderEntity.gui[ 1 ]->HandleNamedEvent( "shock_stall" );
					}

					vfl.stalled = true;
					PostEventMS( &EV_StalledRestart, spawnArgs.GetInt( "electric_stall_delay", "3500" ), save, damage );
				}
			}
		}*/
	}


	shieldHitTime = gameLocal.time;

	// Shields off when there is no controller
	if ( !damageDef->GetBool( "noShields", "0" ) )
	{
		if ( save > 0 )	{
			int shield;
			save = Min( save, damage );
			damage -= save;	
			
			shield = shieldHealth.GetCurrentValue ( gameLocal.time ) - save;
			shieldHealth.Init ( gameLocal.time, 0, shield, shield );
			// always stop the sound because we may be playing the recharge just now.
			StopSound ( SND_CHANNEL_BODY2, false );

			// Looping warning sound for shield
			if ( shield <= 0 && !vfl.stalled ) {
				StartSound ( "snd_shieldWarning", SND_CHANNEL_BODY2, 0, false, NULL );
			}
		}
	}

	// God Mode?
	if ( vfl.godmode && !damageDef->GetBool( "noGod" ) )  {
		godModeDamage += damage;
		damage = 0;
	}
	
	if ( !damage ) {
		return;
	}

	// Static on the gui when hit
	if ( renderEntity.gui[0] && gameLocal.random.RandomFloat() < damageStaticChance ) {
		renderEntity.gui[0]->HandleNamedEvent ( "shot_static" );
	}
 
	// Play low health warning on transition to low health value
	if ( health >= healthLow && health - damage < healthLow ) {
		StartSound ( "snd_voiceLowHealth", SND_CHANNEL_VOICE, 0, false, NULL );
	}
 
// RAVEN BEGIN
// MCG - added damage over time
	if ( 0 ) { //!inDamageEvent
		if ( damageDef->GetFloat( "dot_duration" ) ) {
			int endTime;
			if ( damageDef->GetFloat( "dot_duration" ) == -1 ) {
				endTime = -1;
			} else {
				endTime = gameLocal.GetTime() + SEC2MS(damageDef->GetFloat( "dot_duration" ));
			}
			int interval = SEC2MS(damageDef->GetFloat( "dot_interval", "0" ));
			if ( endTime == -1 || gameLocal.GetTime() + interval <= endTime ) {//post it again
			//	PostEventMS( &EV_DamageOverTime, interval, endTime, interval, inflictor, attacker, dir, damageDefName, damageScale, location );
			}
			if ( damageDef->GetString( "fx_dot", NULL ) ) {
			//	ProcessEvent( &EV_DamageOverTimeEffect, endTime, interval, damageDefName );
			}
			if ( damageDef->GetString( "snd_dot_start", NULL ) ) {
				StartSound ( "snd_dot_start", SND_CHANNEL_ANY, 0, false, NULL );
			}
		}
	}
// RAVEN END

	health -= damage;
	if ( health < 1 && damageDef->GetBool( "nonLethal" ) ) {
		health = 1;
	}
	if ( health <= 0 ) {
		// keep the driver from being killed if we're forcing undying state
        /*
		if ( g_forceUndying.GetBool() && HasDrivers() ) {
			idActor * driver = NULL;
			for ( int i = 0; i < positions.Num(); i++ ) {
				idActor * currentDriver = positions[ i ].GetDriver();
				if ( currentDriver && currentDriver->IsType( idPlayer::Type ) ) {
					driver = currentDriver;
					break;
				}
			}

			if ( driver ) {
				health = 1;
				Pain( inflictor, attacker, damage, dir, 0 );
				return;
			}
		}*/

		if ( health < -999 ) {
			health = -999;
		}

		Killed( inflictor, attacker, damage, dir, location );
	} else {
		Pain( inflictor, attacker, damage, dir, 0 );
	}
}

/*
================
jkVehicle::Killed
================
*/
void jkVehicle::Killed ( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location ) {
	int i;

	lockWarningTime   = 0;
	hazardWarningTime = 0;
	vfl.locked	      = false;
	vfl.dead		  = true;

	// Try removing the vehicle from all lists it could be part of.
	//aiManager.RemoveTeammate ( this );
	
	// Remove all drivers from the vehicle and kill them	
	for ( i = 0; i < positions.Num(); i ++ ) {
		idActor* driver = positions[i].GetDriver();
		if ( !driver ) {
			continue;
		}

		// Dump the driver out of the vehicle and kill them	
		driver->health = 0;
		driver->ProcessEvent ( &AI_ExitVehicle, true );
		driver->Killed( inflictor, attacker, damage, dir, location );
	}

	OnDeath();
	//CheckDeathObjectives();

	if ( spawnArgs.GetBool( "remove_on_death", "1" ) ) {
		StartSound ( "snd_death", SND_CHANNEL_ANY, 0, false, NULL );
		
		if ( spawnArgs.GetBool( "orient_death_fx", "0" ) ) {
		//	gameLocal.PlayEffect ( spawnArgs, "fx_death", GetPhysics()->GetAbsBounds().GetCenter(), GetPhysics()->GetAxis() );
		} else {
		//	gameLocal.PlayEffect ( spawnArgs, "fx_death", GetPhysics()->GetAbsBounds().GetCenter(), idVec3(0,0,1).ToMat3() );
		}

		Hide ( );

		PostEventMS( &EV_Remove, 0 );
	}
}

/*
================
jkVehicle::AddDamageEffect
================
*/
void jkVehicle::AddDamageEffect ( const trace_t &collision, const idVec3 &velocity, const char *damageDefName, idEntity* inflictor ) {
	// If there are still shields remaining then play a shield effect at the impact point
	if ( HasDrivers() && shieldHealth.GetCurrentValue ( gameLocal.time ) > 0 ) {
		jointHandle_t joint = animator.GetJointHandle( spawnArgs.GetString( "fx_shield_joint", "" ) );
		idVec3 dir;
		//dir = collision.c.point - GetPhysics()->GetCenterMass (); Add centre of mass
        dir = collision.c.point - GetPhysics()->GetOrigin();
		dir.NormalizeFast ( );
		if ( INVALID_JOINT == joint ) {
		//	PlayEffect ( "fx_shield", collision.c.point, dir.ToMat3(), false, vec3_origin, true );
		} else {
		//	PlayEffect ( "fx_shield", joint, false, vec3_origin, true );
		}
	}
}

/*
================
jkVehicle::Collide
================
*/
bool jkVehicle::Collide( const trace_t &collision, const idVec3 &velocity ) {
	idVec3	dir;
	float	speed;
	float	collisionSelfDamage = 0.0f;

	dir   = velocity;
	speed = dir.Normalize();

	// No collision effect when hitting a no impact surfac
//	if ( collision.c.material && collision.c.material->GetSurfaceFlags() & SURF_NOIMPACT ) {
//		return false;
//	}

	if ( vfl.dead ) {
		StartSound ( "snd_death", SND_CHANNEL_ANY, 0, false, NULL );
		if ( spawnArgs.GetBool( "orient_death_fx", "0" ) ) {
		//	gameLocal.PlayEffect ( spawnArgs, "fx_death", GetPhysics()->GetAbsBounds().GetCenter(), GetPhysics()->GetAxis() );
		} else {
		//	gameLocal.PlayEffect ( spawnArgs, "fx_death", GetPhysics()->GetAbsBounds().GetCenter(), idVec3(0,0,1).ToMat3() );
		}
		Hide ( );
		PostEventMS( &EV_Remove, 0 );
	}

	if ( !alwaysImpactDamage ) {
		if ( speed < crashSpeedSmall ) {
			return false;
		}
	}

	// When colliding with the world play a collision effect
	if ( collision.c.entityNum == ENTITYNUM_WORLD ) {
		// TODO: MAterial types
		//if ( !crashEffect ) {
		//	crashEffect = gameLocal.PlayEffect ( gameLocal.GetEffect ( spawnArgs, "fx_crash" ), collision.c.point, collision.c.normal.ToMat3(), true );
		//}	
		//if ( crashEffect )	{
		//	crashEffect->SetOrigin ( collision.c.point );
		//	crashEffect->SetAxis ( collision.c.normal.ToMat3() );
		//	crashEffect->Attenuate ( idMath::ClampFloat ( 0.01f, 1.0f, speed / (float)crashSpeedLarge ) );
		//}
	}
		
	idStr collDmgDef = "damage_gev_collision_self";
	if ( gameLocal.time > crashNextSound ) {

		float dot = dir * collision.c.normal;
		if( dot < -0.5f ) { 

		// Crash impact sounds	
		if ( speed > crashSpeedLarge ) {
			collisionSelfDamage += 50.0f;
			StartSound ( "snd_crash_large", SND_CHANNEL_ANY, 0, false, NULL );
			crashNextSound = gameLocal.time + VEHICLE_CRASH_DELAY;
	//		damage = true;
		} else if ( speed > crashSpeedMedium ) {	
			collisionSelfDamage += 25.0f;
			StartSound ( "snd_crash_medium", SND_CHANNEL_ANY, 0, false, NULL );
			crashNextSound = gameLocal.time + VEHICLE_CRASH_DELAY;
	//		damage = true;
		} else if ( speed > crashSpeedSmall ) {	
			StartSound ( "snd_crash_small", SND_CHANNEL_ANY, 0, false, NULL );
			crashNextSound = gameLocal.time + VEHICLE_CRASH_DELAY;
		}
		}
	}
	
	crashTime = gameLocal.time;
	float	vel	= GetPhysics()->GetLinearVelocity().Length();

	if ( vel > crashVelocitySmall && crashDamage.Length ( ) ) 	{
		idEntity* ent = gameLocal.entities[ collision.c.entityNum ];

		if ( ent )  {
			float   f	= vel > crashVelocityLarge ? 1.0f : idMath::Sqrt( vel - crashVelocityMedium ) * ( 1.0f / idMath::Sqrt( crashVelocityLarge - crashVelocityMedium ) );

			// Now hurt him
			//if ( !( team != gameLocal.GetLocalPlayer()->team && ent->IsType( idPlayer::Type ) ) ) { FIXME1 multiplayer stuff
			if ( !ent->IsType( idPlayer::Type ) ) {

				const idDict* damageDef	= gameLocal.FindEntityDefDict ( crashDamage, false );
				//MCG NOTE: This used to call vehicleController.GetDriver(), which was always NULL...
				idActor* theDriver		= positions[0].GetDriver();
				
				if ( ent->fl.takedamage 
					&& ent->health >= 0 
					&& !ent->spawnArgs.GetBool( "ignore_vehicle_damage" ) 
					&& (vel > 100.0f || !ent->IsType( idPlayer::Type ) ) ) {
					//MCG NOTE: now that theDriver is being set correctly, this damage will get credited to the driver (as the attacker), unlike before
					float dScale = f * ent->spawnArgs.GetFloat( "vehicle_damage_scale", "1" );
					ent->Damage( this, theDriver, dir, crashDamage, dScale, INVALID_JOINT );
					if ( vel > 100.0f ) {
						//NOTE: we PURPOSELY override this here, so you don't take damage from slamming into damageable things, only invulnerable things...
						collisionSelfDamage = ent->spawnArgs.GetFloat( "vehicle_impact_damage", "0" );
						collDmgDef = "damage_gev_collision";
					}
				}

				// ApplyImpulse doesn't like it when you give it a null driver
				// MCG: okay, now ApplyImpulse actually is getting called, 
				//		but I'm only going to allow it on idMoveables since ApplyImpulse is 
				//		rejected by idActors if it comes from an idActor and this code was 
				//		never being called before.
				if ( theDriver && damageDef && !ent->spawnArgs.GetBool("ignore_vehicle_push") && ent->IsType( idMoveable::Type ) ) {
					float push		= damageDef->GetFloat( "push" ) * speed / idMath::ClampFloat ( 0.01f, 1.0f, speed / (float)crashSpeedLarge );
					idVec3 impulse	= -push * f * collision.c.normal;
					impulse[2]		=  push * f * 0.75f;

					// Send him flying
					ent->ApplyImpulse( theDriver, collision.c.id, collision.c.point, impulse );
					
                    
					if ( g_debugVehicle.GetInteger() ) {
						gameRenderWorld->DebugArrow ( colorGreen, collision.c.point, collision.c.point + impulse, 3, 10000 );
					}
                    
				}
			}
		}
	}
	if ( collisionSelfDamage ) {
		idVec3 dmgDir = GetPhysics()->GetLinearVelocity()*-1.0f;
		Damage( this, this, dmgDir, collDmgDef.c_str(), collisionSelfDamage, 0 );
	}
	return false;
}

/*
===============
jkVehicle::Give
===============
*/
bool jkVehicle::Give( const char *statname, const char *value ) {
	if ( !idStr::Icmp( statname, "health" ) ) {
		if ( health >= healthMax ) {
			return false;
		}
		health = Min ( atoi( value ) + health, healthMax );
	}
	
	return true;
}

/*
================
jkVehicle::AutoRight
================
*/
void jkVehicle::AutoRight( idEntity* activator ) {
	if( autoRight ) {
		return;
	}

	autoRight = true;

	/*
	idVec3 vec = renderEntity.origin - activator->renderEntity.origin;
	//idVec3 vec = renderEntity.origin - renderEntity.origin;
	vec.Normalize();

	float dot = DotProduct( vec, renderEntity.axis[		1 ] );

	if( dot < 0 ) 
	{
		autoRightDir = -1.0f;
	} 
	else 
	{
		autoRightDir = 1.0f;
	}
		*/
	

}

/*
================
jkVehicle::GetPositionIndex
================
*/
int jkVehicle::GetPositionIndex ( const jkVehiclePosition* position ) const {
	int i;
	for ( i = positions.Num() - 1; i >= 0; i -- ) {
		if ( &positions[i] == position ) {
			return i;
		}
	}
	return -1;
}

/*
================
jkVehicle::UpdateHUD
================
*/
void jkVehicle::UpdateHUD ( int position, idUserInterface* gui ) {
// VEH_FIXME: godmode ?
/*
	if ( mController && mController->GetDriver() && mController->GetDriver()->IsType ( idPlayer::Type ) )
	{
		idPlayer* player = static_cast<idPlayer*>(mController->GetDriver());
		gui->State()->SetInt ( "vehicle_god", player->godmode && g_showGodDamage->integer );
		gui->State()->SetInt ( "vehicle_god_damage", godModeDamage );
	}
	else
	{
		gui->State()->SetInt ( "vehicle_god", 0 );
	}
*/
	gui->SetStateFloat( "vehicle_health", health / spawnArgs.GetFloat( "health", "1" ) );
	gui->SetStateInt ( "vehicle_armor",  0 );
	gui->SetStateInt ( "vehicle_position", position );
	gui->SetStateFloat ( "vehicle_shield", (float)shieldHealth.GetCurrentValue(gameLocal.time) / (float)shieldMaxHealth );
	gui->SetStateInt ( "vehicle_haz", hazardWarningTime != 0 );
	gui->SetStateInt ( "vehicle_locked", IsLocked ( ) );
	
	// Update position specific information	
	positions[position].UpdateHUD ( gui );	
}

/*
================
jkVehicle::UpdateCursorGUI
================
*/
void jkVehicle::UpdateCursorGUI ( int position, idUserInterface* gui ) {
	positions[position].UpdateCursorGUI ( gui );	
}

/*
================
jkVehicle::DrawHUD
================
*/
void jkVehicle::DrawHUD ( int position ) {
    return;
	//if ( !hud || gameLocal.GetLocalPlayer()->IsObjectiveUp() || gameLocal.GetLocalPlayer()->objectiveSystemOpen ) {
	//	return;
	//}
	
	UpdateHUD ( position, hud );
	
	hud->Redraw ( gameLocal.time );
}

/*
==================
jkVehicle::IssueHazardWarning
==================
*/
void jkVehicle::IssueHazardWarning ( void ) {
	if ( !hazardWarningTime ) {
		StartSound ( "snd_voiceHazard", SND_CHANNEL_VOICE, 0, false, NULL );

		if ( renderEntity.gui[0] ) {
			renderEntity.gui[0]->HandleNamedEvent ( "warning_hazard" );
		}
	}
	
	hazardWarningTime = gameLocal.time;	
}

/*
==================
jkVehicle::IssueHazardWarning
==================
*/
void jkVehicle::IssueLockedWarning ( void ) {
	if ( gameLocal.time > lockWarningTime + VEHICLE_LOCK_TIMEOUT ) {
		StartSound ( "snd_voiceNoExit", SND_CHANNEL_VOICE, 0, false, NULL );

		if ( renderEntity.gui[0] ) {
			renderEntity.gui[0]->HandleNamedEvent ( "warning_locked" );
		}

		lockWarningTime = gameLocal.time;
	}
}

/*
==================
jkVehicle::Hide
==================
*/
void jkVehicle::Hide ( void ) {
	idActor::Hide ( );

	GetPhysics()->SetContents( 0 );
	GetPhysics()->GetClipModel()->Unlink();
}

/*
==================
jkVehicle::Show
==================
*/
void jkVehicle::Show ( void ) {
	idActor::Show ( );
	
	GetPhysics()->SetContents ( CONTENTS_BODY );
	//GetPhysics()->GetClipModel()->Link();
}

/*
==================
jkVehicle::Lock
==================
*/
void jkVehicle::Lock( void ) {
	Event_Lock( true );
}

/*
==================
jkVehicle::Unlock
==================
*/
void jkVehicle::Unlock( void ) {
	Event_Lock( false );
}

/*
==================
jkVehicle::GuidedProjectileLocked
==================
*/
void jkVehicle::GuidedProjectileIncoming( idGuidedProjectile * projectile ) {
	if ( projectile ) {
		//incomingProjectiles.Insert( projectile );
	}
}

/*
==================
jkVehicle::UpdateIncomingProjectiles
==================
*/
void jkVehicle::UpdateIncomingProjectiles( void ) {
    return;
    /*
	idGuidedProjectile * proj = NULL;
	float dist = 0.0f;

	for( int i = incomingProjectiles.Num() - 1; i >= 0; i-- ) {
		if ( !incomingProjectiles[ i ].IsValid() ) {
			incomingProjectiles.RemoveIndex( i );
			continue;
		}

		if ( proj )  {
			float d = ( incomingProjectiles[ i ]->GetPhysics()->GetOrigin() - this->GetPhysics()->GetOrigin() ).LengthSqr();

			if ( dist > d ) {
				proj = incomingProjectiles[ i ];
				dist = d;
			}
		} else {
			proj = incomingProjectiles[ i ];
			dist = ( incomingProjectiles[ i ]->GetPhysics()->GetOrigin() - this->GetPhysics()->GetOrigin() ).LengthSqr();
		}
	}

	if ( proj ) {
		idVec3 localDir;
		int length;

		GetPosition( 0 )->mEyeAxis.ProjectVector( proj->GetPhysics()->GetOrigin() - GetOrigin(), localDir );
		GetHud()->SetStateFloat ( "missiledir", localDir.ToAngles()[YAW] );

		//idSoundEmitter *emitter = soundSystem->EmitterForIndex( SOUNDWORLD_GAME, refSound.referenceSoundHandle );

		if ( !vfl.missileWarningOn ) {
			if ( GetHud() ) {
				GetHud()->HandleNamedEvent( "missileThreatUp" );
			}

			if ( emitter ) {
				StartSound( "snd_incomingProjectile", SND_CHANNEL_BODY3, 0, false, &length );
			}

			vfl.missileWarningOn = true;
		}

		if ( emitter ) {
			//float lerp						= 1.0f - idMath::ClampFloat( 0.0f, 1.0f, ( dist / 100000.0f ) );
			//refSound.parms.volume			= lerp * 0.6f + 0.5f;
			//refSound.parms.frequencyShift	= lerp * 0.4f + 0.8f;
			//emitter->ModifySound ( SND_CHANNEL_ANY, &refSound.parms );
		}
	} else if ( vfl.missileWarningOn ) {
		if ( GetHud() ) {
			GetHud()->HandleNamedEvent( "missileThreatDown" );
		}

		StopSound( SND_CHANNEL_BODY3, false );

		vfl.missileWarningOn = false;
	}
        */
}

/*
==================
jkVehicle::Event_Lock
==================
*/
void jkVehicle::Event_Lock ( bool lock ) {
	vfl.locked = lock;
}

/*
==================
jkVehicle::Event_IsLocked
==================
*/
void jkVehicle::Event_IsLocked ( void ) {	
	idThread::ReturnFloat( (float)IsLocked() );
}

/*
==================
jkVehicle::Event_EnableWeapon
==================
*/
void jkVehicle::Event_EnableWeapon ( void ) {	
	vfl.disableWeapons = false;
}

/*
==================
jkVehicle::Event_DisableWeapon
==================
*/
void jkVehicle::Event_DisableWeapon ( void ) {	
	vfl.disableWeapons = true;
}


/*
==================
jkVehicle::Event_EnableMovement
==================
*/
void jkVehicle::Event_EnableMovement( void ) {	
	vfl.disableMovement = false;
}

/*
==================
jkVehicle::Event_DisableMovement
==================
*/
void jkVehicle::Event_DisableMovement ( void ) {	
	vfl.disableMovement = true;
}

/*
==================
jkVehicle::Event_EnableClip
==================
*/
void jkVehicle::Event_EnableClip( void ) {
	GetPhysics()->SetContents( cachedContents );
}

/*
==================
jkVehicle::Event_DisableClip
==================
*/
void jkVehicle::Event_DisableClip( void ) {
	cachedContents = GetPhysics()->GetContents();
	GetPhysics()->SetContents( 0 );
}

/*
==================
jkVehicle::Event_Activate
==================
*/
void jkVehicle::Event_Activate( idEntity* activator ) {
	RemoveNullTargets();

	if( !targets.Num() && activator && activator->IsType(idPlayer::Type) ) {
		//static_cast<idPlayer*>( activator )->EnterVehicle( this ); FIXME1
	}
}

/*
==================
jkVehicle::Event_LaunchProjectiles
==================
*/
void jkVehicle::Event_LaunchProjectiles( const idList<idStr>* parms ) {
	int pos = -1;

	assert( parms && parms->Num() );

	sscanf( (*parms)[0].c_str(), "%d", &pos );
	GetPosition( pos )->FireWeapon();
}

/*
==================
jkVehicle::Event_SetScript
==================
*/
void jkVehicle::Event_SetScript( const char* scriptName, const char* funcName ) {
	if ( !funcName || !funcName[0] ) {
		return;
	} 

	// Set the associated script
	if ( !idStr::Icmp ( scriptName, "enter" ) ) {
		//funcs.enter.Init( funcName );
	} else if ( !idStr::Icmp ( scriptName, "exit" ) ) {
		//funcs.exit.Init( funcName );
	} else {
		gameLocal.Warning ( "unknown script '%s' specified on vehicle '%s'", scriptName, name.c_str() );
	}
}

/*
================
jkVehicle::Event_SetHealth
================
*/
void jkVehicle::Event_SetHealth	( float health ) {
	this->health = health;
}

/*
================
jkVehicle::Event_HUDShockWarningOff
================
*/
void jkVehicle::Event_HUDShockWarningOff ( ) {
	if ( GetHud() ) {
		GetHud()->HandleNamedEvent( "electricWarningOff" );
	}
}

/*
================
jkVehicle::Event_StalledRestart
================
*/
void jkVehicle::Event_StalledRestart ( float shield, float damage ) {
	vfl.stalled = false;

	if ( renderEntity.gui[ 0 ] ) {
		renderEntity.gui[ 0 ]->HandleNamedEvent( "shock_stall_restart" );
	}

	if ( renderEntity.gui[ 1 ] ) {
		renderEntity.gui[ 1 ]->HandleNamedEvent( "shock_stall_restart" );
	}

	// Looping warning sound for shield
	if ( shield <= 0 ) {
		StopSound ( SND_CHANNEL_BODY2, false );
		StartSound ( "snd_shieldWarning", SND_CHANNEL_BODY2, 0, false, NULL );
	}

	// Play low health warning on transition to low health value
	if ( health >= healthLow && health - damage < healthLow ) {
		StartSound ( "snd_voiceLowHealth", SND_CHANNEL_VOICE, 0, false, NULL );
	}
}

/*
================
jkVehicle::Event_GetViewAngles
================
*/
void jkVehicle::Event_GetViewAngles ( ) {

	idThread::ReturnVector( GetPosition( 0)->GetEyeAxis()[0]);
}

/*
===============================================================================

	States 

===============================================================================
*/
/*
CLASS_STATES_DECLARATION ( jkVehicle )
	STATE ( "Wait_Driver",					jkVehicle::State_Wait_Driver )
END_CLASS_STATES
*/
/*
================
jkVehicle::State_Wait_Driver
================
*/
/*
stateResult_t jkVehicle::State_Wait_Driver ( int blendFrames ) {
	if ( !vfl.driver || vfl.stalled ) {
		return SRESULT_WAIT;
	}
	
	return SRESULT_DONE;
}
*/