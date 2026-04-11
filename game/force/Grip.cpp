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

CLASS_DECLARATION( jkSimpleForcePower, jkForceGrip )
END_CLASS


/*
================
jkForceGrip::Event_DoForcePower
================
*/
void jkForceGrip::Event_DoForcePower( void ) {
	gameLocal.DPrintf ("Event_DoForcePower Grip\n");

	int i, listedEntities;
	idEntity *entityList[ MAX_GENTITIES ];
	int pushRadius = 400;

	int forceLevel = 1;

	listedEntities = gameLocal.EntitiesWithinRadius( GetPhysics()->GetOrigin(), pushRadius, entityList, MAX_GENTITIES );

	gameLocal.Printf("Grip\n");
	idVec3 dir( 0.0f, 40.0f, 0.0f );
	// Collect entities within radius. Doom 3 doesn't have a direct RadiusList helper in public SDK,
	// so we use clip.Contents or iterate entities and test distance (simple but works for small maps).

	for( i = 0; i < listedEntities; i++ ) {
			idEntity *ent = entityList[ i ];
			if ( !ent || ent == owner ) continue;
			if ( ent->IsHidden() ) continue;
			if ( ent->IsType( idWorldspawn::Type) ) continue;
			if ( ent->IsType( idLight::Type) ) continue;
			ent->GetName();
			gameLocal.DPrintf(ent->GetName());
			// basic distance check

			ent->ForcePowerResponse(this, this, dir, "grip", 1, INVALID_JOINT);
			//idMover_Binary *binary = dynamic_cast<idMover_Binary*>( ent );
			// Handle NPCs/enemies (idAI or subclasses)
			idAI *ai = dynamic_cast<idAI*>( ent );
			if ( ent->IsType( idAI::Type ) ) {

					}

		}
}