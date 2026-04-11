//----------------------------------------------------------------
// VehicleController.cpp
//
// Copyright 2002-2004 Raven Software
//----------------------------------------------------------------




#include "../Game_local.h"
#include "VehicleController.h"
#include "Vehicle.h"
#include "../Actor.h"

/*
=====================
jkVehicleController::jkVehicleController
=====================
*/
jkVehicleController::jkVehicleController ( void ) {
	mVehicle  = NULL;
	//mDriver   = NULL;
	mPosition = 0;
}

/*
=====================
jkVehicleController::Save
=====================
*/
void jkVehicleController::Save ( idSaveGame *savefile ) const {
	//mVehicle.Save ( savefile );
	savefile->WriteInt ( mPosition );
}

/*
=====================
jkVehicleController::Restore
=====================
*/
void jkVehicleController::Restore ( idRestoreGame *savefile ) {
	//mVehicle.Restore ( savefile );
	savefile->ReadInt ( mPosition );
}

/*
=====================
jkVehicleController::Drive
=====================
*/
bool jkVehicleController::Drive ( jkVehicle* vehicle, idActor* driver ) {
	assert ( vehicle && driver );

	// Flip the vehicle back over?
	
	if ( vehicle->IsFlipped ( ) ) {
		vehicle->AutoRight ( vehicle );
		return false;
	}
		
	// Add the driver to the vehicle and cache the position it was added.
	if ( -1 == (mPosition = vehicle->AddDriver ( 0, driver ) ) ) {
		return false;
	}
	
	mVehicle = vehicle;	

	//twhitaker: for scripted callback events
	//vehicle->OnEnter();
	
	//Fix all these
	if ( driver->IsType( idPlayer::Type ) ) {
		idPlayer * player = static_cast< idPlayer *>( driver );

		/*
		if ( player->GetHud() ) {
			GetHud()->Activate( true, gameLocal.time );
			GetHud()->HandleNamedEvent( "showExitmessage" );
		}
		*/
	}

	return true;
}

/*
=====================
jkVehicleController::Eject
=====================
*/
bool jkVehicleController::Eject ( bool force ) {
	if ( !GetVehicle() ) {
		return true;
	}
	
	/*
	if ( GetDriver()->IsType( idPlayer::Type ) ) {
		idPlayer * player = static_cast< idPlayer *>( GetDriver() );

		
		if ( player->GetHud() ) {
			GetHud()->HandleNamedEvent( "hideExitmessage" );
		}
	}
	*/
	
	if ( mVehicle->RemoveDriver ( GetPosition(), force ) ) {
		//mVehicle->OnExit();
		mVehicle = NULL;
		gameLocal.DPrintf("Removedriver call");
		return true;
	}
	
	return false;
}

/*
=====================
jkVehicleController::FindClearExitPoint
=====================
*/
bool jkVehicleController::FindClearExitPoint( idVec3& origin, idMat3& axis ) const {
	//return;
	return GetVehicle()->FindClearExitPoint( mPosition, origin, axis );
}

/*
=====================
jkVehicleController::KillVehicles
=====================
*/
void jkVehicleController::KillVehicles ( void ) {
	return;
	//KillEntities( idCmdArgs(), jkVehicle::Type ); FIXME1 add it to that header
}

/*
=====================
jkVehicleController::DrawHUD
=====================
*/
void jkVehicleController::DrawHUD ( void ) {
	assert ( mVehicle );

	if ( GetDriver() && GetDriver()->IsType( idPlayer::Type ) ) {
		idPlayer * player = static_cast<idPlayer*>( GetDriver() );
		jkVehicleWeapon * weapon = mVehicle->GetPosition( mPosition )->GetActiveWeapon();

		/* FIX
		if ( weapon ) {
			if ( weapon->CanZoom() && player->IsZoomed() && player == gameLocal.GetLocalPlayer() ) {
				weapon->GetZoomGui()->Redraw( gameLocal.time );			
			}
//            if ( mVehicle->GetHud() ) {
//				mVehicle->GetHud()->SetStateFloat( "tram_heatpct", weapon->GetOverheatPercent());
//	 			mVehicle->GetHud()->SetStateFloat( "tram_overheat", weapon->GetOverheatState());
//			}
		}
		*/

	}

	mVehicle->DrawHUD ( mPosition );
}


/*
=====================
jkVehicleController::Give
=====================
*/
void jkVehicleController::Give ( const char* statname, const char* value ) {
	//if( mVehicle.IsValid() ) {
	if (mVehicle) {
		mVehicle->Give ( statname, value );
	}
}

/*
=====================
jkVehicleController::GetEyePosition
=====================
*/
void jkVehicleController::GetEyePosition ( idVec3& origin, idMat3& axis ) const {
	//if( mVehicle.IsValid() ) {
	if (mVehicle) {
		mVehicle->GetEyePosition ( mPosition, origin, axis );
	}
}

/*
=====================
jkVehicleController::GetDriverPosition
=====================
*/
void jkVehicleController::GetDriverPosition ( idVec3& origin, idMat3& axis ) const {
	//if( mVehicle.IsValid() ) {
	if (mVehicle) {
		mVehicle->GetDriverPosition ( mPosition, origin, axis );
	}
}

/*
=====================
jkVehicleController::GetVehicle
=====================
*/
jkVehicle* jkVehicleController::GetVehicle ( void ) const {
	return mVehicle;
}

idActor* jkVehicleController::GetDriver ( void ) const {
	return (GetVehicle()) ? GetVehicle()->GetPosition(GetPosition())->GetDriver() : NULL;
}

/*
=====================
jkVehicleController::SetInput
=====================
*/
void jkVehicleController::SetInput ( const usercmd_t& cmd, const idAngles &angles ) {
	//if( mVehicle.IsValid() ) {
	if (mVehicle) {
		mVehicle->SetInput ( mPosition, cmd, angles );
	}
}

/*
=====================
jkVehicleController::GetInput
=====================
*/
void jkVehicleController::GetInput( usercmd_t& cmd, idAngles &newAngles ) const {
	//if( mVehicle.IsValid() ) {
	if (mVehicle) {
		mVehicle->GetInput( mPosition, cmd, newAngles );
	}
}

/*
================
jkVehicleController::GetHud
================
*/
idUserInterface* jkVehicleController::GetHud( void ) {
	return (IsDriving()) ? mVehicle->GetHud() : NULL;
}

/*
================
jkVehicleController::GetHud
================
*/
const idUserInterface* jkVehicleController::GetHud( void ) const {
	return (IsDriving()) ? mVehicle->GetHud() : NULL;
}

/*
================
jkVehicleController::WriteToSnapshot
================
*/
void jkVehicleController::WriteToSnapshot ( idBitMsgDelta &msg ) const {
	return;
	//msg.WriteLong ( mPosition );
	//msg.WriteLong ( mVehicle.GetSpawnId ( ) );
}

/*
================
jkVehicleController::ReadFromSnapshot
================
*/
void jkVehicleController::ReadFromSnapshot ( const idBitMsgDelta &msg ) {
	return;
	//mPosition = msg.ReadLong ( );
	//mVehicle.SetSpawnId ( msg.ReadLong ( ) );
}

/*
================
jkVehicleController::UpdateCursorGUI
================
*/
void jkVehicleController::UpdateCursorGUI ( idUserInterface* ui ) {
	assert ( mVehicle );
	mVehicle->UpdateCursorGUI ( mPosition, ui );
}

/*
================
jkVehicleController::SelectWeapon
================
*/
void jkVehicleController::SelectWeapon ( int weapon ) {
	//if( mVehicle.IsValid() ) {
	if (mVehicle) {
		mVehicle->GetPosition( mPosition )->SelectWeapon( weapon );
	}
}
