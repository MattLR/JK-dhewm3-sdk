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

#include "Anim_Vertex.h"
#include "Game_local.h"
#include "renderer/ModelManager.h"

/*
========================================

dnDeclVertexAnim

========================================
*/

/*
=====================
dnDeclVertexAnim::DefaultDefinition
=====================
*/
const char* dnDeclVertexAnim::DefaultDefinition() const {
	return "{\n\t\"model\" : \"path/to/model.md3\"\n\t\"anim <name> <start_frame> <end_frame>\"\n}\n";
}

/*
=====================
dnDeclVertexAnim::Parse
=====================
*/
bool dnDeclVertexAnim::Parse(const char* text, const int textLength) {
	idLexer src;
	idToken token;

	src.LoadMemory(text, textLength, GetFileName(), GetLineNum());
	src.SetFlags(DECL_LEXER_FLAGS);
	src.SkipUntilString("{");

	while (src.ReadToken(&token)) {
		if (token == "model") {
			src.ReadToken(&token);
			model = token;
		}
		else if (token == "anim") {
			idDict anim;
			src.ReadToken(&token); // Read anim name
			anim.Set("name", token);

			src.ReadToken(&token); // Read start frame
			anim.Set("start_frame", token);

			src.ReadToken(&token); // Read end frame
			anim.Set("end_frame", token);

			anims.Append(anim);
		}
		else if (token == "}") {
			break;
		}
		else {
			src.Warning("Unknown token: %s", token.c_str());
			return false;
		}
	}
	return true;
}

/*
=====================
dnDeclVertexAnim::FindAnim
=====================
*/
const idDict* dnDeclVertexAnim::FindAnim(const char* name) const {
	for (int i = 0; i < anims.Num(); i++) {
		idStr animName = anims[i].GetString("name");
		if (animName == name) {
			return &anims[i];
		}
	}

	return nullptr;
}

/*
=====================
dnDeclVertexAnim::FreeData
=====================
*/
void dnDeclVertexAnim::FreeData() {
	model.Clear();
	anims.Clear();
}

/*
=====================
dnDeclVertexAnim::GetModel
=====================
*/
const idStr& dnDeclVertexAnim::GetModel() const {
	return model;
}

/*
=====================
dnDeclVertexAnim::GetAnims
=====================
*/
const idList<idDict>& dnDeclVertexAnim::GetAnims() const {
	return anims;
}

/*
========================================

dnDeclVertexAnim

========================================
*/

/*
=====================
dnVertexAnimator::SetModel
=====================
*/
idRenderModel* dnVertexAnimator::SetModel(const char* modelDecl) {
	vertexAnim = (const dnDeclVertexAnim *)declManager->FindType(DECL_VERTEXMODELDEF, modelDecl, false);
	if (!vertexAnim) {
		gameLocal.Warning("dnVertexAnimator::SetModel: Failed to load vertex model decl %s\n", modelDecl);
		return nullptr;
	}

	start_frame = 0;
	end_frame = 0;
	currentFrame = 0;
	isLooping = false;

	int currentTime = sys->GetMilliseconds();
	lastTime = currentTime;

	renderModel = renderModelManager->FindModel(vertexAnim->GetModel());
	return renderModel;
}

/*
=====================
dnVertexAnimator::PlayAnim
=====================
*/
void dnVertexAnimator::PlayAnim(const char* animName, bool loop) {
	currentAnim = vertexAnim->FindAnim(animName);
	if (currentAnim == nullptr) {
		common->Warning("Failed to find vertex animation %s\n", animName);
		return;
	}

	if (isLooping && loop && currentAnimName == animName)
		return;

	currentAnimName = animName;

	int currentTime = sys->GetMilliseconds();
	lastTime = currentTime;

	isLooping = loop;
	start_frame = currentFrame = currentAnim->GetInt("start_frame");
	end_frame = currentAnim->GetInt("end_frame");
}

/*
=====================
dnVertexAnimator::Update
=====================
*/

void dnVertexAnimator::Update(void) {
	if (vertexAnim == nullptr || currentAnim == nullptr) {
		gameLocal.DPrintf("TestingANimator broken");
		return;
	}

	// Get the current time and calculate the delta time in seconds
	int currentTime = sys->GetMilliseconds();
	int deltaTime = currentTime - lastTime;
	lastTime = currentTime;
	float deltaTimeSeconds = deltaTime / 1000.0f;

	// Calculate the total animation duration in seconds
	float animSpeed = 24.0f;
	int frameCount = end_frame - start_frame;
	float animDuration = frameCount / animSpeed;

	// Update the current frame based on the elapsed time
	float framesAdvanced = deltaTimeSeconds * animSpeed;
	lastFrame = currentFrame;
	currentFrame += deltaTimeSeconds * animSpeed;

	// Handle looping or clamping to end_frame
	if (currentFrame >= end_frame) {
		if (isLooping) {
			currentFrame = start_frame;			
		}
		else {
			currentFrame = end_frame;
		}		
	}

	// Calculate the backlerp value
	float frameFraction = fmod(framesAdvanced, 1.0f);
	backlerp = 1.0f - frameFraction;
}

/*
=====================
dnVertexAnimator::IsAnimDone
=====================
*/
bool dnVertexAnimator::IsAnimDone(void) {
	if (isLooping) {
		return false;
	}

	return currentFrame >= end_frame;
}

/*
=====================
dnVertexAnimator::ClearAllAnims
=====================
*/
void dnVertexAnimator::ClearAllAnims(void) {
	currentAnim = nullptr;
}

/* FIXME Dynamix - engine side
=====================
dnVertexAnimator::GetJointHandle
=====================

jointHandle_t dnVertexAnimator::GetJointHandle(const char* name) {
	if (renderModel == nullptr)
		return INVALID_JOINT;

	return renderModel->FindTag(name);
}
*/

/* FIXME Dynamix - engine side
=====================
dnVertexAnimator::GetJointTransform
=====================

bool dnVertexAnimator::GetJointTransform(jointHandle_t jointHandle, idVec3& offset, idMat3& axis) {
	offset.Zero();
	axis.Identity();

	if (renderModel == nullptr)
		return false;

	if (jointHandle == INVALID_JOINT)
		return false;

	md3Tag_t* tag = renderModel->GetTag(jointHandle, currentFrame);
	if (tag == nullptr)
		return false;

	offset = tag->origin;
	axis = idMat3(tag->axis[0], tag->axis[1], tag->axis[2]);

	return true;
}
	*/