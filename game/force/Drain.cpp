#include "../Force.h"

#include "sys/platform.h"
#include "framework/DeclEntityDef.h"
#include "framework/DeclSkin.h"

#include "gamesys/SysCvar.h"
#include "ai/AI.h"
#include "Player.h"
#include "Trigger.h"
#include "SmokeParticles.h"
#include "WorldSpawn.h"
#include "Mover.h"

const idEventDef EV_Force_EndDrain( "endDrain" );

CLASS_DECLARATION( jkSimpleForcePower, jkForceDrain )
	EVENT( EV_Force_EndDrain,				jkForceDrain::Event_EndDrain )
END_CLASS


/*
================
jkForceDrain::Event_DoForcePower
================
*/
void jkForceDrain::Event_DoForcePower( void ) {
	gameLocal.DPrintf ("Event_DoForcePower Drain\n");

	int i, listedEntities;
	idEntity *entityList[ MAX_GENTITIES ];
	int pushRadius = 400;

	int forceLevel = 1;

	listedEntities = gameLocal.EntitiesWithinRadius( GetPhysics()->GetOrigin(), pushRadius, entityList, MAX_GENTITIES );


	idVec3 origin = GetPhysics()->GetOrigin();

	for ( i = 0; i < listedEntities; i++ ) {
		idEntity *ent = entityList[ i ];
		if ( !ent || ent == owner ) continue;
		if ( ent->IsHidden() ) continue;
		if ( ent->IsType( idWorldspawn::Type) ) continue;
		if ( ent->IsType( idLight::Type) ) continue;

		idVec3 dir(0, 0, 0);

		const function_t *state = owner->GetScriptFunction( "state_Drain" );

		// Handle NPCs/enemies (idAI or subclasses)
		if ( ent->IsType( idAI::Type ) ) {
			gameLocal.DPrintf(ent->GetName());
			// Check if it's alive I guess
			if ( !ent->ForcePowerResponse( this, this, dir, "drain", 1, INVALID_JOINT ) ) {
				idThread::ReturnInt( false );
				return;
			}
			drainTarget = ent;
			owner->AI_DRAIN = TRUE;
			owner->SetState(state);
			//Set this to min duration + a little buffer and then tick it up
			owner->AI_DISABLED_TIME = 1.0f;
			idThread::ReturnInt( true );
			return;
			}
	}

	idThread::ReturnInt( false );
	return;
}

/*
================
jkForceDrain::Event_DoForceTick
================
*/
void jkForceDrain::Event_DoForceTick( void ) {
	gameLocal.DPrintf ("Event_DoForceTick Drain\n");

	float temp = owner->AI_DISABLED_TIME + 0.2f;
	owner->AI_DISABLED_TIME = temp;
	//Need to change this to deal with ticks somehow

	if (drainTarget == NULL || !drainTarget->ForcePowerResponse( "drainTick", 1) ) {
		idThread::ReturnInt( false );
		return;
	}

	owner->health += 5;
	idThread::ReturnInt( true );
	//return false; Only if no target or no force points?
}

/*
================
jkForceDrain::Event_EndDrain
================
*/
void jkForceDrain::Event_EndDrain( void ) {
	gameLocal.DPrintf("Event_EndDrain");
	owner->AI_DRAIN = FALSE;
	owner->AI_DISABLED_TIME = 0.0f;

	if (drainTarget != NULL) {
	drainTarget->ForcePowerResponse( "drainEnd", 1);
	drainTarget = NULL;
	return;
	}

	drainTarget = NULL;
}