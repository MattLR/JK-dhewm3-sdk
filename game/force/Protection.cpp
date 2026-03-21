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

CLASS_DECLARATION( jkSimpleForcePower, jkForceProtection )
END_CLASS


/*
================
jkForceProtection::Event_DoForcePower
================
*/
void jkForceProtection::Event_DoForcePower( void ) {
	gameLocal.DPrintf ("Event_DoForcePower Protection\n");

	switch (owner->forceLevels[0]) {
		case 0:
			gameLocal.DPrintf ("DoForcePower Protection level 0\n");
				if ( owner->UseForcePoints(50) ) {
					struct statusEffect s = {1, 0.75, gameLocal.time + 20000};
					owner->statusEffects.Append(s);
					owner->hud->SetStateString( "activeForceBG", Icon() );
				}
			break;
		case 1:
			gameLocal.DPrintf ("DoForcePower Protection level 1\n");
				if ( owner->UseForcePoints(50) ) {
					struct statusEffect s = {1, 0.5, gameLocal.time + 20000};
					owner->statusEffects.Append(s);
					owner->hud->SetStateString( "activeForceBG", Icon() );
				}
			break;
		case 3:
			gameLocal.DPrintf ("DoForcePower Protection level 3\n");
				if ( owner->UseForcePoints(50) ) {
					struct statusEffect s = {1, 0.25, gameLocal.time + 20000};
					owner->statusEffects.Append(s);
					owner->hud->SetStateString( "activeForceBG", Icon() );
				}
			break;
		default:
			gameLocal.DPrintf ("DoForcePower Protection default\n");
	}
}