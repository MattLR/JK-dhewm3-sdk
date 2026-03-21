/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "sys/platform.h"
#include "framework/DeclEntityDef.h"
#include "framework/DeclSkin.h"

#include "gamesys/SysCvar.h"
#include "ai/AI.h"
#include "Player.h"
#include "Trigger.h"
#include "SmokeParticles.h"
#include "WorldSpawn.h"

#include "Force.h"

/***********************************************************************

  jkSimpleForcePower

***********************************************************************/

//
// event defs
//
const idEventDef EV_Weapon_Clear( "<clear>" );
const idEventDef EV_Weapon_GetOwner( "getOwner", NULL, 'e' );
const idEventDef EV_Weapon_Next( "nextWeapon" );
const idEventDef EV_Weapon_State( "weaponState", "sd" );
const idEventDef EV_Weapon_UseAmmo( "useAmmo", "d" );
const idEventDef EV_Weapon_AddToClip( "addToClip", "d" );
const idEventDef EV_Weapon_AmmoInClip( "ammoInClip", NULL, 'f' );
const idEventDef EV_Weapon_AmmoAvailable( "ammoAvailable", NULL, 'f' );
const idEventDef EV_Weapon_TotalAmmoCount( "totalAmmoCount", NULL, 'f' );
const idEventDef EV_Weapon_ClipSize( "clipSize", NULL, 'f' );
const idEventDef EV_Weapon_WeaponOutOfAmmo( "weaponOutOfAmmo" );
const idEventDef EV_Weapon_WeaponReady( "weaponReady" );
const idEventDef EV_Weapon_WeaponReloading( "weaponReloading" );
const idEventDef EV_Weapon_WeaponHolstered( "weaponHolstered" );
const idEventDef EV_Weapon_WeaponRising( "weaponRising" );
const idEventDef EV_Weapon_WeaponLowering( "weaponLowering" );
const idEventDef EV_Weapon_Flashlight( "flashlight", "d" );
const idEventDef EV_Weapon_LaunchProjectiles( "launchProjectiles", "dffff" );
const idEventDef EV_Weapon_LaunchProjectiles_Alt( "launchProjectilesAlt", "dffff" );
const idEventDef EV_Weapon_CreateProjectile( "createProjectile", NULL, 'e' );
const idEventDef EV_Weapon_EjectBrass( "ejectBrass" );
const idEventDef EV_Weapon_GetWorldModel( "getWorldModel", NULL, 'e' );
const idEventDef EV_Weapon_AllowDrop( "allowDrop", "d" );
const idEventDef EV_Weapon_AutoReload( "autoReload", NULL, 'f' );
const idEventDef EV_Weapon_NetReload( "netReload" );
const idEventDef EV_Weapon_IsInvisible( "isInvisible", NULL, 'f' );
const idEventDef EV_Weapon_NetEndReload( "netEndReload" );
const idEventDef EV_Force_DoForcePower( "doForcePower" );
const idEventDef EV_Force_UseForcePoints( "useForcePoints", "fdd" );


//
// class def
//
CLASS_DECLARATION( idAnimatedEntity, jkSimpleForcePower )
	EVENT( EV_Weapon_Clear,						jkSimpleForcePower::Event_Clear )
	EVENT( EV_Weapon_GetOwner,					jkSimpleForcePower::Event_GetOwner )
	EVENT( EV_Weapon_State,						jkSimpleForcePower::Event_WeaponState )
	EVENT( EV_Weapon_WeaponReady,				jkSimpleForcePower::Event_WeaponReady )
	EVENT( EV_Weapon_WeaponOutOfAmmo,			jkSimpleForcePower::Event_WeaponOutOfAmmo )
	EVENT( EV_Weapon_WeaponReloading,			jkSimpleForcePower::Event_WeaponReloading )
	EVENT( EV_Weapon_WeaponHolstered,			jkSimpleForcePower::Event_WeaponHolstered )
	EVENT( EV_Weapon_WeaponRising,				jkSimpleForcePower::Event_WeaponRising )
	EVENT( EV_Weapon_WeaponLowering,			jkSimpleForcePower::Event_WeaponLowering )
	EVENT( EV_Weapon_UseAmmo,					jkSimpleForcePower::Event_UseAmmo )
	EVENT( EV_Weapon_AddToClip,					jkSimpleForcePower::Event_AddToClip )
	EVENT( EV_Weapon_AmmoInClip,				jkSimpleForcePower::Event_AmmoInClip )
	EVENT( EV_Weapon_AmmoAvailable,				jkSimpleForcePower::Event_AmmoAvailable )
	EVENT( EV_Weapon_TotalAmmoCount,			jkSimpleForcePower::Event_TotalAmmoCount )
	EVENT( EV_Weapon_ClipSize,					jkSimpleForcePower::Event_ClipSize )
	EVENT( AI_PlayAnim,							jkSimpleForcePower::Event_PlayAnim )
	EVENT( AI_PlayCycle,						jkSimpleForcePower::Event_PlayCycle )
	EVENT( AI_SetBlendFrames,					jkSimpleForcePower::Event_SetBlendFrames )
	EVENT( AI_GetBlendFrames,					jkSimpleForcePower::Event_GetBlendFrames )
	EVENT( AI_AnimDone,							jkSimpleForcePower::Event_AnimDone )
	EVENT( EV_Weapon_Next,						jkSimpleForcePower::Event_Next )
	EVENT( EV_SetSkin,							jkSimpleForcePower::Event_SetSkin )
	EVENT( EV_Weapon_Flashlight,				jkSimpleForcePower::Event_Flashlight )
	EVENT( EV_Light_GetLightParm,				jkSimpleForcePower::Event_GetLightParm )
	EVENT( EV_Light_SetLightParm,				jkSimpleForcePower::Event_SetLightParm )
	EVENT( EV_Light_SetLightParms,				jkSimpleForcePower::Event_SetLightParms )
	EVENT( EV_Weapon_LaunchProjectiles,			jkSimpleForcePower::Event_LaunchProjectiles )
	EVENT( EV_Weapon_LaunchProjectiles_Alt,		jkSimpleForcePower::Event_LaunchProjectiles_Alt )
	EVENT( EV_Weapon_CreateProjectile,			jkSimpleForcePower::Event_CreateProjectile )
	EVENT( EV_Weapon_EjectBrass,				jkSimpleForcePower::Event_EjectBrass )
	EVENT( EV_Weapon_GetWorldModel,				jkSimpleForcePower::Event_GetWorldModel )
	EVENT( EV_Weapon_AllowDrop,					jkSimpleForcePower::Event_AllowDrop )
	EVENT( EV_Weapon_AutoReload,				jkSimpleForcePower::Event_AutoReload )
	EVENT( EV_Weapon_NetReload,					jkSimpleForcePower::Event_NetReload )
	EVENT( EV_Weapon_IsInvisible,				jkSimpleForcePower::Event_IsInvisible )
	EVENT( EV_Weapon_NetEndReload,				jkSimpleForcePower::Event_NetEndReload )
	EVENT( EV_Force_DoForcePower,				jkSimpleForcePower::Event_DoForcePower )
	EVENT( EV_Force_UseForcePoints,				jkSimpleForcePower::Event_UseForcePoints )
END_CLASS

CLASS_DECLARATION( jkSimpleForcePower, jkForcePower )

END_CLASS

/***********************************************************************

	init

***********************************************************************/

/*
================
jkSimpleForcePower::jkSimpleForcePower()
================
*/
jkSimpleForcePower::jkSimpleForcePower() {
	owner					= NULL;
	worldModel				= NULL;
	weaponDef				= NULL;
	thread					= NULL;

	memset( &guiLight, 0, sizeof( guiLight ) );
	memset( &muzzleFlash, 0, sizeof( muzzleFlash ) );
	memset( &worldMuzzleFlash, 0, sizeof( worldMuzzleFlash ) );
	memset( &nozzleGlow, 0, sizeof( nozzleGlow ) );

	muzzleFlashEnd			= 0;
	flashColor				= vec3_origin;
	muzzleFlashHandle		= -1;
	worldMuzzleFlashHandle	= -1;
	guiLightHandle			= -1;
	nozzleGlowHandle		= -1;
	modelDefHandle			= -1;

	berserk					= 2;
	brassDelay				= 0;

	allowDrop				= true;

	Clear();

	fl.networkSync = true;
}

/*
================
jkSimpleForcePower::~jkSimpleForcePower()
================
*/
jkSimpleForcePower::~jkSimpleForcePower() {
	Clear();
	delete worldModel.GetEntity();
}


/*
================
jkSimpleForcePower::Spawn
================
*/
void jkSimpleForcePower::Spawn( void ) {
	if ( !gameLocal.isClient ) {
		// setup the world model
		worldModel = static_cast< idAnimatedEntity * >( gameLocal.SpawnEntityType( idAnimatedEntity::Type, NULL ) );
		worldModel.GetEntity()->fl.networkSync = true;
	}

	thread = new idThread();
	thread->ManualDelete();
	thread->ManualControl();
}

/*
================
jkSimpleForcePower::SetOwner

Only called at player spawn time, not each weapon switch
================
*/
void jkSimpleForcePower::SetOwner( idPlayer *_owner ) {
	assert( !owner );
	owner = _owner;
	SetName( va( "%s_fp", owner->name.c_str() ) );

	if ( worldModel.GetEntity() ) {
		worldModel.GetEntity()->SetName( va( "%s_fp_worldmodel", owner->name.c_str() ) );
	}
}

/*
================
jkSimpleForcePower::ShouldConstructScriptObjectAtSpawn

Called during idEntity::Spawn to see if it should construct the script object or not.
Overridden by subclasses that need to spawn the script object themselves.
================
*/
bool jkSimpleForcePower::ShouldConstructScriptObjectAtSpawn( void ) const {
	return false;
}

/*
================
jkSimpleForcePower::CacheWeapon
================
*/
void jkSimpleForcePower::CacheWeapon( const char *weaponName ) {
	const idDeclEntityDef *weaponDef;
	const char *brassDefName;
	const char *clipModelName;
	idTraceModel trm;
	const char *guiName;

	weaponDef = gameLocal.FindEntityDef( weaponName, false );
	if ( !weaponDef ) {
		return;
	}

	// precache the brass collision model
	brassDefName = weaponDef->dict.GetString( "def_ejectBrass" );
	if ( brassDefName[0] ) {
		const idDeclEntityDef *brassDef = gameLocal.FindEntityDef( brassDefName, false );
		if ( brassDef ) {
			brassDef->dict.GetString( "clipmodel", "", &clipModelName );
			if ( !clipModelName[0] ) {
				clipModelName = brassDef->dict.GetString( "model" );		// use the visual model
			}
			// load the trace model
			collisionModelManager->TrmFromModel( clipModelName, trm );
		}
	}

	guiName = weaponDef->dict.GetString( "gui" );
	if ( guiName[0] ) {
		uiManager->FindGui( guiName, true, false, true );
	}
}

/*
================
jkSimpleForcePower::Save
================
*/
void jkSimpleForcePower::Save( idSaveGame *savefile ) const {

	savefile->WriteInt( status );
	savefile->WriteObject( thread );
	savefile->WriteString( state );
	savefile->WriteString( idealState );
	savefile->WriteInt( animBlendFrames );
	savefile->WriteInt( animDoneTime );
	savefile->WriteBool( isLinked );

	savefile->WriteObject( owner );
	worldModel.Save( savefile );

	savefile->WriteInt( hideTime );
	savefile->WriteFloat( hideDistance );
	savefile->WriteInt( hideStartTime );
	savefile->WriteFloat( hideStart );
	savefile->WriteFloat( hideEnd );
	savefile->WriteFloat( hideOffset );
	savefile->WriteBool( hide );
	savefile->WriteBool( disabled );

	savefile->WriteInt( berserk );

	savefile->WriteVec3( playerViewOrigin );
	savefile->WriteMat3( playerViewAxis );

	savefile->WriteVec3( viewWeaponOrigin );
	savefile->WriteMat3( viewWeaponAxis );

	savefile->WriteVec3( muzzleOrigin );
	savefile->WriteMat3( muzzleAxis );

	savefile->WriteVec3( pushVelocity );

	savefile->WriteString( weaponDef->GetName() );
	savefile->WriteInt( brassDelay );
	savefile->WriteString( icon );

	savefile->WriteInt( guiLightHandle );
	savefile->WriteRenderLight( guiLight );

	savefile->WriteInt( muzzleFlashHandle );
	savefile->WriteRenderLight( muzzleFlash );

	savefile->WriteInt( worldMuzzleFlashHandle );
	savefile->WriteRenderLight( worldMuzzleFlash );

	savefile->WriteVec3( flashColor );
	savefile->WriteInt( muzzleFlashEnd );
	savefile->WriteInt( flashTime );

	savefile->WriteBool( lightOn );
	savefile->WriteBool( silent_fire );

	savefile->WriteInt( kick_endtime );
	savefile->WriteInt( muzzle_kick_time );
	savefile->WriteInt( muzzle_kick_maxtime );
	savefile->WriteAngles( muzzle_kick_angles );
	savefile->WriteVec3( muzzle_kick_offset );

	savefile->WriteInt( ammoType );
	savefile->WriteInt( ammoRequired );
	savefile->WriteInt( clipSize );
	savefile->WriteInt( ammoClip );
	savefile->WriteInt( lowAmmo );
	savefile->WriteBool( powerAmmo );

	// savegames <= 17
	savefile->WriteInt( 0 );

	savefile->WriteInt( zoomFov );

	savefile->WriteJoint( barrelJointView );
	savefile->WriteJoint( flashJointView );
	savefile->WriteJoint( ejectJointView );
	savefile->WriteJoint( guiLightJointView );
	savefile->WriteJoint( ventLightJointView );

	savefile->WriteJoint( flashJointWorld );
	savefile->WriteJoint( barrelJointWorld );
	savefile->WriteJoint( ejectJointWorld );

	savefile->WriteBool( hasBloodSplat );

	savefile->WriteSoundShader( sndHum );

	savefile->WriteParticle( weaponSmoke );
	savefile->WriteInt( weaponSmokeStartTime );
	savefile->WriteBool( continuousSmoke );
	savefile->WriteParticle( strikeSmoke );
	savefile->WriteInt( strikeSmokeStartTime );
	savefile->WriteVec3( strikePos );
	savefile->WriteMat3( strikeAxis );
	savefile->WriteInt( nextStrikeFx );

	savefile->WriteBool( nozzleFx );
	savefile->WriteInt( nozzleFxFade );

	savefile->WriteInt( lastAttack );

	savefile->WriteInt( nozzleGlowHandle );
	savefile->WriteRenderLight( nozzleGlow );

	savefile->WriteVec3( nozzleGlowColor );
	savefile->WriteMaterial( nozzleGlowShader );
	savefile->WriteFloat( nozzleGlowRadius );

	savefile->WriteInt( weaponAngleOffsetAverages );
	savefile->WriteFloat( weaponAngleOffsetScale );
	savefile->WriteFloat( weaponAngleOffsetMax );
	savefile->WriteFloat( weaponOffsetTime );
	savefile->WriteFloat( weaponOffsetScale );

	savefile->WriteBool( allowDrop );
	savefile->WriteObject( projectileEnt );

}

/*
================
jkSimpleForcePower::Restore
================
*/
void jkSimpleForcePower::Restore( idRestoreGame *savefile ) {

	savefile->ReadInt( (int &)status );
	savefile->ReadObject( reinterpret_cast<idClass *&>( thread ) );
	savefile->ReadString( state );
	savefile->ReadString( idealState );
	savefile->ReadInt( animBlendFrames );
	savefile->ReadInt( animDoneTime );
	savefile->ReadBool( isLinked );

	// Re-link script fields
	WEAPON_ATTACK.LinkTo(		scriptObject, "WEAPON_ATTACK" );
	WEAPON_ATTACK_ALT.LinkTo(	scriptObject, "WEAPON_ATTACK_ALT" );
	WEAPON_RELOAD.LinkTo(		scriptObject, "WEAPON_RELOAD" );
	WEAPON_NETRELOAD.LinkTo(	scriptObject, "WEAPON_NETRELOAD" );
	WEAPON_NETENDRELOAD.LinkTo(	scriptObject, "WEAPON_NETENDRELOAD" );
	WEAPON_NETFIRING.LinkTo(	scriptObject, "WEAPON_NETFIRING" );
	WEAPON_RAISEWEAPON.LinkTo(	scriptObject, "WEAPON_RAISEWEAPON" );
	WEAPON_LOWERWEAPON.LinkTo(	scriptObject, "WEAPON_LOWERWEAPON" );

	savefile->ReadObject( reinterpret_cast<idClass *&>( owner ) );
	worldModel.Restore( savefile );

	savefile->ReadInt( hideTime );
	savefile->ReadFloat( hideDistance );
	savefile->ReadInt( hideStartTime );
	savefile->ReadFloat( hideStart );
	savefile->ReadFloat( hideEnd );
	savefile->ReadFloat( hideOffset );
	savefile->ReadBool( hide );
	savefile->ReadBool( disabled );

	savefile->ReadInt( berserk );

	savefile->ReadVec3( playerViewOrigin );
	savefile->ReadMat3( playerViewAxis );

	savefile->ReadVec3( viewWeaponOrigin );
	savefile->ReadMat3( viewWeaponAxis );

	savefile->ReadVec3( muzzleOrigin );
	savefile->ReadMat3( muzzleAxis );

	savefile->ReadVec3( pushVelocity );

	idStr objectname;
	savefile->ReadString( objectname );
	weaponDef = gameLocal.FindEntityDef( objectname );

	const idDeclEntityDef *projectileDef = gameLocal.FindEntityDef( weaponDef->dict.GetString( "def_projectile" ), false );
	if ( projectileDef ) {
		projectileDict = projectileDef->dict;
	} else {
		projectileDict.Clear();
	}

	const idDeclEntityDef *brassDef = gameLocal.FindEntityDef( weaponDef->dict.GetString( "def_ejectBrass" ), false );
	if ( brassDef ) {
		brassDict = brassDef->dict;
	} else {
		brassDict.Clear();
	}

	savefile->ReadInt( brassDelay );
	savefile->ReadString( icon );

	savefile->ReadInt( guiLightHandle );
	savefile->ReadRenderLight( guiLight );
	// DG: we need to get a fresh handle, otherwise this will be tied to a completely unrelated light!
	if ( guiLightHandle != -1 ) {
		guiLightHandle = gameRenderWorld->AddLightDef( &guiLight );
	}
	savefile->ReadInt( muzzleFlashHandle );
	savefile->ReadRenderLight( muzzleFlash );
	if ( muzzleFlashHandle != -1 ) { // DG: enforce getting fresh handle
		muzzleFlashHandle = gameRenderWorld->AddLightDef( &muzzleFlash );
	}

	savefile->ReadInt( worldMuzzleFlashHandle );
	savefile->ReadRenderLight( worldMuzzleFlash );
	if ( worldMuzzleFlashHandle != -1 ) { // DG: enforce getting fresh handle
		worldMuzzleFlashHandle = gameRenderWorld->AddLightDef( &worldMuzzleFlash );
	}

	savefile->ReadVec3( flashColor );
	savefile->ReadInt( muzzleFlashEnd );
	savefile->ReadInt( flashTime );

	savefile->ReadBool( lightOn );
	savefile->ReadBool( silent_fire );

	savefile->ReadInt( kick_endtime );
	savefile->ReadInt( muzzle_kick_time );
	savefile->ReadInt( muzzle_kick_maxtime );
	savefile->ReadAngles( muzzle_kick_angles );
	savefile->ReadVec3( muzzle_kick_offset );

	savefile->ReadInt( (int &)ammoType );
	savefile->ReadInt( ammoRequired );
	savefile->ReadInt( clipSize );
	savefile->ReadInt( ammoClip );
	savefile->ReadInt( lowAmmo );
	savefile->ReadBool( powerAmmo );

	// savegame versions <= 17
	int foo;
	savefile->ReadInt( foo );

	savefile->ReadInt( zoomFov );

	savefile->ReadJoint( barrelJointView );
	savefile->ReadJoint( flashJointView );
	savefile->ReadJoint( ejectJointView );
	savefile->ReadJoint( guiLightJointView );
	savefile->ReadJoint( ventLightJointView );

	savefile->ReadJoint( flashJointWorld );
	savefile->ReadJoint( barrelJointWorld );
	savefile->ReadJoint( ejectJointWorld );

	savefile->ReadBool( hasBloodSplat );

	savefile->ReadSoundShader( sndHum );

	savefile->ReadParticle( weaponSmoke );
	savefile->ReadInt( weaponSmokeStartTime );
	savefile->ReadBool( continuousSmoke );
	savefile->ReadParticle( strikeSmoke );
	savefile->ReadInt( strikeSmokeStartTime );
	savefile->ReadVec3( strikePos );
	savefile->ReadMat3( strikeAxis );
	savefile->ReadInt( nextStrikeFx );

	savefile->ReadBool( nozzleFx );
	savefile->ReadInt( nozzleFxFade );

	savefile->ReadInt( lastAttack );

	savefile->ReadInt( nozzleGlowHandle );
	savefile->ReadRenderLight( nozzleGlow );
	if ( nozzleGlowHandle != -1 ) { // DG: enforce getting fresh handle
		nozzleGlowHandle = gameRenderWorld->AddLightDef( &nozzleGlow );
	}

	savefile->ReadVec3( nozzleGlowColor );
	savefile->ReadMaterial( nozzleGlowShader );
	savefile->ReadFloat( nozzleGlowRadius );

	savefile->ReadInt( weaponAngleOffsetAverages );
	savefile->ReadFloat( weaponAngleOffsetScale );
	savefile->ReadFloat( weaponAngleOffsetMax );
	savefile->ReadFloat( weaponOffsetTime );
	savefile->ReadFloat( weaponOffsetScale );

	savefile->ReadBool( allowDrop );
	savefile->ReadObject( reinterpret_cast<idClass *&>( projectileEnt ) );
}

/***********************************************************************

	Weapon definition management

***********************************************************************/

/*
================
jkSimpleForcePower::Clear
================
*/
void jkSimpleForcePower::Clear( void ) {
	CancelEvents( &EV_Weapon_Clear );

	DeconstructScriptObject();
	scriptObject.Free();

	WEAPON_ATTACK.Unlink();
	WEAPON_ATTACK_ALT.Unlink();
	WEAPON_RELOAD.Unlink();
	WEAPON_NETRELOAD.Unlink();
	WEAPON_NETENDRELOAD.Unlink();
	WEAPON_NETFIRING.Unlink();
	WEAPON_RAISEWEAPON.Unlink();
	WEAPON_LOWERWEAPON.Unlink();

	if ( muzzleFlashHandle != -1 ) {
		gameRenderWorld->FreeLightDef( muzzleFlashHandle );
		muzzleFlashHandle = -1;
	}
	if ( muzzleFlashHandle != -1 ) {
		gameRenderWorld->FreeLightDef( muzzleFlashHandle );
		muzzleFlashHandle = -1;
	}
	if ( worldMuzzleFlashHandle != -1 ) {
		gameRenderWorld->FreeLightDef( worldMuzzleFlashHandle );
		worldMuzzleFlashHandle = -1;
	}
	if ( guiLightHandle != -1 ) {
		gameRenderWorld->FreeLightDef( guiLightHandle );
		guiLightHandle = -1;
	}
	if ( nozzleGlowHandle != -1 ) {
		gameRenderWorld->FreeLightDef( nozzleGlowHandle );
		nozzleGlowHandle = -1;
	}

	memset( &renderEntity, 0, sizeof( renderEntity ) );
	renderEntity.entityNum	= entityNumber;

	renderEntity.noShadow		= true;
	renderEntity.noSelfShadow	= true;
	renderEntity.customSkin		= NULL;

	// set default shader parms
	renderEntity.shaderParms[ SHADERPARM_RED ]	= 1.0f;
	renderEntity.shaderParms[ SHADERPARM_GREEN ]= 1.0f;
	renderEntity.shaderParms[ SHADERPARM_BLUE ]	= 1.0f;
	renderEntity.shaderParms[3] = 1.0f;
	renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = 0.0f;
	renderEntity.shaderParms[5] = 0.0f;
	renderEntity.shaderParms[6] = 0.0f;
	renderEntity.shaderParms[7] = 0.0f;

	if ( refSound.referenceSound ) {
		refSound.referenceSound->Free( true );
	}
	memset( &refSound, 0, sizeof( refSound_t ) );

	// setting diversity to 0 results in no random sound.  -1 indicates random.
	refSound.diversity = -1.0f;

	if ( owner ) {
		// don't spatialize the weapon sounds
		refSound.listenerId = owner->GetListenerId();
	}

	// clear out the sounds from our spawnargs since we'll copy them from the weapon def
	const idKeyValue *kv = spawnArgs.MatchPrefix( "snd_" );
	while( kv ) {
		spawnArgs.Delete( kv->GetKey() );
		kv = spawnArgs.MatchPrefix( "snd_" );
	}

	hideTime		= 300;
	hideDistance	= -15.0f;
	hideStartTime	= gameLocal.time - hideTime;
	hideStart		= 0.0f;
	hideEnd			= 0.0f;
	hideOffset		= 0.0f;
	hide			= false;
	disabled		= false;

	weaponSmoke		= NULL;
	weaponSmokeStartTime = 0;
	continuousSmoke = false;
	strikeSmoke		= NULL;
	strikeSmokeStartTime = 0;
	strikePos.Zero();
	strikeAxis = mat3_identity;
	nextStrikeFx = 0;

	icon			= "";

	playerViewAxis.Identity();
	playerViewOrigin.Zero();
	viewWeaponAxis.Identity();
	viewWeaponOrigin.Zero();
	muzzleAxis.Identity();
	muzzleOrigin.Zero();
	pushVelocity.Zero();

	status			= FP_HOLSTERED;
	state			= "";
	idealState		= "";
	animBlendFrames	= 0;
	animDoneTime	= 0;

	projectileDict.Clear();
	brassDict.Clear();

	flashTime		= 250;
	lightOn			= false;
	silent_fire		= false;

	ammoType		= 0;
	ammoRequired	= 0;
	ammoClip		= 0;
	clipSize		= 0;
	lowAmmo			= 0;
	powerAmmo		= false;

	kick_endtime		= 0;
	muzzle_kick_time	= 0;
	muzzle_kick_maxtime	= 0;
	muzzle_kick_angles.Zero();
	muzzle_kick_offset.Zero();

	zoomFov = 90;

	barrelJointView		= INVALID_JOINT;
	flashJointView		= INVALID_JOINT;
	ejectJointView		= INVALID_JOINT;
	guiLightJointView	= INVALID_JOINT;
	ventLightJointView	= INVALID_JOINT;

	barrelJointWorld	= INVALID_JOINT;
	flashJointWorld		= INVALID_JOINT;
	ejectJointWorld		= INVALID_JOINT;

	hasBloodSplat		= false;
	nozzleFx			= false;
	nozzleFxFade		= 1500;
	lastAttack			= 0;
	nozzleGlowHandle	= -1;
	nozzleGlowShader	= NULL;
	nozzleGlowRadius	= 10;
	nozzleGlowColor.Zero();

	weaponAngleOffsetAverages	= 0;
	weaponAngleOffsetScale		= 0.0f;
	weaponAngleOffsetMax		= 0.0f;
	weaponOffsetTime			= 0.0f;
	weaponOffsetScale			= 0.0f;

	allowDrop			= true;

	animator.ClearAllAnims( gameLocal.time, 0 );
	FreeModelDef();

	sndHum				= NULL;

	isLinked			= false;
	projectileEnt		= NULL;

	isFiring			= false;
}

/*
================
jkSimpleForcePower::InitWorldModel
================
*/
void jkSimpleForcePower::InitWorldModel( const idDeclEntityDef *def ) {
	idEntity *ent;

	ent = worldModel.GetEntity();

	assert( ent );
	assert( def );

	const char *model = def->dict.GetString( "model_world" );
	const char *attach = def->dict.GetString( "joint_attach" );

	ent->SetSkin( NULL );
	if ( model[0] && attach[0] ) {
		ent->Show();
		ent->SetModel( model );
		if ( ent->GetAnimator()->ModelDef() ) {
			ent->SetSkin( ent->GetAnimator()->ModelDef()->GetDefaultSkin() );
		}
		ent->GetPhysics()->SetContents( 0 );
		ent->GetPhysics()->SetClipModel( NULL, 1.0f );
		ent->BindToJoint( owner, attach, true );
		ent->GetPhysics()->SetOrigin( vec3_origin );
		ent->GetPhysics()->SetAxis( mat3_identity );

		// supress model in player views, but allow it in mirrors and remote views
		renderEntity_t *worldModelRenderEntity = ent->GetRenderEntity();
		if ( worldModelRenderEntity ) {
			worldModelRenderEntity->suppressSurfaceInViewID = owner->entityNumber+1;
			worldModelRenderEntity->suppressShadowInViewID = owner->entityNumber+1;
			worldModelRenderEntity->suppressShadowInLightID = LIGHTID_VIEW_MUZZLE_FLASH + owner->entityNumber;
		}
	} else {
		ent->SetModel( "" );
		ent->Hide();
	}

	flashJointWorld = ent->GetAnimator()->GetJointHandle( "flash" );
	barrelJointWorld = ent->GetAnimator()->GetJointHandle( "muzzle" );
	ejectJointWorld = ent->GetAnimator()->GetJointHandle( "eject" );
}

/*
================
jkSimpleForcePower::GetWeaponDef
================
*/
void jkSimpleForcePower::GetWeaponDef( const char *objectname, int ammoinclip ) {
	const char *shader;
	const char *objectType;
	const char *forceType;
	const char *vmodel;
	const char *guiName;
	const char *projectileName;
	const char *projectileNameAlt;
	const char *brassDefName;
	const char *smokeName;
	int			ammoAvail;

	Clear();

	if ( !objectname || !objectname[ 0 ] ) {
		return;
	}

	assert( owner );

	weaponDef			= gameLocal.FindEntityDef( objectname );

	ammoType			= GetAmmoNumForName( weaponDef->dict.GetString( "ammoType" ) );
	ammoRequired		= weaponDef->dict.GetInt( "ammoRequired" );
	clipSize			= weaponDef->dict.GetInt( "clipSize" );
	lowAmmo				= weaponDef->dict.GetInt( "lowAmmo" );

	icon				= weaponDef->dict.GetString( "icon" );
	silent_fire			= weaponDef->dict.GetBool( "silent_fire" );
	powerAmmo			= weaponDef->dict.GetBool( "powerAmmo" );

	muzzle_kick_time	= SEC2MS( weaponDef->dict.GetFloat( "muzzle_kick_time" ) );
	muzzle_kick_maxtime	= SEC2MS( weaponDef->dict.GetFloat( "muzzle_kick_maxtime" ) );
	muzzle_kick_angles	= weaponDef->dict.GetAngles( "muzzle_kick_angles" );
	muzzle_kick_offset	= weaponDef->dict.GetVector( "muzzle_kick_offset" );

	hideTime			= SEC2MS( weaponDef->dict.GetFloat( "hide_time", "0.3" ) );
	hideDistance		= weaponDef->dict.GetFloat( "hide_distance", "-15" );

	// muzzle smoke
	smokeName = weaponDef->dict.GetString( "smoke_muzzle" );
	if ( *smokeName != '\0' ) {
		weaponSmoke = static_cast<const idDeclParticle *>( declManager->FindType( DECL_PARTICLE, smokeName ) );
	} else {
		weaponSmoke = NULL;
	}
	continuousSmoke = weaponDef->dict.GetBool( "continuousSmoke" );
	weaponSmokeStartTime = ( continuousSmoke ) ? gameLocal.time : 0;

	smokeName = weaponDef->dict.GetString( "smoke_strike" );
	if ( *smokeName != '\0' ) {
		strikeSmoke = static_cast<const idDeclParticle *>( declManager->FindType( DECL_PARTICLE, smokeName ) );
	} else {
		strikeSmoke = NULL;
	}
	strikeSmokeStartTime = 0;
	strikePos.Zero();
	strikeAxis = mat3_identity;
	nextStrikeFx = 0;

	// setup gui light
	memset( &guiLight, 0, sizeof( guiLight ) );
	const char *guiLightShader = weaponDef->dict.GetString( "mtr_guiLightShader" );
	if ( *guiLightShader != '\0' ) {
		guiLight.shader = declManager->FindMaterial( guiLightShader, false );
		guiLight.lightRadius[0] = guiLight.lightRadius[1] = guiLight.lightRadius[2] = 3;
		guiLight.pointLight = true;
	}

	// setup the view model
	vmodel = weaponDef->dict.GetString( "model_view" );
	SetModel( vmodel );

	// setup the world model
	InitWorldModel( weaponDef );

	// copy the sounds from the weapon view model def into out spawnargs
	const idKeyValue *kv = weaponDef->dict.MatchPrefix( "snd_" );
	while( kv ) {
		spawnArgs.Set( kv->GetKey(), kv->GetValue() );
		kv = weaponDef->dict.MatchPrefix( "snd_", kv );
	}

	// find some joints in the model for locating effects
	barrelJointView = animator.GetJointHandle( "barrel" );
	flashJointView = animator.GetJointHandle( "flash" );
	ejectJointView = animator.GetJointHandle( "eject" );
	guiLightJointView = animator.GetJointHandle( "guiLight" );
	ventLightJointView = animator.GetJointHandle( "ventLight" );

	// get the projectile
	projectileDict.Clear();

	projectileName = weaponDef->dict.GetString( "def_projectile" );
	if ( projectileName[0] != '\0' ) {
		const idDeclEntityDef *projectileDef = gameLocal.FindEntityDef( projectileName, false );
		if ( !projectileDef ) {
			gameLocal.Warning( "Unknown projectile '%s' in weapon '%s'", projectileName, objectname );
		} else {
			const char *spawnclass = projectileDef->dict.GetString( "spawnclass" );
			idTypeInfo *cls = idClass::GetClass( spawnclass );
			if ( !cls || !cls->IsType( idProjectile::Type ) ) {
				gameLocal.Warning( "Invalid spawnclass '%s' on projectile '%s' (used by weapon '%s')", spawnclass, projectileName, objectname );
			} else {
				projectileDict = projectileDef->dict;
			}
		}
	}
	//Get the alt fire projectile Dynamix
	//Add a thing to check if it has an alt fire for other stuff
	projectileDictAlt.Clear();

	projectileNameAlt = weaponDef->dict.GetString( "def_projectile_alt" );
	if ( projectileNameAlt[0] != '\0' ) {
		const idDeclEntityDef *projectileDefAlt = gameLocal.FindEntityDef( projectileNameAlt, false );
		if ( !projectileDefAlt ) {
			gameLocal.Warning( "Unknown projectile '%s' in weapon '%s'", projectileNameAlt, objectname );
		} else {
			const char *spawnclass = projectileDefAlt->dict.GetString( "spawnclass" );
			idTypeInfo *cls = idClass::GetClass( spawnclass );
			if ( !cls || !cls->IsType( idProjectile::Type ) ) {
				gameLocal.Warning( "Invalid spawnclass '%s' on projectile '%s' (used by weapon '%s')", spawnclass, projectileNameAlt, objectname );
			} else {
				projectileDictAlt = projectileDefAlt->dict;
			}
		}
	}

	// set up muzzleflash render light
	const idMaterial*flashShader;
	idVec3			flashTarget;
	idVec3			flashUp;
	idVec3			flashRight;
	float			flashRadius;
	bool			flashPointLight;

	weaponDef->dict.GetString( "mtr_flashShader", "", &shader );
	flashShader = declManager->FindMaterial( shader, false );
	flashPointLight = weaponDef->dict.GetBool( "flashPointLight", "1" );
	weaponDef->dict.GetVector( "flashColor", "0 0 0", flashColor );
	flashRadius		= (float)weaponDef->dict.GetInt( "flashRadius" );	// if 0, no light will spawn
	flashTime		= SEC2MS( weaponDef->dict.GetFloat( "flashTime", "0.25" ) );
	flashTarget		= weaponDef->dict.GetVector( "flashTarget" );
	flashUp			= weaponDef->dict.GetVector( "flashUp" );
	flashRight		= weaponDef->dict.GetVector( "flashRight" );

	memset( &muzzleFlash, 0, sizeof( muzzleFlash ) );
	muzzleFlash.lightId = LIGHTID_VIEW_MUZZLE_FLASH + owner->entityNumber;
	muzzleFlash.allowLightInViewID = owner->entityNumber+1;

	// the weapon lights will only be in first person
	guiLight.allowLightInViewID = owner->entityNumber+1;
	nozzleGlow.allowLightInViewID = owner->entityNumber+1;

	muzzleFlash.pointLight								= flashPointLight;
	muzzleFlash.shader									= flashShader;
	muzzleFlash.shaderParms[ SHADERPARM_RED ]			= flashColor[0];
	muzzleFlash.shaderParms[ SHADERPARM_GREEN ]			= flashColor[1];
	muzzleFlash.shaderParms[ SHADERPARM_BLUE ]			= flashColor[2];
	muzzleFlash.shaderParms[ SHADERPARM_TIMESCALE ]		= 1.0f;

	muzzleFlash.lightRadius[0]							= flashRadius;
	muzzleFlash.lightRadius[1]							= flashRadius;
	muzzleFlash.lightRadius[2]							= flashRadius;

	if ( !flashPointLight ) {
		muzzleFlash.target								= flashTarget;
		muzzleFlash.up									= flashUp;
		muzzleFlash.right								= flashRight;
		muzzleFlash.end									= flashTarget;
	}

	// the world muzzle flash is the same, just positioned differently
	worldMuzzleFlash = muzzleFlash;
	worldMuzzleFlash.suppressLightInViewID = owner->entityNumber+1;
	worldMuzzleFlash.allowLightInViewID = 0;
	worldMuzzleFlash.lightId = LIGHTID_WORLD_MUZZLE_FLASH + owner->entityNumber;

	//-----------------------------------

	nozzleFx			= weaponDef->dict.GetBool("nozzleFx");
	nozzleFxFade		= weaponDef->dict.GetInt("nozzleFxFade", "1500");
	nozzleGlowColor		= weaponDef->dict.GetVector("nozzleGlowColor", "1 1 1");
	nozzleGlowRadius	= weaponDef->dict.GetFloat("nozzleGlowRadius", "10");
	weaponDef->dict.GetString( "mtr_nozzleGlowShader", "", &shader );
	nozzleGlowShader = declManager->FindMaterial( shader, false );


	// get the brass def
	brassDict.Clear();
	brassDelay = weaponDef->dict.GetInt( "ejectBrassDelay", "0" );
	brassDefName = weaponDef->dict.GetString( "def_ejectBrass" );

	if ( brassDefName[0] ) {
		const idDeclEntityDef *brassDef = gameLocal.FindEntityDef( brassDefName, false );
		if ( !brassDef ) {
			gameLocal.Warning( "Unknown brass '%s'", brassDefName );
		} else {
			brassDict = brassDef->dict;
		}
	}

	if ( ( ammoType < 0 ) || ( ammoType >= AMMO_NUMTYPES ) ) {
		gameLocal.Warning( "Unknown ammotype in object '%s'", objectname );
	}

	ammoClip = ammoinclip;
	if ( ( ammoClip < 0 ) || ( ammoClip > clipSize ) ) {
		// first time using this weapon so have it fully loaded to start
		ammoClip = clipSize;
		ammoAvail = owner->inventory.HasAmmo( ammoType, ammoRequired );
		if ( ammoClip > ammoAvail ) {
			ammoClip = ammoAvail;
		}
	}

	renderEntity.gui[ 0 ] = NULL;
	guiName = weaponDef->dict.GetString( "gui" );
	if ( guiName[0] ) {
		renderEntity.gui[ 0 ] = uiManager->FindGui( guiName, true, false, true );
	}

	zoomFov = weaponDef->dict.GetInt( "zoomFov", "70" );
	berserk = weaponDef->dict.GetInt( "berserk", "2" );

	weaponAngleOffsetAverages = weaponDef->dict.GetInt( "weaponAngleOffsetAverages", "10" );
	weaponAngleOffsetScale = weaponDef->dict.GetFloat( "weaponAngleOffsetScale", "0.25" );
	weaponAngleOffsetMax = weaponDef->dict.GetFloat( "weaponAngleOffsetMax", "10" );

	weaponOffsetTime = weaponDef->dict.GetFloat( "weaponOffsetTime", "400" );
	weaponOffsetScale = weaponDef->dict.GetFloat( "weaponOffsetScale", "0.005" );

	if ( !weaponDef->dict.GetString( "weapon_scriptobject", NULL, &objectType ) ) {
		gameLocal.Error( "2No 'weapon_scriptobject' set on '%s'.", objectname );
	}

	if (!weaponDef->dict.GetString("forceclass", NULL, &forceType)) {
		gameLocal.Error("No 'weaponclass' set on '%s'.", objectname);
	}

	// setup script object
	if ( !scriptObject.SetType( objectType ) ) {
		gameLocal.Error( "Script object '%s' not found on weapon '%s'.", objectType, objectname );
	}

	WEAPON_ATTACK.LinkTo(		scriptObject, "WEAPON_ATTACK" );
	WEAPON_ATTACK_ALT.LinkTo(		scriptObject, "WEAPON_ATTACK_ALT" );
	WEAPON_RELOAD.LinkTo(		scriptObject, "WEAPON_RELOAD" );
	WEAPON_NETRELOAD.LinkTo(	scriptObject, "WEAPON_NETRELOAD" );
	WEAPON_NETENDRELOAD.LinkTo(	scriptObject, "WEAPON_NETENDRELOAD" );
	WEAPON_NETFIRING.LinkTo(	scriptObject, "WEAPON_NETFIRING" );
	WEAPON_RAISEWEAPON.LinkTo(	scriptObject, "WEAPON_RAISEWEAPON" );
	WEAPON_LOWERWEAPON.LinkTo(	scriptObject, "WEAPON_LOWERWEAPON" );

	spawnArgs = weaponDef->dict;

	shader = spawnArgs.GetString( "snd_hum" );
	if ( shader && *shader ) {
		sndHum = declManager->FindSound( shader );
		StartSoundShader( sndHum, SND_CHANNEL_BODY, 0, false, NULL );
	}

	isLinked = true;

	// call script object's constructor
	ConstructScriptObject();

	// make sure we have the correct skin
	UpdateSkin();
}

/***********************************************************************

	GUIs

***********************************************************************/

/*
================
jkSimpleForcePower::Icon
================
*/
const char *jkSimpleForcePower::Icon( void ) const {
	return icon;
}

/*
================
jkSimpleForcePower::UpdateGUI
================
*/
void jkSimpleForcePower::UpdateGUI( void ) {
	if ( !renderEntity.gui[ 0 ] ) {
		return;
	}

	if ( status == FP_HOLSTERED ) {
		return;
	}

	if ( owner->weaponGone ) {
		// dropping weapons was implemented wierd, so we have to not update the gui when it happens or we'll get a negative ammo count
		return;
	}

	if ( gameLocal.localClientNum != owner->entityNumber ) {
		// if updating the hud for a followed client
		if ( gameLocal.localClientNum >= 0 && gameLocal.entities[ gameLocal.localClientNum ] && gameLocal.entities[ gameLocal.localClientNum ]->IsType( idPlayer::Type ) ) {
			idPlayer *p = static_cast< idPlayer * >( gameLocal.entities[ gameLocal.localClientNum ] );
			if ( !p->spectating || p->spectator != owner->entityNumber ) {
				return;
			}
		} else {
			return;
		}
	}

	int inclip = AmmoInClip();
	int ammoamount = AmmoAvailable();

	if ( ammoamount < 0 ) {
		// show infinite ammo
		renderEntity.gui[ 0 ]->SetStateString( "player_ammo", "" );
	} else {
		// show remaining ammo
		renderEntity.gui[ 0 ]->SetStateString( "player_totalammo", va( "%i", ammoamount - inclip) );
		renderEntity.gui[ 0 ]->SetStateString( "player_ammo", ClipSize() ? va( "%i", inclip ) : "--" );
		renderEntity.gui[ 0 ]->SetStateString( "player_clips", ClipSize() ? va("%i", ammoamount / ClipSize()) : "--" );
		renderEntity.gui[ 0 ]->SetStateString( "player_allammo", va( "%i/%i", inclip, ammoamount - inclip ) );
	}
	renderEntity.gui[ 0 ]->SetStateBool( "player_ammo_empty", ( ammoamount == 0 ) );
	renderEntity.gui[ 0 ]->SetStateBool( "player_clip_empty", ( inclip == 0 ) );
	renderEntity.gui[ 0 ]->SetStateBool( "player_clip_low", ( inclip <= lowAmmo ) );
}

/***********************************************************************

	Model and muzzleflash

***********************************************************************/

/*
================
jkSimpleForcePower::UpdateFlashPosition
================
*/
void jkSimpleForcePower::UpdateFlashPosition( void ) {
	// the flash has an explicit joint for locating it
	GetGlobalJointTransform( true, flashJointView, muzzleFlash.origin, muzzleFlash.axis );

	// if the desired point is inside or very close to a wall, back it up until it is clear
	idVec3	start = muzzleFlash.origin - playerViewAxis[0] * 16;
	idVec3	end = muzzleFlash.origin + playerViewAxis[0] * 8;
	trace_t	tr;
	gameLocal.clip.TracePoint( tr, start, end, MASK_SHOT_RENDERMODEL, owner );
	// be at least 8 units away from a solid
	muzzleFlash.origin = tr.endpos - playerViewAxis[0] * 8;

	// put the world muzzle flash on the end of the joint, no matter what
	GetGlobalJointTransform( false, flashJointWorld, worldMuzzleFlash.origin, worldMuzzleFlash.axis );
}

/*
================
jkSimpleForcePower::MuzzleFlashLight
================
*/
void jkSimpleForcePower::MuzzleFlashLight( void ) {

	if ( !lightOn && ( !g_muzzleFlash.GetBool() || !muzzleFlash.lightRadius[0] ) ) {
		return;
	}

	if ( flashJointView == INVALID_JOINT ) {
		return;
	}

	UpdateFlashPosition();

	// these will be different each fire
	muzzleFlash.shaderParms[ SHADERPARM_TIMEOFFSET ]	= -MS2SEC( gameLocal.time );
	muzzleFlash.shaderParms[ SHADERPARM_DIVERSITY ]		= renderEntity.shaderParms[ SHADERPARM_DIVERSITY ];

	worldMuzzleFlash.shaderParms[ SHADERPARM_TIMEOFFSET ]	= -MS2SEC( gameLocal.time );
	worldMuzzleFlash.shaderParms[ SHADERPARM_DIVERSITY ]	= renderEntity.shaderParms[ SHADERPARM_DIVERSITY ];

	// the light will be removed at this time
	muzzleFlashEnd = gameLocal.time + flashTime;

	if ( muzzleFlashHandle != -1 ) {
		gameRenderWorld->UpdateLightDef( muzzleFlashHandle, &muzzleFlash );
		gameRenderWorld->UpdateLightDef( worldMuzzleFlashHandle, &worldMuzzleFlash );
	} else {
		muzzleFlashHandle = gameRenderWorld->AddLightDef( &muzzleFlash );
		worldMuzzleFlashHandle = gameRenderWorld->AddLightDef( &worldMuzzleFlash );
	}
}

/*
================
jkSimpleForcePower::UpdateSkin
================
*/
bool jkSimpleForcePower::UpdateSkin( void ) {
	const function_t *func;

	if ( !isLinked ) {
		return false;
	}

	func = scriptObject.GetFunction( "UpdateSkin" );
	if ( !func ) {
		common->Warning( "Can't find function 'UpdateSkin' in object '%s'", scriptObject.GetTypeName() );
		return false;
	}

	// use the frameCommandThread since it's safe to use outside of framecommands
	gameLocal.frameCommandThread->CallFunction( this, func, true );
	gameLocal.frameCommandThread->Execute();

	return true;
}

/*
================
jkSimpleForcePower::SetModel
================
*/
void jkSimpleForcePower::SetModel( const char *modelname ) {
	assert( modelname );

	if ( modelDefHandle >= 0 ) {
		gameRenderWorld->RemoveDecals( modelDefHandle );
	}

	renderEntity.hModel = animator.SetModel( modelname );
	if ( renderEntity.hModel ) {
		renderEntity.customSkin = animator.ModelDef()->GetDefaultSkin();
		animator.GetJoints( &renderEntity.numJoints, &renderEntity.joints );
	} else {
		renderEntity.customSkin = NULL;
		renderEntity.callback = NULL;
		renderEntity.numJoints = 0;
		renderEntity.joints = NULL;
	}

	// hide the model until an animation is played
	Hide();
}

/*
================
jkSimpleForcePower::GetGlobalJointTransform

This returns the offset and axis of a weapon bone in world space, suitable for attaching models or lights
================
*/
bool jkSimpleForcePower::GetGlobalJointTransform( bool viewModel, const jointHandle_t jointHandle, idVec3 &offset, idMat3 &axis ) {
	if ( viewModel ) {
		// view model
		if ( animator.GetJointTransform( jointHandle, gameLocal.time, offset, axis ) ) {
			offset = offset * viewWeaponAxis + viewWeaponOrigin;
			axis = axis * viewWeaponAxis;
			return true;
		}
	} else {
		// world model
		if ( worldModel.GetEntity() && worldModel.GetEntity()->GetAnimator()->GetJointTransform( jointHandle, gameLocal.time, offset, axis ) ) {
			offset = worldModel.GetEntity()->GetPhysics()->GetOrigin() + offset * worldModel.GetEntity()->GetPhysics()->GetAxis();
			axis = axis * worldModel.GetEntity()->GetPhysics()->GetAxis();
			return true;
		}
	}
	offset = viewWeaponOrigin;
	axis = viewWeaponAxis;
	return false;
}

/*
================
jkSimpleForcePower::SetPushVelocity
================
*/
void jkSimpleForcePower::SetPushVelocity( const idVec3 &pushVelocity ) {
	this->pushVelocity = pushVelocity;
}


/***********************************************************************

	State control/player interface

***********************************************************************/

/*
================
jkSimpleForcePower::Think
================
*/
void jkSimpleForcePower::Think( void ) {
	// do nothing because the present is called from the player through PresentWeapon
}

/*
================
jkSimpleForcePower::Raise
================
*/
void jkSimpleForcePower::Raise( void ) {
	if ( isLinked ) {
		WEAPON_RAISEWEAPON = true;
	}
}

/*
================
jkSimpleForcePower::PutAway
================
*/
void jkSimpleForcePower::PutAway( void ) {
	hasBloodSplat = false;
	if ( isLinked ) {
		WEAPON_LOWERWEAPON = true;
	}
}

/*
================
jkSimpleForcePower::Reload
NOTE: this is only for impulse-triggered reload, auto reload is scripted
================
*/
void jkSimpleForcePower::Reload( void ) {
	if ( isLinked ) {
		WEAPON_RELOAD = true;
	}
}

/*
================
jkSimpleForcePower::LowerWeapon
================
*/
void jkSimpleForcePower::LowerWeapon( void ) {
	if ( !hide ) {
		hideStart	= 0.0f;
		hideEnd		= hideDistance;
		if ( gameLocal.time - hideStartTime < hideTime ) {
			hideStartTime = gameLocal.time - ( hideTime - ( gameLocal.time - hideStartTime ) );
		} else {
			hideStartTime = gameLocal.time;
		}
		hide = true;
	}
}

/*
================
jkSimpleForcePower::RaiseWeapon
================
*/
void jkSimpleForcePower::RaiseWeapon( void ) {
	Show();

	if ( hide ) {
		hideStart	= hideDistance;
		hideEnd		= 0.0f;
		if ( gameLocal.time - hideStartTime < hideTime ) {
			hideStartTime = gameLocal.time - ( hideTime - ( gameLocal.time - hideStartTime ) );
		} else {
			hideStartTime = gameLocal.time;
		}
		hide = false;
	}
}

/*
================
jkSimpleForcePower::HideWeapon
================
*/
void jkSimpleForcePower::HideWeapon( void ) {
	Hide();
	if ( worldModel.GetEntity() ) {
		worldModel.GetEntity()->Hide();
	}
	muzzleFlashEnd = 0;
}

/*
================
jkSimpleForcePower::ShowWeapon
================
*/
void jkSimpleForcePower::ShowWeapon( void ) {
	Show();
	if ( worldModel.GetEntity() ) {
		worldModel.GetEntity()->Show();
	}
	if ( lightOn ) {
		MuzzleFlashLight();
	}
}

/*
================
jkSimpleForcePower::HideWorldModel
================
*/
void jkSimpleForcePower::HideWorldModel( void ) {
	if ( worldModel.GetEntity() ) {
		worldModel.GetEntity()->Hide();
	}
}

/*
================
jkSimpleForcePower::ShowWorldModel
================
*/
void jkSimpleForcePower::ShowWorldModel( void ) {
	if ( worldModel.GetEntity() ) {
		worldModel.GetEntity()->Show();
	}
}

/*
================
jkSimpleForcePower::OwnerDied
================
*/
void jkSimpleForcePower::OwnerDied( void ) {
	if ( isLinked ) {
		SetState( "OwnerDied", 0 );
		thread->Execute();
	}

	Hide();
	if ( worldModel.GetEntity() ) {
		worldModel.GetEntity()->Hide();
	}

	// don't clear the weapon immediately since the owner might have killed himself by firing the weapon
	// within the current stack frame
	PostEventMS( &EV_Weapon_Clear, 0 );
}

/*
================
jkSimpleForcePower::BeginAttack
================
*/
void jkSimpleForcePower::BeginAttack( void ) {
	if ( status != FP_OUTOFAMMO ) {
		lastAttack = gameLocal.time;
	}

	if ( !isLinked ) {
		return;
	}

	if ( !WEAPON_ATTACK ) {
		if ( sndHum ) {
			StopSound( SND_CHANNEL_BODY, false );
		}
	}

	WEAPON_ATTACK = true;

}

/*
================
jkSimpleForcePower::BeginAttackAlt
================
*/
void jkSimpleForcePower::BeginAttackAlt( void ) {
	if ( status != FP_OUTOFAMMO ) {
		lastAttack = gameLocal.time;
	}

	if ( !isLinked ) {
		return;
	}

	if ( !WEAPON_ATTACK_ALT ) {
		if ( sndHum ) {
			StopSound( SND_CHANNEL_BODY, false );
		}
	}
	WEAPON_ATTACK_ALT = true;
}

/*
================
jkSimpleForcePower::EndAttack
================
*/
void jkSimpleForcePower::EndAttack( void ) {
	if ( !WEAPON_ATTACK.IsLinked() ) {
		return;
	}
	if ( WEAPON_ATTACK ) {
		WEAPON_ATTACK = false;
		if ( sndHum ) {
			StartSoundShader( sndHum, SND_CHANNEL_BODY, 0, false, NULL );
		}
	}
}

/*
================
jkSimpleForcePower::EndAttackAlt
================
*/
void jkSimpleForcePower::EndAttackAlt( void ) {
	if ( !WEAPON_ATTACK_ALT.IsLinked() ) {
		return;
	}
	if ( WEAPON_ATTACK_ALT ) {
		WEAPON_ATTACK_ALT = false;
		if ( sndHum ) {
			StartSoundShader( sndHum, SND_CHANNEL_BODY, 0, false, NULL );
		}
	}
}

/*
================
jkSimpleForcePower::isReady
================
*/
bool jkSimpleForcePower::IsReady( void ) const {
	return !hide && !IsHidden() && ( ( status == FP_RELOAD ) || ( status == FP_READY ) || ( status == FP_OUTOFAMMO ) );
}

/*
================
jkSimpleForcePower::IsReloading
================
*/
bool jkSimpleForcePower::IsReloading( void ) const {
	return ( status == FP_RELOAD );
}

/*
================
jkSimpleForcePower::IsHolstered
================
*/
bool jkSimpleForcePower::IsHolstered( void ) const {
	return ( status == FP_HOLSTERED );
}

/*
================
jkSimpleForcePower::ShowCrosshair
================
*/
bool jkSimpleForcePower::ShowCrosshair( void ) const {
	return !( state == idStr( FP_RISING ) || state == idStr( FP_LOWERING ) || state == idStr( FP_HOLSTERED ) );
}

/*
=====================
jkSimpleForcePower::CanDrop
=====================
*/
bool jkSimpleForcePower::CanDrop( void ) const {
	if ( !weaponDef || !worldModel.GetEntity() ) {
		return false;
	}
	const char *classname = weaponDef->dict.GetString( "def_dropItem" );
	if ( !classname[ 0 ] ) {
		return false;
	}
	return true;
}

/*
================
jkSimpleForcePower::WeaponStolen
================
*/
void jkSimpleForcePower::WeaponStolen( void ) {
	assert( !gameLocal.isClient );
	if ( projectileEnt ) {
		if ( isLinked ) {
			SetState( "WeaponStolen", 0 );
			thread->Execute();
		}
		projectileEnt = NULL;
	}

	// set to holstered so we can switch weapons right away
	status = FP_HOLSTERED;

	HideWeapon();
}

/*
=====================
jkSimpleForcePower::DropItem
=====================
*/
idEntity * jkSimpleForcePower::DropItem( const idVec3 &velocity, int activateDelay, int removeDelay, bool died ) {
	if ( !weaponDef || !worldModel.GetEntity() ) {
		return NULL;
	}
	if ( !allowDrop ) {
		return NULL;
	}
	const char *classname = weaponDef->dict.GetString( "def_dropItem" );
	if ( !classname[0] ) {
		return NULL;
	}
	StopSound( SND_CHANNEL_BODY, true );
	StopSound( SND_CHANNEL_BODY3, true );

	return idMoveableItem::DropItem( classname, worldModel.GetEntity()->GetPhysics()->GetOrigin(), worldModel.GetEntity()->GetPhysics()->GetAxis(), velocity, activateDelay, removeDelay );
}

/***********************************************************************

	Script state management

***********************************************************************/

/*
=====================
jkSimpleForcePower::SetState
=====================
*/
void jkSimpleForcePower::SetState( const char *statename, int blendFrames ) {
	const function_t *func;

	if ( !isLinked ) {
		return;
	}

	func = scriptObject.GetFunction( statename );
	if ( !func ) {
		assert( 0 );
		gameLocal.Error( "Can't find function '%s' in object '%s'", statename, scriptObject.GetTypeName() );
	}

	thread->CallFunction( this, func, true );
	state = statename;

	animBlendFrames = blendFrames;
	if ( g_debugWeapon.GetBool() ) {
		gameLocal.Printf( "%d: weapon state : %s\n", gameLocal.time, statename );
	}

	idealState = "";
}


/***********************************************************************

	Particles/Effects

***********************************************************************/

/*
================
jkSimpleForcePower::UpdateNozzelFx
================
*/
void jkSimpleForcePower::UpdateNozzleFx( void ) {
	if ( !nozzleFx ) {
		return;
	}

	//
	// shader parms
	//
	int la = gameLocal.time - lastAttack + 1;
	float s = 1.0f;
	float l = 0.0f;
	if ( la < nozzleFxFade ) {
		s = ((float)la / nozzleFxFade);
		l = 1.0f - s;
	}
	renderEntity.shaderParms[5] = s;
	renderEntity.shaderParms[6] = l;

	if ( ventLightJointView == INVALID_JOINT ) {
		return;
	}

	//
	// vent light
	//
	if ( nozzleGlowHandle == -1 ) {
		memset(&nozzleGlow, 0, sizeof(nozzleGlow));
		if ( owner ) {
			nozzleGlow.allowLightInViewID = owner->entityNumber+1;
		}
		nozzleGlow.pointLight = true;
		nozzleGlow.noShadows = true;
		nozzleGlow.lightRadius.x = nozzleGlowRadius;
		nozzleGlow.lightRadius.y = nozzleGlowRadius;
		nozzleGlow.lightRadius.z = nozzleGlowRadius;
		nozzleGlow.shader = nozzleGlowShader;
		nozzleGlow.shaderParms[ SHADERPARM_TIMESCALE ]	= 1.0f;
		nozzleGlow.shaderParms[ SHADERPARM_TIMEOFFSET ]	= -MS2SEC( gameLocal.time );
		GetGlobalJointTransform( true, ventLightJointView, nozzleGlow.origin, nozzleGlow.axis );
		nozzleGlowHandle = gameRenderWorld->AddLightDef(&nozzleGlow);
	}

	GetGlobalJointTransform( true, ventLightJointView, nozzleGlow.origin, nozzleGlow.axis );

	nozzleGlow.shaderParms[ SHADERPARM_RED ] = nozzleGlowColor.x * s;
	nozzleGlow.shaderParms[ SHADERPARM_GREEN ] = nozzleGlowColor.y * s;
	nozzleGlow.shaderParms[ SHADERPARM_BLUE ] = nozzleGlowColor.z * s;
	gameRenderWorld->UpdateLightDef(nozzleGlowHandle, &nozzleGlow);
}


/*
================
jkSimpleForcePower::BloodSplat
================
*/
bool jkSimpleForcePower::BloodSplat( float size ) {
	float s, c;
	idMat3 localAxis, axistemp;
	idVec3 localOrigin, normal;

	if ( hasBloodSplat ) {
		return true;
	}

	hasBloodSplat = true;

	if ( modelDefHandle < 0 ) {
		return false;
	}

	if ( !GetGlobalJointTransform( true, ejectJointView, localOrigin, localAxis ) ) {
		return false;
	}

	localOrigin[0] += gameLocal.random.RandomFloat() * -10.0f;
	localOrigin[1] += gameLocal.random.RandomFloat() * 1.0f;
	localOrigin[2] += gameLocal.random.RandomFloat() * -2.0f;

	normal = idVec3( gameLocal.random.CRandomFloat(), -gameLocal.random.RandomFloat(), -1 );
	normal.Normalize();

	idMath::SinCos16( gameLocal.random.RandomFloat() * idMath::TWO_PI, s, c );

	localAxis[2] = -normal;
	localAxis[2].NormalVectors( axistemp[0], axistemp[1] );
	localAxis[0] = axistemp[ 0 ] * c + axistemp[ 1 ] * -s;
	localAxis[1] = axistemp[ 0 ] * -s + axistemp[ 1 ] * -c;

	localAxis[0] *= 1.0f / size;
	localAxis[1] *= 1.0f / size;

	idPlane		localPlane[2];

	localPlane[0] = localAxis[0];
	localPlane[0][3] = -(localOrigin * localAxis[0]) + 0.5f;

	localPlane[1] = localAxis[1];
	localPlane[1][3] = -(localOrigin * localAxis[1]) + 0.5f;

	const idMaterial *mtr = declManager->FindMaterial( "textures/decals/duffysplatgun" );

	gameRenderWorld->ProjectOverlay( modelDefHandle, localPlane, mtr );

	return true;
}


/***********************************************************************

	Visual presentation

***********************************************************************/

/*
================
jkSimpleForcePower::MuzzleRise

The machinegun and chaingun will incrementally back up as they are being fired
================
*/
void jkSimpleForcePower::MuzzleRise( idVec3 &origin, idMat3 &axis ) {
	int			time;
	float		amount;
	idAngles	ang;
	idVec3		offset;

	time = kick_endtime - gameLocal.time;
	if ( time <= 0 ) {
		return;
	}

	if ( muzzle_kick_maxtime <= 0 ) {
		return;
	}

	if ( time > muzzle_kick_maxtime ) {
		time = muzzle_kick_maxtime;
	}

	amount = ( float )time / ( float )muzzle_kick_maxtime;
	ang		= muzzle_kick_angles * amount;
	offset	= muzzle_kick_offset * amount;

	origin = origin - axis * offset;
	axis = ang.ToMat3() * axis;
}

/*
================
jkSimpleForcePower::ConstructScriptObject

Called during idEntity::Spawn.  Calls the constructor on the script object.
Can be overridden by subclasses when a thread doesn't need to be allocated.
================
*/
idThread *jkSimpleForcePower::ConstructScriptObject( void ) {
	const function_t *constructor;

	thread->EndThread();

	// call script object's constructor
	constructor = scriptObject.GetConstructor();
	if ( !constructor ) {
		gameLocal.Error( "Missing constructor on '%s' for weapon", scriptObject.GetTypeName() );
	}

	// init the script object's data
	scriptObject.ClearObject();
	thread->CallFunction( this, constructor, true );
	thread->Execute();

	return thread;
}

/*
================
jkSimpleForcePower::DeconstructScriptObject

Called during idEntity::~idEntity.  Calls the destructor on the script object.
Can be overridden by subclasses when a thread doesn't need to be allocated.
Not called during idGameLocal::MapShutdown.
================
*/
void jkSimpleForcePower::DeconstructScriptObject( void ) {
	const function_t *destructor;

	if ( !thread ) {
		return;
	}

	// don't bother calling the script object's destructor on map shutdown
	if ( gameLocal.GameState() == GAMESTATE_SHUTDOWN ) {
		return;
	}

	thread->EndThread();

	// call script object's destructor
	destructor = scriptObject.GetDestructor();
	if ( destructor ) {
		// start a thread that will run immediately and end
		thread->CallFunction( this, destructor, true );
		thread->Execute();
		thread->EndThread();
	}

	// clear out the object's memory
	scriptObject.ClearObject();
}

/*
================
jkSimpleForcePower::UpdateScript
================
*/
void jkSimpleForcePower::UpdateScript( void ) {
	int	count;

	if ( !isLinked ) {
		return;
	}

	// only update the script on new frames
	if ( !gameLocal.isNewFrame ) {
		return;
	}

	if ( idealState.Length() ) {
		SetState( idealState, animBlendFrames );
	}

	// update script state, which may call Event_LaunchProjectiles, among other things
	count = 10;
	while( ( thread->Execute() || idealState.Length() ) && count-- ) {
		// happens for weapons with no clip (like grenades)
		if ( idealState.Length() ) {
			SetState( idealState, animBlendFrames );
		}
	}

	WEAPON_RELOAD = false;
}

/*
================
jkSimpleForcePower::AlertMonsters
================
*/
void jkSimpleForcePower::AlertMonsters( void ) {
	trace_t	tr;
	idEntity *ent;
	idVec3 end = muzzleFlash.origin + muzzleFlash.axis * muzzleFlash.target;

	gameLocal.clip.TracePoint( tr, muzzleFlash.origin, end, CONTENTS_OPAQUE | MASK_SHOT_RENDERMODEL | CONTENTS_FLASHLIGHT_TRIGGER, owner );
	if ( g_debugWeapon.GetBool() ) {
		gameRenderWorld->DebugLine( colorYellow, muzzleFlash.origin, end, 0 );
		gameRenderWorld->DebugArrow( colorGreen, muzzleFlash.origin, tr.endpos, 2, 0 );
	}

	if ( tr.fraction < 1.0f ) {
		ent = gameLocal.GetTraceEntity( tr );
		if ( ent->IsType( idAI::Type ) ) {
			static_cast<idAI *>( ent )->TouchedByFlashlight( owner );
		} else if ( ent->IsType( idTrigger::Type ) ) {
			ent->Signal( SIG_TOUCH );
			ent->ProcessEvent( &EV_Touch, owner, &tr );
		}
	}

	// jitter the trace to try to catch cases where a trace down the center doesn't hit the monster
	end += muzzleFlash.axis * muzzleFlash.right * idMath::Sin16( MS2SEC( gameLocal.time ) * 31.34f );
	end += muzzleFlash.axis * muzzleFlash.up * idMath::Sin16( MS2SEC( gameLocal.time ) * 12.17f );
	gameLocal.clip.TracePoint( tr, muzzleFlash.origin, end, CONTENTS_OPAQUE | MASK_SHOT_RENDERMODEL | CONTENTS_FLASHLIGHT_TRIGGER, owner );
	if ( g_debugWeapon.GetBool() ) {
		gameRenderWorld->DebugLine( colorYellow, muzzleFlash.origin, end, 0 );
		gameRenderWorld->DebugArrow( colorGreen, muzzleFlash.origin, tr.endpos, 2, 0 );
	}

	if ( tr.fraction < 1.0f ) {
		ent = gameLocal.GetTraceEntity( tr );
		if ( ent->IsType( idAI::Type ) ) {
			static_cast<idAI *>( ent )->TouchedByFlashlight( owner );
		} else if ( ent->IsType( idTrigger::Type ) ) {
			ent->Signal( SIG_TOUCH );
			ent->ProcessEvent( &EV_Touch, owner, &tr );
		}
	}
}

/*
================
jkSimpleForcePower::PresentWeapon
================
*/
void jkSimpleForcePower::PresentWeapon( bool showViewModel ) {
	playerViewOrigin = owner->firstPersonViewOrigin;
	playerViewAxis = owner->firstPersonViewAxis;

	// calculate weapon position based on player movement bobbing
	owner->CalculateViewWeaponPos( viewWeaponOrigin, viewWeaponAxis );

	// hide offset is for dropping the gun when approaching a GUI or NPC
	// This is simpler to manage than doing the weapon put-away animation
	if ( gameLocal.time - hideStartTime < hideTime ) {
		float frac = ( float )( gameLocal.time - hideStartTime ) / ( float )hideTime;
		if ( hideStart < hideEnd ) {
			frac = 1.0f - frac;
			frac = 1.0f - frac * frac;
		} else {
			frac = frac * frac;
		}
		hideOffset = hideStart + ( hideEnd - hideStart ) * frac;
	} else {
		hideOffset = hideEnd;
		if ( hide && disabled ) {
			Hide();
		}
	}
	viewWeaponOrigin += hideOffset * viewWeaponAxis[ 2 ];

	// kick up based on repeat firing
	MuzzleRise( viewWeaponOrigin, viewWeaponAxis );

	// set the physics position and orientation
	GetPhysics()->SetOrigin( viewWeaponOrigin );
	GetPhysics()->SetAxis( viewWeaponAxis );
	UpdateVisuals();

	// update the weapon script
	UpdateScript();

	UpdateGUI();

	// update animation
	UpdateAnimation();

	// only show the surface in player view
	renderEntity.allowSurfaceInViewID = owner->entityNumber+1;

	// crunch the depth range so it never pokes into walls this breaks the machine gun gui
	renderEntity.weaponDepthHack = true;

	// present the model
	if ( showViewModel ) {
		Present();
	} else {
		FreeModelDef();
	}

	if ( worldModel.GetEntity() && worldModel.GetEntity()->GetRenderEntity() ) {
		// deal with the third-person visible world model
		// don't show shadows of the world model in first person
		if ( gameLocal.isMultiplayer || g_showPlayerShadow.GetBool() || pm_thirdPerson.GetBool() ) {
			worldModel.GetEntity()->GetRenderEntity()->suppressShadowInViewID	= 0;
		} else {
			worldModel.GetEntity()->GetRenderEntity()->suppressShadowInViewID	= owner->entityNumber+1;
			worldModel.GetEntity()->GetRenderEntity()->suppressShadowInLightID = LIGHTID_VIEW_MUZZLE_FLASH + owner->entityNumber;
		}
	}

	if ( nozzleFx ) {
		UpdateNozzleFx();
	}

	// muzzle smoke
	if ( showViewModel && !disabled && weaponSmoke && ( weaponSmokeStartTime != 0 ) ) {
		// use the barrel joint if available
		if ( barrelJointView ) {
			GetGlobalJointTransform( true, barrelJointView, muzzleOrigin, muzzleAxis );
		} else {
			// default to going straight out the view
			muzzleOrigin = playerViewOrigin;
			muzzleAxis = playerViewAxis;
		}
		// spit out a particle
		if ( !gameLocal.smokeParticles->EmitSmoke( weaponSmoke, weaponSmokeStartTime, gameLocal.random.RandomFloat(), muzzleOrigin, muzzleAxis ) ) {
			weaponSmokeStartTime = ( continuousSmoke ) ? gameLocal.time : 0;
		}
	}

	if ( showViewModel && strikeSmoke && strikeSmokeStartTime != 0 ) {
		// spit out a particle
		if ( !gameLocal.smokeParticles->EmitSmoke( strikeSmoke, strikeSmokeStartTime, gameLocal.random.RandomFloat(), strikePos, strikeAxis ) ) {
			strikeSmokeStartTime = 0;
		}
	}

	// remove the muzzle flash light when it's done
	if ( ( !lightOn && ( gameLocal.time >= muzzleFlashEnd ) ) || IsHidden() ) {
		if ( muzzleFlashHandle != -1 ) {
			gameRenderWorld->FreeLightDef( muzzleFlashHandle );
			muzzleFlashHandle = -1;
		}
		if ( worldMuzzleFlashHandle != -1 ) {
			gameRenderWorld->FreeLightDef( worldMuzzleFlashHandle );
			worldMuzzleFlashHandle = -1;
		}
	}

	// update the muzzle flash light, so it moves with the gun
	if ( muzzleFlashHandle != -1 ) {
		UpdateFlashPosition();
		gameRenderWorld->UpdateLightDef( muzzleFlashHandle, &muzzleFlash );
		gameRenderWorld->UpdateLightDef( worldMuzzleFlashHandle, &worldMuzzleFlash );

		// wake up monsters with the flashlight
		if ( !gameLocal.isMultiplayer && lightOn && !owner->fl.notarget ) {
			AlertMonsters();
		}
	}

	// update the gui light
	if ( guiLight.lightRadius[0] && guiLightJointView != INVALID_JOINT ) {
		GetGlobalJointTransform( true, guiLightJointView, guiLight.origin, guiLight.axis );

		if ( ( guiLightHandle != -1 ) ) {
			gameRenderWorld->UpdateLightDef( guiLightHandle, &guiLight );
		} else {
			guiLightHandle = gameRenderWorld->AddLightDef( &guiLight );
		}
	}

	if ( status != FP_READY && sndHum ) {
		StopSound( SND_CHANNEL_BODY, false );
	}

	UpdateSound();
}

/*
================
jkSimpleForcePower::EnterCinematic
================
*/
void jkSimpleForcePower::EnterCinematic( void ) {
	StopSound( SND_CHANNEL_ANY, false );

	if ( isLinked ) {
		SetState( "EnterCinematic", 0 );
		thread->Execute();

		WEAPON_ATTACK		= false;
		WEAPON_ATTACK_ALT	= false;
		WEAPON_RELOAD		= false;
		WEAPON_NETRELOAD	= false;
		WEAPON_NETENDRELOAD	= false;
		WEAPON_NETFIRING	= false;
		WEAPON_RAISEWEAPON	= false;
		WEAPON_LOWERWEAPON	= false;
	}

	disabled = true;

	LowerWeapon();
}

/*
================
jkSimpleForcePower::ExitCinematic
================
*/
void jkSimpleForcePower::ExitCinematic( void ) {
	disabled = false;

	if ( isLinked ) {
		SetState( "ExitCinematic", 0 );
		thread->Execute();
	}

	RaiseWeapon();
}

/*
================
jkSimpleForcePower::NetCatchup
================
*/
void jkSimpleForcePower::NetCatchup( void ) {
	if ( isLinked ) {
		SetState( "NetCatchup", 0 );
		thread->Execute();
	}
}

/*
================
jkSimpleForcePower::GetZoomFov
================
*/
int	jkSimpleForcePower::GetZoomFov( void ) {
	return zoomFov;
}

/*
================
jkSimpleForcePower::GetWeaponAngleOffsets
================
*/
void jkSimpleForcePower::GetWeaponAngleOffsets( int *average, float *scale, float *max ) {
	*average = weaponAngleOffsetAverages;
	*scale = weaponAngleOffsetScale;
	*max = weaponAngleOffsetMax;
}

/*
================
jkSimpleForcePower::GetWeaponTimeOffsets
================
*/
void jkSimpleForcePower::GetWeaponTimeOffsets( float *time, float *scale ) {
	*time = weaponOffsetTime;
	*scale = weaponOffsetScale;
}


/***********************************************************************

	Ammo

***********************************************************************/

/*
================
jkSimpleForcePower::GetAmmoNumForName
================
*/
ammo_t jkSimpleForcePower::GetAmmoNumForName( const char *ammoname ) {
	int num;
	const idDict *ammoDict;

	assert( ammoname );

	ammoDict = gameLocal.FindEntityDefDict( "ammo_types", false );
	if ( !ammoDict ) {
		gameLocal.Error( "Could not find entity definition for 'ammo_types'\n" );
	}

	if ( !ammoname[ 0 ] ) {
		return 0;
	}

	if ( !ammoDict->GetInt( ammoname, "-1", num ) ) {
		gameLocal.Error( "Unknown ammo type '%s'", ammoname );
	}

	if ( ( num < 0 ) || ( num >= AMMO_NUMTYPES ) ) {
		gameLocal.Error( "Ammo type '%s' value out of range.  Maximum ammo types is %d.\n", ammoname, AMMO_NUMTYPES );
	}

	return ( ammo_t )num;
}

/*
================
jkSimpleForcePower::GetAmmoNameForNum
================
*/
const char *jkSimpleForcePower::GetAmmoNameForNum( ammo_t ammonum ) {
	int i;
	int num;
	const idDict *ammoDict;
	const idKeyValue *kv;
	char text[ 32 ];

	ammoDict = gameLocal.FindEntityDefDict( "ammo_types", false );
	if ( !ammoDict ) {
		gameLocal.Error( "Could not find entity definition for 'ammo_types'\n" );
	}

	sprintf( text, "%d", ammonum );

	num = ammoDict->GetNumKeyVals();
	for( i = 0; i < num; i++ ) {
		kv = ammoDict->GetKeyVal( i );
		if ( kv->GetValue() == text ) {
			return kv->GetKey();
		}
	}

	return NULL;
}

/*
================
jkSimpleForcePower::GetAmmoPickupNameForNum
================
*/
const char *jkSimpleForcePower::GetAmmoPickupNameForNum( ammo_t ammonum ) {
	int i;
	int num;
	const idDict *ammoDict;
	const idKeyValue *kv;

	ammoDict = gameLocal.FindEntityDefDict( "ammo_names", false );
	if ( !ammoDict ) {
		gameLocal.Error( "Could not find entity definition for 'ammo_names'\n" );
	}

	const char *name = GetAmmoNameForNum( ammonum );

	if ( name && *name ) {
		num = ammoDict->GetNumKeyVals();
		for( i = 0; i < num; i++ ) {
			kv = ammoDict->GetKeyVal( i );
			if ( idStr::Icmp( kv->GetKey(), name) == 0 ) {
				return kv->GetValue();
			}
		}
	}

	return "";
}

/*
================
jkSimpleForcePower::AmmoAvailable
================
*/
int jkSimpleForcePower::AmmoAvailable( void ) const {
	if ( owner ) {
		return owner->inventory.HasAmmo( ammoType, ammoRequired );
	} else {
		return 0;
	}
}

/*
================
jkSimpleForcePower::AmmoInClip
================
*/
int jkSimpleForcePower::AmmoInClip( void ) const {
	return ammoClip;
}

/*
================
jkSimpleForcePower::ResetAmmoClip
================
*/
void jkSimpleForcePower::ResetAmmoClip( void ) {
	ammoClip = -1;
}

/*
================
jkSimpleForcePower::GetAmmoType
================
*/
ammo_t jkSimpleForcePower::GetAmmoType( void ) const {
	return ammoType;
}

/*
================
jkSimpleForcePower::ClipSize
================
*/
int	jkSimpleForcePower::ClipSize( void ) const {
	return clipSize;
}

/*
================
jkSimpleForcePower::LowAmmo
================
*/
int	jkSimpleForcePower::LowAmmo() const {
	return lowAmmo;
}

/*
================
jkSimpleForcePower::AmmoRequired
================
*/
int	jkSimpleForcePower::AmmoRequired( void ) const {
	return ammoRequired;
}

/*
================
jkSimpleForcePower::WriteToSnapshot
================
*/
void jkSimpleForcePower::WriteToSnapshot( idBitMsgDelta &msg ) const {
	msg.WriteBits( ammoClip, ASYNC_PLAYER_INV_CLIP_BITS );
	msg.WriteBits( worldModel.GetSpawnId(), 32 );
	msg.WriteBits( lightOn, 1 );
	msg.WriteBits( isFiring ? 1 : 0, 1 );
}

/*
================
jkSimpleForcePower::ReadFromSnapshot
================
*/
void jkSimpleForcePower::ReadFromSnapshot( const idBitMsgDelta &msg ) {
	ammoClip = msg.ReadBits( ASYNC_PLAYER_INV_CLIP_BITS );
	worldModel.SetSpawnId( msg.ReadBits( 32 ) );
	bool snapLight = msg.ReadBits( 1 ) != 0;
	isFiring = msg.ReadBits( 1 ) != 0;

	// WEAPON_NETFIRING is only turned on for other clients we're predicting. not for local client
	if ( owner && gameLocal.localClientNum != owner->entityNumber && WEAPON_NETFIRING.IsLinked() ) {

		// immediately go to the firing state so we don't skip fire animations
		if ( !WEAPON_NETFIRING && isFiring ) {
			idealState = "Fire";
		}

		// immediately switch back to idle
		if ( WEAPON_NETFIRING && !isFiring ) {
			idealState = "Idle";
		}

		WEAPON_NETFIRING = isFiring;
	}

	if ( snapLight != lightOn ) {
		Reload();
	}
}

/*
================
jkSimpleForcePower::ClientReceiveEvent
================
*/
bool jkSimpleForcePower::ClientReceiveEvent( int event, int time, const idBitMsg &msg ) {

	switch( event ) {
		case EVENT_RELOAD: {
			if ( gameLocal.time - time < 1000 ) {
				if ( WEAPON_NETRELOAD.IsLinked() ) {
					WEAPON_NETRELOAD = true;
					WEAPON_NETENDRELOAD = false;
				}
			}
			return true;
		}
		case EVENT_ENDRELOAD: {
			if ( WEAPON_NETENDRELOAD.IsLinked() ) {
				WEAPON_NETENDRELOAD = true;
			}
			return true;
		}
		case EVENT_CHANGESKIN: {
			int index = gameLocal.ClientRemapDecl( DECL_SKIN, msg.ReadInt() );
			renderEntity.customSkin = ( index != -1 ) ? static_cast<const idDeclSkin *>( declManager->DeclByIndex( DECL_SKIN, index ) ) : NULL;
			UpdateVisuals();
			if ( worldModel.GetEntity() ) {
				worldModel.GetEntity()->SetSkin( renderEntity.customSkin );
			}
			return true;
		}
		default:
			break;
	}

	return idEntity::ClientReceiveEvent( event, time, msg );
}

/***********************************************************************

	Script events

***********************************************************************/

/*
===============
jkSimpleForcePower::Event_Clear
===============
*/
void jkSimpleForcePower::Event_Clear( void ) {
	Clear();
}

/*
===============
jkSimpleForcePower::Event_GetOwner
===============
*/
void jkSimpleForcePower::Event_GetOwner( void ) {
	idThread::ReturnEntity( owner );
}

/*
===============
jkSimpleForcePower::Event_WeaponState
===============
*/
void jkSimpleForcePower::Event_WeaponState( const char *statename, int blendFrames ) {
	const function_t *func;

	func = scriptObject.GetFunction( statename );
	if ( !func ) {
		assert( 0 );
		gameLocal.Error( "Can't find function '%s' in object '%s'", statename, scriptObject.GetTypeName() );
	}

	idealState = statename;

	if ( !idealState.Icmp( "Fire" ) ) {
		isFiring = true;
		isFiringAlt = false;
	} else if ( !idealState.Icmp( "AltFire" ) ) {
		isFiring = false;
		isFiringAlt = true;
	} else {
		isFiring = false;
		isFiringAlt = false;
	}

	animBlendFrames = blendFrames;
	thread->DoneProcessing();
}

/*
===============
jkSimpleForcePower::Event_WeaponReady
===============
*/
void jkSimpleForcePower::Event_WeaponReady( void ) {
	status = FP_READY;
	if ( isLinked ) {
		WEAPON_RAISEWEAPON = false;
	}
	if ( sndHum ) {
		StartSoundShader( sndHum, SND_CHANNEL_BODY, 0, false, NULL );
	}

}

/*
===============
jkSimpleForcePower::Event_WeaponOutOfAmmo
===============
*/
void jkSimpleForcePower::Event_WeaponOutOfAmmo( void ) {
	status = FP_OUTOFAMMO;
	if ( isLinked ) {
		WEAPON_RAISEWEAPON = false;
	}
}

/*
===============
jkSimpleForcePower::Event_WeaponReloading
===============
*/
void jkSimpleForcePower::Event_WeaponReloading( void ) {
	status = FP_RELOAD;
}

/*
===============
jkSimpleForcePower::Event_WeaponHolstered
===============
*/
void jkSimpleForcePower::Event_WeaponHolstered( void ) {
	status = FP_HOLSTERED;
	if ( isLinked ) {
		WEAPON_LOWERWEAPON = false;
	}
}

/*
===============
jkSimpleForcePower::Event_WeaponRising
===============
*/
void jkSimpleForcePower::Event_WeaponRising( void ) {
	status = FP_RISING;
	if ( isLinked ) {
		WEAPON_LOWERWEAPON = false;
	}
	//owner->WeaponRisingCallback();
}

/*
===============
jkSimpleForcePower::Event_WeaponLowering
===============
*/
void jkSimpleForcePower::Event_WeaponLowering( void ) {
	status = FP_LOWERING;
	if ( isLinked ) {
		WEAPON_RAISEWEAPON = false;
	}
	//owner->WeaponLoweringCallback();
}

/*
===============
jkSimpleForcePower::Event_UseAmmo
===============
*/
void jkSimpleForcePower::Event_UseAmmo( int amount ) {
	if ( gameLocal.isClient ) {
		return;
	}

	owner->inventory.UseAmmo( ammoType, ( powerAmmo ) ? amount : ( amount * ammoRequired ) );
	if ( clipSize && ammoRequired ) {
		ammoClip -= powerAmmo ? amount : ( amount * ammoRequired );
		if ( ammoClip < 0 ) {
			ammoClip = 0;
		}
	}
}

/*
===============
jkSimpleForcePower::Event_AddToClip
===============
*/
void jkSimpleForcePower::Event_AddToClip( int amount ) {
	int ammoAvail;

	if ( gameLocal.isClient ) {
		return;
	}

	ammoClip += amount;
	if ( ammoClip > clipSize ) {
		ammoClip = clipSize;
	}

	ammoAvail = owner->inventory.HasAmmo( ammoType, ammoRequired );
	if ( ammoClip > ammoAvail ) {
		ammoClip = ammoAvail;
	}
}

/*
===============
jkSimpleForcePower::Event_AmmoInClip
===============
*/
void jkSimpleForcePower::Event_AmmoInClip( void ) {
	int ammo = AmmoInClip();
	idThread::ReturnFloat( ammo );
}

/*
===============
jkSimpleForcePower::Event_AmmoAvailable
===============
*/
void jkSimpleForcePower::Event_AmmoAvailable( void ) {
	int ammoAvail = owner->inventory.HasAmmo( ammoType, ammoRequired );
	idThread::ReturnFloat( ammoAvail );
}

/*
===============
jkSimpleForcePower::Event_TotalAmmoCount
===============
*/
void jkSimpleForcePower::Event_TotalAmmoCount( void ) {
	int ammoAvail = owner->inventory.HasAmmo( ammoType, 1 );
	idThread::ReturnFloat( ammoAvail );
}

/*
===============
jkSimpleForcePower::Event_ClipSize
===============
*/
void jkSimpleForcePower::Event_ClipSize( void ) {
	idThread::ReturnFloat( clipSize );
}

/*
===============
jkSimpleForcePower::Event_AutoReload
===============
*/
void jkSimpleForcePower::Event_AutoReload( void ) {
	assert( owner );
	if ( gameLocal.isClient ) {
		idThread::ReturnFloat( 0.0f );
		return;
	}
	idThread::ReturnFloat( gameLocal.userInfo[ owner->entityNumber ].GetBool( "ui_autoReload" ) );
}

/*
===============
jkSimpleForcePower::Event_NetReload
===============
*/
void jkSimpleForcePower::Event_NetReload( void ) {
	assert( owner );
	if ( gameLocal.isServer ) {
		ServerSendEvent( EVENT_RELOAD, NULL, false, -1 );
	}
}

/*
===============
jkSimpleForcePower::Event_NetEndReload
===============
*/
void jkSimpleForcePower::Event_NetEndReload( void ) {
	assert( owner );
	if ( gameLocal.isServer ) {
		ServerSendEvent( EVENT_ENDRELOAD, NULL, false, -1 );
	}
}

/*
===============
jkSimpleForcePower::Event_PlayAnim
===============
*/
void jkSimpleForcePower::Event_PlayAnim( int channel, const char *animname ) {
	int anim;

	anim = animator.GetAnim( animname );
	if ( !anim ) {
		gameLocal.Warning( "missing '%s' animation on '%s' (%s)", animname, name.c_str(), GetEntityDefName() );
		animator.Clear( channel, gameLocal.time, FRAME2MS( animBlendFrames ) );
		animDoneTime = 0;
	} else {
		if ( !( owner && owner->GetInfluenceLevel() ) ) {
			Show();
		}
		animator.PlayAnim( channel, anim, gameLocal.time, FRAME2MS( animBlendFrames ) );
		animDoneTime = animator.CurrentAnim( channel )->GetEndTime();
		if ( worldModel.GetEntity() ) {
			anim = worldModel.GetEntity()->GetAnimator()->GetAnim( animname );
			if ( anim ) {
				worldModel.GetEntity()->GetAnimator()->PlayAnim( channel, anim, gameLocal.time, FRAME2MS( animBlendFrames ) );
			}
		}
	}
	animBlendFrames = 0;
	idThread::ReturnInt( 0 );
}

/*
===============
jkSimpleForcePower::Event_PlayCycle
===============
*/
void jkSimpleForcePower::Event_PlayCycle( int channel, const char *animname ) {
	int anim;

	anim = animator.GetAnim( animname );
	if ( !anim ) {
		gameLocal.Warning( "missing '%s' animation on '%s' (%s)", animname, name.c_str(), GetEntityDefName() );
		animator.Clear( channel, gameLocal.time, FRAME2MS( animBlendFrames ) );
		animDoneTime = 0;
	} else {
		if ( !( owner && owner->GetInfluenceLevel() ) ) {
			Show();
		}
		animator.CycleAnim( channel, anim, gameLocal.time, FRAME2MS( animBlendFrames ) );
		animDoneTime = animator.CurrentAnim( channel )->GetEndTime();
		if ( worldModel.GetEntity() ) {
			anim = worldModel.GetEntity()->GetAnimator()->GetAnim( animname );
			worldModel.GetEntity()->GetAnimator()->CycleAnim( channel, anim, gameLocal.time, FRAME2MS( animBlendFrames ) );
		}
	}
	animBlendFrames = 0;
	idThread::ReturnInt( 0 );
}

/*
===============
jkSimpleForcePower::Event_AnimDone
===============
*/
void jkSimpleForcePower::Event_AnimDone( int channel, int blendFrames ) {
	if ( animDoneTime - FRAME2MS( blendFrames ) <= gameLocal.time ) {
		idThread::ReturnInt( true );
	} else {
		idThread::ReturnInt( false );
	}
}

/*
===============
jkSimpleForcePower::Event_SetBlendFrames
===============
*/
void jkSimpleForcePower::Event_SetBlendFrames( int channel, int blendFrames ) {
	animBlendFrames = blendFrames;
}

/*
===============
jkSimpleForcePower::Event_GetBlendFrames
===============
*/
void jkSimpleForcePower::Event_GetBlendFrames( int channel ) {
	idThread::ReturnInt( animBlendFrames );
}

/*
================
jkSimpleForcePower::Event_Next
================
*/
void jkSimpleForcePower::Event_Next( void ) {
	// change to another weapon if possible
	owner->NextBestWeapon();
}

/*
================
jkSimpleForcePower::Event_SetSkin
================
*/
void jkSimpleForcePower::Event_SetSkin( const char *skinname ) {
	const idDeclSkin *skinDecl;

	if ( !skinname || !skinname[ 0 ] ) {
		skinDecl = NULL;
	} else {
		skinDecl = declManager->FindSkin( skinname );
	}

	renderEntity.customSkin = skinDecl;
	UpdateVisuals();

	if ( worldModel.GetEntity() ) {
		worldModel.GetEntity()->SetSkin( skinDecl );
	}

	if ( gameLocal.isServer ) {
		idBitMsg	msg;
		byte		msgBuf[MAX_EVENT_PARAM_SIZE];

		msg.Init( msgBuf, sizeof( msgBuf ) );
		msg.WriteInt( ( skinDecl != NULL ) ? gameLocal.ServerRemapDecl( -1, DECL_SKIN, skinDecl->Index() ) : -1 );
		ServerSendEvent( EVENT_CHANGESKIN, &msg, false, -1 );
	}
}

/*
================
jkSimpleForcePower::Event_Flashlight
================
*/
void jkSimpleForcePower::Event_Flashlight( int enable ) {
	if ( enable ) {
		lightOn = true;
		MuzzleFlashLight();
	} else {
		lightOn = false;
		muzzleFlashEnd = 0;
	}
}

/*
================
jkSimpleForcePower::Event_GetLightParm
================
*/
void jkSimpleForcePower::Event_GetLightParm( int parmnum ) {
	if ( ( parmnum < 0 ) || ( parmnum >= MAX_ENTITY_SHADER_PARMS ) ) {
		gameLocal.Error( "shader parm index (%d) out of range", parmnum );
	}

	idThread::ReturnFloat( muzzleFlash.shaderParms[ parmnum ] );
}

/*
================
jkSimpleForcePower::Event_SetLightParm
================
*/
void jkSimpleForcePower::Event_SetLightParm( int parmnum, float value ) {
	if ( ( parmnum < 0 ) || ( parmnum >= MAX_ENTITY_SHADER_PARMS ) ) {
		gameLocal.Error( "shader parm index (%d) out of range", parmnum );
	}

	muzzleFlash.shaderParms[ parmnum ]		= value;
	worldMuzzleFlash.shaderParms[ parmnum ]	= value;
	UpdateVisuals();
}

/*
================
jkSimpleForcePower::Event_SetLightParms
================
*/
void jkSimpleForcePower::Event_SetLightParms( float parm0, float parm1, float parm2, float parm3 ) {
	muzzleFlash.shaderParms[ SHADERPARM_RED ]			= parm0;
	muzzleFlash.shaderParms[ SHADERPARM_GREEN ]			= parm1;
	muzzleFlash.shaderParms[ SHADERPARM_BLUE ]			= parm2;
	muzzleFlash.shaderParms[ SHADERPARM_ALPHA ]			= parm3;

	worldMuzzleFlash.shaderParms[ SHADERPARM_RED ]		= parm0;
	worldMuzzleFlash.shaderParms[ SHADERPARM_GREEN ]	= parm1;
	worldMuzzleFlash.shaderParms[ SHADERPARM_BLUE ]		= parm2;
	worldMuzzleFlash.shaderParms[ SHADERPARM_ALPHA ]	= parm3;

	UpdateVisuals();
}

/*
================
jkSimpleForcePower::Event_CreateProjectile
================
*/
void jkSimpleForcePower::Event_CreateProjectile( void ) {
	if ( !gameLocal.isClient ) {
		projectileEnt = NULL;
		gameLocal.SpawnEntityDef( projectileDict, &projectileEnt, false );
		if ( projectileEnt ) {
			projectileEnt->SetOrigin( GetPhysics()->GetOrigin() );
			projectileEnt->Bind( owner, false );
			projectileEnt->Hide();
		}
		idThread::ReturnEntity( projectileEnt );
	} else {
		idThread::ReturnEntity( NULL );
	}
}

/*
================
jkSimpleForcePower::Event_LaunchProjectiles
================
*/
void jkSimpleForcePower::Event_LaunchProjectiles( int num_projectiles, float spread, float fuseOffset, float launchPower, float dmgPower ) {
	idProjectile	*proj;
	idEntity		*ent;
	int				i;
	idVec3			dir;
	float			ang;
	float			spin;
	float			distance;
	trace_t			tr;
	idVec3			start;
	idVec3			muzzle_pos;
	idBounds		ownerBounds, projBounds;
	//Dynamix - dentonmod beam effect stuff
	bool 			barrelLaunch;
	bool			tracer, beam;

	if ( IsHidden() ) {
		return;
	}

	if ( !projectileDict.GetNumKeyVals() ) {
		const char *classname = weaponDef->dict.GetString( "classname" );
		gameLocal.Warning( "No projectile defined on '%s'", classname );
		return;
	}

	// avoid all ammo considerations on an MP client
	if ( !gameLocal.isClient ) {

		// check if we're out of ammo or the clip is empty
		int ammoAvail = owner->inventory.HasAmmo( ammoType, ammoRequired );
		if ( !ammoAvail || ( ( clipSize != 0 ) && ( ammoClip <= 0 ) ) ) {
			return;
		}

		// if this is a power ammo weapon ( currently only the bfg ) then make sure
		// we only fire as much power as available in each clip
		if ( powerAmmo ) {
			// power comes in as a float from zero to max
			// if we use this on more than the bfg will need to define the max
			// in the .def as opposed to just in the script so proper calcs
			// can be done here.
			dmgPower = ( int )dmgPower + 1;
			if ( dmgPower > ammoClip ) {
				dmgPower = ammoClip;
			}
		}

		owner->inventory.UseAmmo( ammoType, ( powerAmmo ) ? dmgPower : ammoRequired );
		if ( clipSize && ammoRequired ) {
			ammoClip -= powerAmmo ? dmgPower : 1;
		}

	}

	if ( !silent_fire ) {
		// wake up nearby monsters
		gameLocal.AlertAI( owner );
	}

	// set the shader parm to the time of last projectile firing,
	// which the gun material shaders can reference for single shot barrel glows, etc
	renderEntity.shaderParms[ SHADERPARM_DIVERSITY ]	= gameLocal.random.CRandomFloat();
	renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ]	= -MS2SEC( gameLocal.realClientTime );

	if ( worldModel.GetEntity() ) {
		worldModel.GetEntity()->SetShaderParm( SHADERPARM_DIVERSITY, renderEntity.shaderParms[ SHADERPARM_DIVERSITY ] );
		worldModel.GetEntity()->SetShaderParm( SHADERPARM_TIMEOFFSET, renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] );
	}
	//Dynamix dentonmod beam stuff
	idVec3 view_pos = playerViewOrigin + playerViewAxis[ 0 ] * 2.0f; // Muzzle pos for translation clip model only-- For barrel Launched projectiles

	float muzzleDistFromView;
	float traceDist, muzzleToTargetDist;
	idVec3 muzzleDir;

	beam			= projectileDict.GetFloat( "fuse" ) <= 0 || projectileDict.GetBool( "rail_beam");
	tracer			= !beam && projectileDict.GetBool( "tracers" ) && (projectileDict.GetFloat("tracer_probability", "1.0") > gameLocal.random.RandomFloat()); 
	barrelLaunch	= projectileDict.GetBool( "launchFromBarrel" );
	//End Dynamix

	// calculate the muzzle position
	if ( barrelJointView != INVALID_JOINT && projectileDict.GetBool( "launchFromBarrel" ) ) {
		// there is an explicit joint for the muzzle
		GetGlobalJointTransform( true, barrelJointView, muzzleOrigin, muzzleAxis );
		muzzle_pos = muzzleOrigin;
	} else {
		// go straight out of the view
		muzzleOrigin = playerViewOrigin;
		muzzleAxis = playerViewAxis;
	}

	// add some to the kick time, incrementally moving repeat firing weapons back
	if ( kick_endtime < gameLocal.realClientTime ) {
		kick_endtime = gameLocal.realClientTime;
	}
	kick_endtime += muzzle_kick_time;
	if ( kick_endtime > gameLocal.realClientTime + muzzle_kick_maxtime ) {
		kick_endtime = gameLocal.realClientTime + muzzle_kick_maxtime;
	}

	if ( gameLocal.isClient ) {

		// predict instant hit projectiles
		if ( projectileDict.GetBool( "net_instanthit" ) ) {
			float spreadRad = DEG2RAD( spread );
			muzzle_pos = muzzleOrigin + playerViewAxis[ 0 ] * 2.0f;
			for( i = 0; i < num_projectiles; i++ ) {
				ang = idMath::Sin( spreadRad * gameLocal.random.RandomFloat() );
				spin = (float)DEG2RAD( 360.0f ) * gameLocal.random.RandomFloat();
				dir = playerViewAxis[ 0 ] + playerViewAxis[ 2 ] * ( ang * idMath::Sin( spin ) ) - playerViewAxis[ 1 ] * ( ang * idMath::Cos( spin ) );
				dir.Normalize();
				gameLocal.clip.Translation( tr, muzzle_pos, muzzle_pos + dir * 4096.0f, NULL, mat3_identity, MASK_SHOT_RENDERMODEL, owner );
				if ( tr.fraction < 1.0f ) {
					idProjectile::ClientPredictionCollide( this, projectileDict, tr, vec3_origin, true );
				}
			}
		}

	} else {

		ownerBounds = owner->GetPhysics()->GetAbsBounds();

		owner->AddProjectilesFired( num_projectiles );

		float spreadRad = DEG2RAD( spread );
		for( i = 0; i < num_projectiles; i++ ) {
			ang = idMath::Sin( spreadRad * gameLocal.random.RandomFloat() );
			spin = (float)DEG2RAD( 360.0f ) * gameLocal.random.RandomFloat();
			dir = playerViewAxis[ 0 ] + playerViewAxis[ 2 ] * ( ang * idMath::Sin( spin ) ) - playerViewAxis[ 1 ] * ( ang * idMath::Cos( spin ) );
			dir.Normalize();

			//Dentonmod Dynamix
			if ( barrelLaunch || tracer || beam ) { // Do not execute this part unless projectile is barrel launched or has a tracer effect.

				gameLocal.clip.Translation( tr, view_pos, view_pos + dir * 4096.0f, NULL, mat3_identity, MASK_SHOT_RENDERMODEL, owner );

				//traceDist = (tr.endpos - view_pos).Length();
				traceDist = (tr.endpos - view_pos).LengthSqr(); // This is faster
	
				if ( traceDist > muzzleDistFromView ) { // make sure the muzzle is not to close to walls etc
					if ( barrelLaunch ) {
						dir = tr.endpos - muzzle_pos;    
						dir.Normalize();
					}
					else if ( tracer ) { 
						muzzleDir = tr.endpos - muzzle_pos;
						//	muzzleToTargetDist = muzzleDir.Length();
						muzzleToTargetDist = muzzleDir.LengthSqr(); // This is faster
						muzzleDir.Normalize();
					}
				} else { 
				if ( tracer || beam ) {	// Dont do tracers when weapon is too close to walls, objects etc.
					tracer = false;
					beam = false;
				}
			}
		}
		//Dynamix end

			if ( projectileEnt ) {
				ent = projectileEnt;
				ent->Show();
				ent->Unbind();
				projectileEnt = NULL;
			} else {
				gameLocal.SpawnEntityDef( projectileDict, &ent, false );
			}

			if ( !ent || !ent->IsType( idProjectile::Type ) ) {
				const char *projectileName = weaponDef->dict.GetString( "def_projectile" );
				gameLocal.Error( "'%s' is not an idProjectile", projectileName );
			}

			if ( projectileDict.GetBool( "net_instanthit" ) ) {
				// don't synchronize this on top of the already predicted effect
				ent->fl.networkSync = false;
			}

			proj = static_cast<idProjectile *>(ent);
			proj->Create( owner, muzzleOrigin, dir );

			projBounds = proj->GetPhysics()->GetBounds().Rotate( proj->GetPhysics()->GetAxis() );

			// make sure the projectile starts inside the bounding box of the owner
			if ( i == 0 ) {
				muzzle_pos = muzzleOrigin + playerViewAxis[ 0 ] * 2.0f;
				// DG: sometimes the assertion in idBounds::operator-(const idBounds&) triggers
				//     (would get bounding box with negative volume)
				//     => check that before doing ownerBounds - projBounds (equivalent to the check in the assertion)
				idVec3 obDiff = ownerBounds[1] - ownerBounds[0];
				idVec3 pbDiff = projBounds[1] - projBounds[0];
				bool boundsSubLegal =  obDiff.x > pbDiff.x && obDiff.y > pbDiff.y && obDiff.z > pbDiff.z;
				if ( boundsSubLegal && ( ownerBounds - projBounds ).RayIntersection( muzzle_pos, playerViewAxis[0], distance ) ) {
					start = muzzle_pos + distance * playerViewAxis[0];
				} else {
					start = ownerBounds.GetCenter();
				}
				gameLocal.clip.Translation( tr, start, muzzle_pos, proj->GetPhysics()->GetClipModel(), proj->GetPhysics()->GetClipModel()->GetAxis(), MASK_SHOT_RENDERMODEL, owner );
				muzzle_pos = tr.endpos;
			}

			//Dynamix
			if ( tracer ) { 
				/*		if ( traceDist <= muzzleToTargetDist ) // Ideally, this should never happen
					gameLocal.Printf ( " Unpredicted traceDistance in jkSimpleForcePower::Event_LaunchProjectiles " );
				*/
				bool beamTracer = (projectileDict.GetString( "beam_skin", NULL ) != NULL);
				float tracer_speed = 0;
				if ( tracer_speed != 0.0f ) {
					if ( beamTracer ) { // Check whether it's a beamTracer
						proj->setTracerEffect( new dnBeamSpeedTracer(proj, tracer_speed, muzzleToTargetDist * idMath::RSqrt( muzzleToTargetDist ), muzzle_pos, muzzleDir.ToMat3()) );
					} else {
						proj->setTracerEffect( new dnSpeedTracer(proj, tracer_speed, muzzleToTargetDist * idMath::RSqrt( muzzleToTargetDist ), muzzle_pos, muzzleDir.ToMat3()) );
					}
				} else {
					if ( beamTracer ) {
						proj->setTracerEffect( new dnBeamTracer(proj, traceDist/muzzleToTargetDist, view_pos, muzzle_pos, muzzleDir.ToMat3()) );
					} else {
						proj->setTracerEffect( new dnTracer(proj, traceDist/muzzleToTargetDist, view_pos, muzzle_pos, muzzleDir.ToMat3()) );
					}
				}
			} else if( beam ) {
				gameLocal.DPrintf("Else if Beam");
				proj->setTracerEffect( new dnRailBeam(proj, muzzle_pos) );
			}
				//Dynamix End


			proj->Launch( muzzle_pos, dir, pushVelocity, fuseOffset, launchPower, dmgPower );
		}

		// toss the brass
		PostEventMS( &EV_Weapon_EjectBrass, brassDelay );
	}

	// add the light for the muzzleflash
	if ( !lightOn ) {
		MuzzleFlashLight();
	}

	owner->WeaponFireFeedback( &weaponDef->dict );

	// reset muzzle smoke
	weaponSmokeStartTime = gameLocal.realClientTime;
}

/* Dynamix This isn't a good way to do this but it'll work so whatever
/*
================
jkSimpleForcePower::Event_LaunchProjectiles_Alt
================
*/
void jkSimpleForcePower::Event_LaunchProjectiles_Alt( int num_projectiles, float spread, float fuseOffset, float launchPower, float dmgPower ) {
	idProjectile	*proj;
	idEntity		*ent;
	int				i;
	idVec3			dir;
	float			ang;
	float			spin;
	float			distance;
	trace_t			tr;
	idVec3			start;
	idVec3			muzzle_pos;
	idBounds		ownerBounds, projBounds;

	if ( IsHidden() ) {
		return;
	}

	if ( !projectileDict.GetNumKeyVals() ) {
		const char *classname = weaponDef->dict.GetString( "classname" );
		gameLocal.Warning( "No projectile defined on '%s'", classname );
		return;
	}

	// avoid all ammo considerations on an MP client
	if ( !gameLocal.isClient ) {

		// check if we're out of ammo or the clip is empty
		int ammoAvail = owner->inventory.HasAmmo( ammoType, ammoRequired );
		if ( !ammoAvail || ( ( clipSize != 0 ) && ( ammoClip <= 0 ) ) ) {
			return;
		}

		// if this is a power ammo weapon ( currently only the bfg ) then make sure
		// we only fire as much power as available in each clip
		if ( powerAmmo ) {
			// power comes in as a float from zero to max
			// if we use this on more than the bfg will need to define the max
			// in the .def as opposed to just in the script so proper calcs
			// can be done here.
			dmgPower = ( int )dmgPower + 1;
			if ( dmgPower > ammoClip ) {
				dmgPower = ammoClip;
			}
		}

		owner->inventory.UseAmmo( ammoType, ( powerAmmo ) ? dmgPower : ammoRequired );
		if ( clipSize && ammoRequired ) {
			ammoClip -= powerAmmo ? dmgPower : 1;
		}

	}

	if ( !silent_fire ) {
		// wake up nearby monsters
		gameLocal.AlertAI( owner );
	}

	// set the shader parm to the time of last projectile firing,
	// which the gun material shaders can reference for single shot barrel glows, etc
	renderEntity.shaderParms[ SHADERPARM_DIVERSITY ]	= gameLocal.random.CRandomFloat();
	renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ]	= -MS2SEC( gameLocal.realClientTime );

	if ( worldModel.GetEntity() ) {
		worldModel.GetEntity()->SetShaderParm( SHADERPARM_DIVERSITY, renderEntity.shaderParms[ SHADERPARM_DIVERSITY ] );
		worldModel.GetEntity()->SetShaderParm( SHADERPARM_TIMEOFFSET, renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] );
	}

	// calculate the muzzle position
	if ( barrelJointView != INVALID_JOINT && projectileDict.GetBool( "launchFromBarrel" ) ) {
		// there is an explicit joint for the muzzle
		GetGlobalJointTransform( true, barrelJointView, muzzleOrigin, muzzleAxis );
	} else {
		// go straight out of the view
		muzzleOrigin = playerViewOrigin;
		muzzleAxis = playerViewAxis;
	}

	// add some to the kick time, incrementally moving repeat firing weapons back
	if ( kick_endtime < gameLocal.realClientTime ) {
		kick_endtime = gameLocal.realClientTime;
	}
	kick_endtime += muzzle_kick_time;
	if ( kick_endtime > gameLocal.realClientTime + muzzle_kick_maxtime ) {
		kick_endtime = gameLocal.realClientTime + muzzle_kick_maxtime;
	}

	if ( gameLocal.isClient ) {

		// predict instant hit projectiles
		if ( projectileDict.GetBool( "net_instanthit" ) ) {
			float spreadRad = DEG2RAD( spread );
			muzzle_pos = muzzleOrigin + playerViewAxis[ 0 ] * 2.0f;
			for( i = 0; i < num_projectiles; i++ ) {
				ang = idMath::Sin( spreadRad * gameLocal.random.RandomFloat() );
				spin = (float)DEG2RAD( 360.0f ) * gameLocal.random.RandomFloat();
				dir = playerViewAxis[ 0 ] + playerViewAxis[ 2 ] * ( ang * idMath::Sin( spin ) ) - playerViewAxis[ 1 ] * ( ang * idMath::Cos( spin ) );
				dir.Normalize();
				gameLocal.clip.Translation( tr, muzzle_pos, muzzle_pos + dir * 4096.0f, NULL, mat3_identity, MASK_SHOT_RENDERMODEL, owner );
				if ( tr.fraction < 1.0f ) {
					idProjectile::ClientPredictionCollide( this, projectileDict, tr, vec3_origin, true );
				}
			}
		}

	} else {

		ownerBounds = owner->GetPhysics()->GetAbsBounds();

		owner->AddProjectilesFired( num_projectiles );

		float spreadRad = DEG2RAD( spread );
		for( i = 0; i < num_projectiles; i++ ) {
			ang = idMath::Sin( spreadRad * gameLocal.random.RandomFloat() );
			spin = (float)DEG2RAD( 360.0f ) * gameLocal.random.RandomFloat();
			dir = playerViewAxis[ 0 ] + playerViewAxis[ 2 ] * ( ang * idMath::Sin( spin ) ) - playerViewAxis[ 1 ] * ( ang * idMath::Cos( spin ) );
			dir.Normalize();

			if ( projectileEnt ) {
				ent = projectileEnt;
				ent->Show();
				ent->Unbind();
				projectileEnt = NULL;
			} else {
				gameLocal.SpawnEntityDef( projectileDictAlt, &ent, false );
			}

			if ( !ent || !ent->IsType( idProjectile::Type ) ) {
				const char *projectileName = weaponDef->dict.GetString( "def_projectile_alt" );
				gameLocal.Error( "'%s' is not an idProjectile", projectileName );
			}

			if ( projectileDict.GetBool( "net_instanthit" ) ) {
				// don't synchronize this on top of the already predicted effect
				ent->fl.networkSync = false;
			}

			proj = static_cast<idProjectile *>(ent);
			proj->Create( owner, muzzleOrigin, dir );

			projBounds = proj->GetPhysics()->GetBounds().Rotate( proj->GetPhysics()->GetAxis() );

			// make sure the projectile starts inside the bounding box of the owner
			if ( i == 0 ) {
				muzzle_pos = muzzleOrigin + playerViewAxis[ 0 ] * 2.0f;
				// DG: sometimes the assertion in idBounds::operator-(const idBounds&) triggers
				//     (would get bounding box with negative volume)
				//     => check that before doing ownerBounds - projBounds (equivalent to the check in the assertion)
				idVec3 obDiff = ownerBounds[1] - ownerBounds[0];
				idVec3 pbDiff = projBounds[1] - projBounds[0];
				bool boundsSubLegal =  obDiff.x > pbDiff.x && obDiff.y > pbDiff.y && obDiff.z > pbDiff.z;
				if ( boundsSubLegal && ( ownerBounds - projBounds ).RayIntersection( muzzle_pos, playerViewAxis[0], distance ) ) {
					start = muzzle_pos + distance * playerViewAxis[0];
				} else {
					start = ownerBounds.GetCenter();
				}
				gameLocal.clip.Translation( tr, start, muzzle_pos, proj->GetPhysics()->GetClipModel(), proj->GetPhysics()->GetClipModel()->GetAxis(), MASK_SHOT_RENDERMODEL, owner );
				muzzle_pos = tr.endpos;
			}

			proj->Launch( muzzle_pos, dir, pushVelocity, fuseOffset, launchPower, dmgPower );
		}

		// toss the brass
		PostEventMS( &EV_Weapon_EjectBrass, brassDelay );
	}

	// add the light for the muzzleflash
	if ( !lightOn ) {
		MuzzleFlashLight();
	}

	owner->WeaponFireFeedback( &weaponDef->dict );

	// reset muzzle smoke
	weaponSmokeStartTime = gameLocal.realClientTime;
}


/*
=====================
jkSimpleForcePower::Event_GetWorldModel
=====================
*/
void jkSimpleForcePower::Event_GetWorldModel( void ) {
	idThread::ReturnEntity( worldModel.GetEntity() );
}

/*
=====================
jkSimpleForcePower::Event_AllowDrop
=====================
*/
void jkSimpleForcePower::Event_AllowDrop( int allow ) {
	if ( allow ) {
		allowDrop = true;
	} else {
		allowDrop = false;
	}
}

/*
================
jkSimpleForcePower::Event_EjectBrass

Toss a shell model out from the breach if the bone is present
================
*/
void jkSimpleForcePower::Event_EjectBrass( void ) {
	if ( !g_showBrass.GetBool() || !owner->CanShowWeaponViewmodel() ) {
		return;
	}

	if ( ejectJointView == INVALID_JOINT || !brassDict.GetNumKeyVals() ) {
		return;
	}

	if ( gameLocal.isClient ) {
		return;
	}

	idMat3 axis;
	idVec3 origin, linear_velocity, angular_velocity;
	idEntity *ent;

	if ( !GetGlobalJointTransform( true, ejectJointView, origin, axis ) ) {
		return;
	}

	gameLocal.SpawnEntityDef( brassDict, &ent, false );
	if ( !ent || !ent->IsType( idDebris::Type ) ) {
		gameLocal.Error( "'%s' is not an idDebris", weaponDef ? weaponDef->dict.GetString( "def_ejectBrass" ) : "def_ejectBrass" );
	}
	idDebris *debris = static_cast<idDebris *>(ent);
	debris->Create( owner, origin, axis );
	debris->Launch();

	linear_velocity = 40 * ( playerViewAxis[0] + playerViewAxis[1] + playerViewAxis[2] );
	angular_velocity.Set( 10 * gameLocal.random.CRandomFloat(), 10 * gameLocal.random.CRandomFloat(), 10 * gameLocal.random.CRandomFloat() );

	debris->GetPhysics()->SetLinearVelocity( linear_velocity );
	debris->GetPhysics()->SetAngularVelocity( angular_velocity );
}

/*
===============
jkSimpleForcePower::Event_IsInvisible
===============
*/
void jkSimpleForcePower::Event_IsInvisible( void ) {
	if ( !owner ) {
		idThread::ReturnFloat( 0 );
		return;
	}
	idThread::ReturnFloat( owner->PowerUpActive( INVISIBILITY ) ? 1 : 0 );
}

/*
===============
jkSimpleForcePower::ClientPredictionThink
===============
*/
void jkSimpleForcePower::ClientPredictionThink( void ) {
	UpdateAnimation();
}

/*
===============
jkSimpleForcePower::Event_DoForcePower
===============
*/
void jkSimpleForcePower::Event_DoForcePower( void ) {
}

/*
===============
jkSimpleForcePower::Event_DoForcePower
===============
*/
void jkSimpleForcePower::Event_UseForcePoints( float amount, int alignment, int type ) {
	owner->UseForcePoints(amount, alignment, type);
}

/*
================
jkForcePower::BeginAttack
================
*/
void jkForcePower::BeginAttack( void ) {
	if ( status != FP_OUTOFAMMO ) {
		lastAttack = gameLocal.time;
	}

	if ( !isLinked ) {
		return;
	}

	if ( !WEAPON_ATTACK ) {
		if ( sndHum ) {
			StopSound( SND_CHANNEL_BODY, false );
		}
	}
	WEAPON_ATTACK = true;

	int forceLevel = 1;
	gameLocal.DPrintf("Force.cpp BeginAttack %d\n", forceLevel);
	owner->inventory.GivePowerUp( owner, BERSERK, 30000);
}