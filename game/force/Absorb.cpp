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


CLASS_DECLARATION( jkSimpleForcePower, jkForceAbsorb )
END_CLASS


/*
================
jkForceAbsorb::Event_DoForcePower
================
*/
void jkForceAbsorb::Event_DoForcePower( void ) {
	gameLocal.DPrintf ("Event_DoForcePower Absorb\n");
	switch (owner->forceLevels[0]) {
		case 0:
			gameLocal.DPrintf ("DoForcePower Absorb level 0\n");
				if ( owner->UseForcePoints(50) ) {
					struct statusEffect s = {2, 0.75, gameLocal.time + 20000};
					owner->statusEffects.Append(s);
					owner->hud->SetStateString( "activeForceBG", Icon() );
				}
			break;
		case 1:
			gameLocal.DPrintf ("DoForcePower Absorb level 2\n");
				if ( owner->UseForcePoints(50) ) {
					struct statusEffect s = {2, 0.5, gameLocal.time + 20000};
					owner->statusEffects.Append(s);
					owner->hud->SetStateString( "activeForceBG", Icon() );
				}
			break;
		case 3:
			gameLocal.DPrintf ("DoForcePower Absorb level 3\n");
				if ( owner->UseForcePoints(50) ) {
					struct statusEffect s = {2, 0.25, gameLocal.time + 20000};
					owner->statusEffects.Append(s);
					owner->hud->SetStateString( "activeForceBG", Icon() );
				}
			break;
		default:
			gameLocal.DPrintf ("DoForcePower Absorb default\n");
	}
}