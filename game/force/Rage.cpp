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

CLASS_DECLARATION( jkSimpleForcePower, jkForceRage )
END_CLASS

/*
================
jkForceRage::Event_DoForcePower
================
*/
void jkForceRage::Event_DoForcePower( void ) {
	gameLocal.DPrintf ("Event_DoForcePower Rage\n");

	// Make an enum after I'm done with this
	int forceLevel = GetForceLevel(1);

	//disabledAnim = "knockdown";
	//rageAnim??
	const function_t *state = GetOwner()->GetScriptFunction( "state_Rage" );

	//owner->AI_DISABLED = TRUE;
	owner->AI_DISABLED_TIME = 1.0f;
	owner->AI_RAGE = TRUE;
	owner->SetState(state);
	owner->rageActive = TRUE;

	struct statusEffect s = {RAGEDOT, 5, gameLocal.time + 10000, 2000, gameLocal.time + 2000 };
	owner->statusEffects.Append(s);
	if ( owner->hud ) {
		owner->hud->SetStateString( "activeForceBG", Icon() );
	}

	// Fix this, it's dumb Dynamix
	owner->PostEventSec(&EV_Player_EndDisable, 1);
	owner->PostEventSec(&EV_Player_EndRage, 10);
}