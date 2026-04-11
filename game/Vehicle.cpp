#include "sys/platform.h"
#include "renderer/ModelManager.h"

#include "Fx.h"

#include "Vehicle.h"


/*
===============================================================================

  jkRBVehicleTest

===============================================================================
*/

CLASS_DECLARATION( idAnimatedEntity, jkRBVehicleTest )
END_CLASS

/*
================
jkRBVehicleTest::jkRBVehicleTest
================
*/
jkRBVehicleTest::jkRBVehicleTest( void ) {
	player				= NULL;
	eyesJoint			= INVALID_JOINT;
	//steeringWheelJoint	= INVALID_JOINT;
	//wheelRadius			= 0.0f;
	steerAngle			= 0.0f;
	steerSpeed			= 0.0f;
	//dustSmoke			= NULL;
    modelDefHandle			= -1;
}

/*
================
jkRBVehicleTest::jkRBVehicleTest
================
*/
jkRBVehicleTest::~jkRBVehicleTest( void ) {
    
    /*
    combatModel = NULL;
	combatModelContents = 0;
	nextSoundTime = 0;
	spawnOrigin.Zero();
	spawnAxis.Identity();
	//idleAnim		= 0;
	player				= NULL;
	eyesJoint			= INVALID_JOINT;
	//steeringWheelJoint	= INVALID_JOINT;
	//wheelRadius			= 0.0f;
	steerAngle			= 0.0f;
	steerSpeed			= 0.0f;
	//dustSmoke			= NULL;
    */
}

/*
================
jkRBVehicleTest::Spawn
================
*/
void jkRBVehicleTest::Spawn( void ) {
	const char *eyesJointName = spawnArgs.GetString( "eyesJoint", "eyes" );
    idTraceModel trm;
    idStr clipModelName;
	


    float density, friction, bouncyness, mass;
    //idleAnim = animator.GetAnim( "idle" );
   // physicsObj.SetSelf( this );
	
    // check if a clip model is set
	spawnArgs.GetString( "clipmodel", "", clipModelName );
	if ( !clipModelName[0] ) {
		clipModelName = spawnArgs.GetString( "model" );		// use the visual model
	}

    // get rigid body properties
	spawnArgs.GetFloat( "density", "0.005", density );
	density = idMath::ClampFloat( 0.001f, 1000.0f, density );
	spawnArgs.GetFloat( "friction", "0.05", friction );
	friction = idMath::ClampFloat( 0.0f, 1.0f, friction );
	spawnArgs.GetFloat( "bouncyness", "0.2", bouncyness );

    // setup the physics
	physicsObj.SetSelf( this );
    //SetClipModel();
    //physicsObj.SetClipModel( new idClipModel( GetPhysics()->GetClipModel() ), 1.0f );
    //physicsObj.SetClipModel( new idClipModel( trm ), density );
    physicsObj.SetClipModel( new idClipModel( GetPhysics()->GetClipModel() ), density );
   // physicsObj.SetClipModel( new idClipModel( trm ), density );
	//physicsObj.SetClipModel( new idClipModel( trm ), density );
	//physicsObj.GetClipModel()->SetMaterial( GetRenderModelMaterial() );
	physicsObj.SetOrigin( GetPhysics()->GetOrigin() );
	physicsObj.SetAxis( GetPhysics()->GetAxis() );
	physicsObj.SetBouncyness( bouncyness );
	physicsObj.SetFriction( 0.6f, 1.0f, 0.01f );
    physicsObj.SetMass( spawnArgs.GetFloat( "mass", "100" ) );
	physicsObj.SetGravity( gameLocal.GetGravity() );
	physicsObj.SetContents( CONTENTS_SOLID );
	physicsObj.SetClipMask( MASK_SOLID );

    SetPhysics( &physicsObj );

    
    //Work out if this should be before or after physics Dynamix FIXME
    SetCombatModel();

	//fl.takedamage = true;
    
	if ( !eyesJointName[0] ) {
		gameLocal.Error( "jkRBVehicleTest '%s' no eyes joint specified", name.c_str() );
	}
	eyesJoint = animator.GetJointHandle( eyesJointName );
        

	//spawnArgs.GetFloat( "wheelRadius", "20", wheelRadius );
	spawnArgs.GetFloat( "steerSpeed", "5", steerSpeed );

	player = NULL;
	steerAngle = 0.0f;

    animator.CycleAnim ( ANIMCHANNEL_ALL, animator.GetAnim( spawnArgs.GetString( "anim", "idle" ) ), gameLocal.time, 0 );	
    
    //Do this at some point
   // renderEntity.bounds.AddPoint ( (vec-renderEntity.origin) * GetPhysics()->GetAxis().Transpose() );

    BecomeActive( TH_THINK );

	//const char *smokeName = spawnArgs.GetString( "smoke_vehicle_dust", "muzzlesmoke" );
	//if ( *smokeName != '\0' ) {
	//	dustSmoke = static_cast<const idDeclParticle *>( declManager->FindType( DECL_PARTICLE, smokeName ) );
	//}
}

/*
================
jkRBVehicleTest::Use
================
*/
void jkRBVehicleTest::Use( idPlayer *other ) {
	idVec3 origin;
	idMat3 axis;

	if ( player ) {
		if ( player == other ) {
            gameLocal.DPrintf("Getting out");
			other->Unbind();
			player = NULL;

			//af.GetPhysics()->SetComeToRest( true );
		}
	}
	else {
        gameLocal.DPrintf("Getting in");
		player = other;
		//animator.GetJointTransform( eyesJoint, gameLocal.time, origin, axis );
		//origin = renderEntity.origin + origin * renderEntity.axis;
        origin = GetPhysics()->GetOrigin();
		player->GetPhysics()->SetOrigin( origin );
		player->BindToBody( this, 0, true );

		//af.GetPhysics()->SetComeToRest( false );
		//af.GetPhysics()->Activate();
	}
}

/*
================
jkRBVehicleTest::GetSteerAngle
================
*/
float jkRBVehicleTest::GetSteerAngle( void ) {
	float idealSteerAngle, angleDelta;

	idealSteerAngle = player->usercmd.rightmove * ( 30.0f / 128.0f );
	angleDelta = idealSteerAngle - steerAngle;

	if ( angleDelta > steerSpeed ) {
		steerAngle += steerSpeed;
	} else if ( angleDelta < -steerSpeed ) {
		steerAngle -= steerSpeed;
	} else {
		steerAngle = idealSteerAngle;
	}

	return steerAngle;
}

/*
================
jkRBVehicleTest::Think
================
*/
void jkRBVehicleTest::Think( void ) {
	int i;
	float force = 200000.0f, velocity = 0.0f, steerAngle = 0.0f;
	idVec3 origin;
	idMat3 axis;
	idRotation wheelRotation, steerRotation;


	if ( thinkFlags & TH_THINK ) {

		if ( player ) {
         idAngles target = player->viewAngles;
        target.pitch = 0;
        target.roll  = 0;

        idAngles current = GetPhysics()->GetAxis().ToAngles();
        idAngles delta = target - current;

            float turnSpeed = 5.0f;

            current += delta * turnSpeed * 0.01f;

            //SetAngles( current );
    
			// capture the input from a player
			//velocity = g_vehicleVelocity.GetFloat();
            velocity = 200;
			if ( player->usercmd.forwardmove < 0 ) {
				force = -force;
			}
            if (player->usercmd.forwardmove) {

            idVec3 forward = GetPhysics()->GetAxis()[0]; // forward (X axis)
           // idVec3 forward = player->viewAngles.ToForward();
            idVec3 impulse = forward * force;
			//force = idMath::Fabs( player->usercmd.forwardmove * g_vehicleForce.GetFloat() ) * (1.0f / 128.0f);
			force = idMath::Fabs( player->usercmd.forwardmove * 10 ) * (1.0f / 128.0f);
            steerAngle = GetSteerAngle();
            idVec3 raiseVector( 0, -100000, 0);
           //ApplyImpulse(this, 0, this->GetPhysics()->GetOrigin(), impulse);
            AddForce(this, 0, this->GetPhysics()->GetOrigin(), impulse);
            }
			physicsObj.GetAxis();
			//This needs actual physics changes I think but fine for testing Dynamix FIXME
			//Can just spin renderentity, I think that's what they do in QW
			player->GetPhysics()->SetAxis( physicsObj.GetAxis() );
		}

		// update suspension with latest cvar settings

		// run the physics
		RunPhysics();

/*
		// spawn dust particle effects
		if ( force != 0.0f && !( gameLocal.framenum & 7 ) ) {
			int numContacts;
			idAFConstraint_Contact *contacts[2];
			for ( i = 0; i < 4; i++ ) {
				numContacts = af.GetPhysics()->GetBodyContactConstraints( wheels[i]->GetClipModel()->GetId(), contacts, 2 );
				for ( int j = 0; j < numContacts; j++ ) {
					gameLocal.smokeParticles->EmitSmoke( dustSmoke, gameLocal.time, gameLocal.random.RandomFloat(), contacts[j]->GetContact().point, contacts[j]->GetContact().normal.ToMat3() );
				}
			}
		}
*/
	}

	UpdateAnimation();
	if ( thinkFlags & TH_UPDATEVISUALS ) {
		Present();
		LinkCombat();
	}
    //idAnimatedEntity::Think();
}

/*
================
jkRBVehicleTest::jkRBVehicleTest
================
*/
void jkRBVehicleTest::Save( idSaveGame *savefile ) const {
    return;
}

/*
================
jkRBVehicleTest::jkRBVehicleTest
================
*/
void jkRBVehicleTest::Restore( idRestoreGame *savefile ) {
    return;
}

/*
================
jkRBVehicleTest::Collide
================
*/
bool jkRBVehicleTest::Collide( const trace_t &collision, const idVec3 &velocity ) {
    return false;
}

/*
================
jkRBVehicleTest::FreeModelDef
================
*/
void jkRBVehicleTest::FreeModelDef( void ) {
    return;
	UnlinkCombat();
	idEntity::FreeModelDef();
}

void jkRBVehicleTest::Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location ) {
    return;
}

/*
================
jkRBVehicleTest::SetCombatModel
================
*/
void jkRBVehicleTest::SetCombatModel( void ) {
    return;
	if ( combatModel ) {
		combatModel->Unlink();
		combatModel->LoadModel( modelDefHandle );
	} else {
		combatModel = new idClipModel( modelDefHandle );
	}
}

/*
================
jkRBVehicleTest::GetCombatModel
================
*/
idClipModel *jkRBVehicleTest::GetCombatModel( void ) const {
	return combatModel;
}

/*
================
jkRBVehicleTest::SetCombatContents
================
*/
void jkRBVehicleTest::SetCombatContents( bool enable ) {
    return;
	assert( combatModel );
	if ( enable && combatModelContents ) {
		assert( !combatModel->GetContents() );
		combatModel->SetContents( combatModelContents );
		combatModelContents = 0;
	} else if ( !enable && combatModel->GetContents() ) {
		assert( !combatModelContents );
		combatModelContents = combatModel->GetContents();
		combatModel->SetContents( 0 );
	}
}

/*
================
jkRBVehicleTest::LinkCombat
================
*/
void jkRBVehicleTest::LinkCombat( void ) {
    return;
	if ( fl.hidden ) {
		return;
	}
	if ( combatModel ) {
		combatModel->Link( gameLocal.clip, this, 0, renderEntity.origin, renderEntity.axis, modelDefHandle );
	}
}

/*
================
jkRBVehicleTest::UnlinkCombat
================
*/
void jkRBVehicleTest::UnlinkCombat( void ) {
    return;
	if ( combatModel ) {
		combatModel->Unlink();
	}
}

/*
================
jkRBVehicleTest::SetClipModel
================
*/
void jkRBVehicleTest::SetClipModel( void ) {
	const char *temp;
    idTraceModel trm;
    idStr clipModelName;
	idClipModel *clipModel = NULL;

	// check if a clipmodel key/value pair is set
	if ( spawnArgs.GetString( "clipmodel", "", clipModelName ) ) {
		if ( idClipModel::CheckModel( clipModelName ) ) {
			clipModel = new idClipModel( clipModelName );
		}
	}

    if ( !temp[0] ) {
		clipModelName = spawnArgs.GetString( "model" );		// use the visual model
	}

	if ( !collisionModelManager->TrmFromModel( clipModelName, trm ) ) {
		gameLocal.Error( "idMoveable '%s': cannot load collision model %s", name.c_str(), clipModelName.c_str() );
		return;
	}
    /*
    if ( !clipModel ) {
			idVec3 size;
			idBounds bounds;
			bool setClipModel = false;

			if ( spawnArgs.GetVector( "mins", NULL, bounds[0] ) &&
				spawnArgs.GetVector( "maxs", NULL, bounds[1] ) ) {
				setClipModel = true;
				if ( bounds[0][0] > bounds[1][0] || bounds[0][1] > bounds[1][1] || bounds[0][2] > bounds[1][2] ) {
					gameLocal.Error( "Invalid bounds '%s'-'%s' on entity '%s'", bounds[0].ToString(), bounds[1].ToString(), name.c_str() );
				}
			} else if ( spawnArgs.GetVector( "size", NULL, size ) ) {
				if ( ( size.x < 0.0f ) || ( size.y < 0.0f ) || ( size.z < 0.0f ) ) {
					gameLocal.Error( "Invalid size '%s' on entity '%s'", size.ToString(), name.c_str() );
				}
				bounds[0].Set( size.x * -0.5f, size.y * -0.5f, 0.0f );
				bounds[1].Set( size.x * 0.5f, size.y * 0.5f, size.z );
				setClipModel = true;
			}

        if ( setClipModel ) {
				int numSides;
				idTraceModel trm;

				if ( spawnArgs.GetInt( "cylinder", "0", numSides ) && numSides > 0 ) {
					trm.SetupCylinder( bounds, numSides < 3 ? 3 : numSides );
				} else if ( spawnArgs.GetInt( "cone", "0", numSides ) && numSides > 0 ) {
					trm.SetupCone( bounds, numSides < 3 ? 3 : numSides );
				} else if ( spawnArgs.GetInt( "oct", "1", numSides ) ) { // Dynamix, bullets aren't always cones :) octahedron and dodecahedron are fine I guess
					trm.SetupOctahedron( bounds );
				} else if ( spawnArgs.GetInt( "dodeca", "1", numSides ) ) { // Dynamix
					trm.SetupDodecahedron( bounds );
				} else {
					trm.SetupBox( bounds );
				}
				clipModel = new idClipModel( trm );
			}

        }
        */
    float density, friction, bouncyness, mass;
    //idleAnim = animator.GetAnim( "idle" );
	
        // get rigid body properties
	spawnArgs.GetFloat( "density", "0.005", density );
	density = idMath::ClampFloat( 0.001f, 1000.0f, density );
	spawnArgs.GetFloat( "friction", "0.05", friction );
	friction = idMath::ClampFloat( 0.0f, 1.0f, friction );
	spawnArgs.GetFloat( "bouncyness", "0.2", bouncyness );
    // setup the physics
	physicsObj.SetSelf( this );
    //SetClipModel();
    physicsObj.SetClipModel( new idClipModel( trm ), density );
	//physicsObj.SetClipModel( new idClipModel( trm ), density );
	//physicsObj.GetClipModel()->SetMaterial( GetRenderModelMaterial() );
	physicsObj.SetOrigin( GetPhysics()->GetOrigin() );
	physicsObj.SetAxis( GetPhysics()->GetAxis() );
	physicsObj.SetBouncyness( bouncyness );
	physicsObj.SetFriction( 0.6f, 0.6f, friction );
	physicsObj.SetGravity( gameLocal.GetGravity() );
	physicsObj.SetContents( CONTENTS_SOLID );
	physicsObj.SetClipMask( MASK_SOLID | CONTENTS_BODY | CONTENTS_CORPSE | CONTENTS_MOVEABLECLIP );
    

}