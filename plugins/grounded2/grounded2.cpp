// Copyright The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#define MUMBLE_PLUGIN_API_MAJOR_MACRO 1
#define MUMBLE_PLUGIN_API_MINOR_MACRO 0
#define MUMBLE_PLUGIN_API_PATCH_MACRO 0

#include "ProcessWindows.h"
#include "MumblePlugin.h"

#include <math.h>
#include <memory>
#include <string.h>

/* lifted from output from https://github.com/Encryqed/Dumper-7/ */
namespace Grounded2 {

/*
 * ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
 * ┃ UWorld                               ┃
 * ┃  UGameInstance *OwningGameInstance   ●─┐
 * ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ │
 *                   ┌──────────────────────┘
 * ┏━━━━━━━━━━━━━━━━━┷━━━━━━━━━━━━━━━━━━━━┓
 * ┃ UGameInstance                        ┃
 * ┃  ULocalPlayer *LocalPlayers          ●─┐
 * ┃  uint32_t      LocalPlayersLen       ┃ │
 * ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ │
 *                   ┌──────────────────────┘
 * ┏━━━━━━━━━━━━━━━━━┷━━━━━━━━━━━━━━━━━━━━┓
 * ┃ UPlayer                              ┃
 * ┃  APlayerController *PlayerController ●─┐
 * ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ │
 *                    ┌─────────────────────┘
 * ┏━━━━━━━━━━━━━━━━━━┷━━━━━━━━━━━━━━━━━━━━━━━━━┓   ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
 * ┃ APlayerController                          ┃   ┃ ACharacter                      ┃
 * ┃  ACharacter           *Character           ●───┨  USceneComponent *RootComponent ●─┐
 * ┃  APlayerCameraManager *PlayerCameraManager ●─┐ ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ │
 * ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ │                  ┌──────────────────┘
 *                   ┌────────────────────────────┘ ┏━━━━━━━━━━━━━━━━┷━━━━━━━━━━━━━━━━┓
 * ┏━━━━━━━━━━━━━━━━━┷━━━━━━━━━━━━━━━━━━━━━━━┓      ┃ USceneComponent                 ┃
 * ┃ APlayerCameraManager                    ┃      ┃  FVector  RelativeLocation      ┃
 * ┃  FCameraCacheEntry CameraCachePrivate   ●─┐    ┃  FRotator RelativeRotation      ┃
 * ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ │    ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 *                   ┌─────────────────────────┘
 * ┏━━━━━━━━━━━━━━━━━┷━━━━━━━┓
 * ┃ FCameraCacheEntry       ┃
 * ┃  FMinimalViewInfo POV   ●─┐
 * ┗━━━━━━━━━━━━━━━━━━━━━━━━━┛ │
 *                   ┌─────────┘
 * ┏━━━━━━━━━━━━━━━━━┷━━━━━━━┓
 * ┃ FMinimalViewInfo        ┃
 * ┃  FVector  Location      ┃
 * ┃  FRotator Rotation      ┃
 * ┗━━━━━━━━━━━━━━━━━━━━━━━━━┛
 */
struct FVector {
	/* positive x is west, positive y is north, positive z is up  */
	double X, Y, Z;
};
struct FRotator {
	/* the zero value is standing up facing west, 90.0 yaw is facing south,
	 * negative pitch looks down */
	double Pitch, Yaw, Roll;
};

struct FMinimalViewInfo final {
	struct FVector Location;  // 0x0000
	struct FRotator Rotation; // 0x0018
};

struct FCameraCacheEntry {
	uint8_t Pad_0[0x10];
	struct FMinimalViewInfo POV; // 0x0010
};

struct APlayerCameraManager {
	uint8_t Pad_0[0x13a0];
	struct FCameraCacheEntry CameraCachePrivate; // 0x13A0
};

struct USceneComponent {
	uint8_t Pad_0[0x128];
	struct FVector RelativeLocation;  // 0x0128
	struct FRotator RelativeRotation; // 0x0140
};

struct ACharacter {
	uint8_t Pad_0[0x1A0];
	struct USceneComponent *RootComponent; // 0x01A0
};

struct APlayerController {
	uint8_t Pad_0[0x300];
	/* is null when at the main menu */
	struct ACharacter *Character;                     // 0x0300
	uint8_t Pad_308[0x60];                            // 0x0308
	struct APlayerCameraManager *PlayerCameraManager; // 0x0368
};

struct UPlayer {
	uint8_t Pad_0[0x30];
	struct APlayerController *PlayerController; // 0x0030
};

struct TArray {
	uintptr_t *Data;
	uint32_t Len;
};

struct UGameInstance {
	uint8_t Pad_0[0x38];
	struct TArray /* ULocalPlayer* */ LocalPlayers; // 0x0038
};

struct UWorld {
	uint8_t Pad_0[0x1e0];
	struct UGameInstance *OwningGameInstance; // 0x01E0
};
} // namespace Grounded2

constexpr float gameToMumbleUnits(float game) {
	return game / 200.0f;
}

float deg2radf(double rad) {
	return ((float) rad) * M_PI / 180.0;
}

void rotatorToUnitVectors(Grounded2::FRotator rot, float forward[3], float up[3]) {
	float sinPitch = sin(deg2radf(rot.Pitch));
	float cosPitch = cos(deg2radf(rot.Pitch));

	float sinYaw = sin(deg2radf(rot.Yaw));
	float cosYaw = cos(deg2radf(rot.Yaw));

	float sinRoll = sin(deg2radf(rot.Roll));
	float cosRoll = cos(deg2radf(rot.Roll));

	forward[0] = cosPitch * cosYaw;
	forward[1] = cosPitch * sinYaw;
	forward[2] = sinPitch;

	up[0] = cosYaw * sinPitch * cosRoll + sinYaw * sinRoll;
	up[1] = -sinYaw * sinPitch * cosRoll - cosYaw * sinRoll;
	up[2] = cosPitch * cosRoll;
}

struct LocationRotation {
	Grounded2::FVector Location;
	Grounded2::FRotator Rotation;
};

struct Grounded2Lookup {
	Grounded2::UWorld *world;
	Grounded2::USceneComponent *component;
	Grounded2::APlayerCameraManager *camera;
};

struct Grounded2Attachment {
	ProcessWindows proc;
	Grounded2::UWorld **worldp;
	Grounded2Lookup last;
};

static std::unique_ptr< Grounded2Attachment > handle;


mumble_error_t mumble_init(uint32_t) {
	return MUMBLE_STATUS_OK;
}

void mumble_shutdown() {
}

MumbleStringWrapper mumble_getName() {
	static const char name[] = "Grounded 2";

	MumbleStringWrapper wrapper;
	wrapper.data           = name;
	wrapper.size           = strlen(name);
	wrapper.needsReleasing = false;

	return wrapper;
}

MumbleStringWrapper mumble_getDescription() {
	static const char description[] = "Positional audio support for Grounded 2. Steam release version 0.2.0.1 Rel 2181664";

	MumbleStringWrapper wrapper;
	wrapper.data           = description;
	wrapper.size           = strlen(description);
	wrapper.needsReleasing = false;

	return wrapper;
}

MumbleStringWrapper mumble_getAuthor() {
	static const char author[] = "sqwishy <somebody@froghat.ca>";

	MumbleStringWrapper wrapper;
	wrapper.data           = author;
	wrapper.size           = strlen(author);
	wrapper.needsReleasing = false;

	return wrapper;
}

mumble_version_t mumble_getAPIVersion() {
	return MUMBLE_PLUGIN_API_VERSION;
}

void mumble_registerAPIFunctions(void *) {
}

void mumble_releaseResource(const void *) {
}

mumble_version_t mumble_getVersion() {
	return { 1, 0, 0 };
}

uint32_t mumble_getFeatures() {
	return MUMBLE_FEATURE_POSITIONAL;
}

uint8_t mumble_initPositionalData(const char *const *programNames, const uint64_t *programPIDs, size_t programCount) {
	const std::string exename = "Grounded2-WinGRTS-Shipping.exe";

	for (size_t i = 0; i < programCount; ++i) {
		if (programNames[i] != exename) {
			continue;
		}

		ProcessWindows proc(programPIDs[i], programNames[i]);

		if (!proc.isOk()) {
			continue;
		}

		const Modules &modules = proc.modules();
		const auto iter        = modules.find(exename);

		if (iter == modules.cend()) {
			continue;
		}

		// 48:8B41 18           | mov rax,qword ptr ds:[rcx+18]
		// 48:85C0              | test rax,rax
		// 74 09                | je loc-win64-shipping.xxxxxxxxxxxx
		// 48:8B40 30           | mov rax,qword ptr ds:[rax+30]
		// 48:85C0              | test rax,rax
		// 75 07                | jne loc-win64-shipping.xxxxxxxxxxxx
		// 48:8B05 FF9CB705     | mov rax,qword ptr ds:[<class UWorldProxy GWorld>]
		// C3                   | ret
		//
		// 48 8B 41 18 48 85 C0 74 09 48 8B 40 30 48 85 C0 75 07 48 8B 05 ?? ?? ?? ?? C3
		//
		// This pattern worked for an Early Access version of
		// "Oddsparks: An Automation Adventure" (UE5.4) and two stable
		// versions (1.x) (UE5.5) on Steam. And on Grounded 2 on Steam
		// version 0.2.0.1 Rel 2181664 (UE5.4).
		//
		// This gives us a handle to a UWorld. So, peekRIP here gets us
		// an address containing a pointer to a UWorld. The pointer at
		// that address can change -- for example when backing out to
		// the main menu.
		const std::vector< uint8_t > pattern = {
			// clang-format off
			0x48, 0x8B, 0x41, 0x18,
			0x48, 0x85, 0xC0,
			0x74, 0x09,
			0x48, 0x8B, 0x40, 0x30,
			0x48, 0x85, 0xC0,
			0x75, 0x07,
			0x48, 0x8B, 0x05, '?',  '?', '?', '?',
			0xC3,
			// clang-format on
		};

		procptr_t addr, ok;

		if (!(addr = proc.findPattern(pattern, iter->second))) {
			continue;
		}

		if (!(addr = proc.peekRIP(addr + 0x15))) {
			continue;
		}

		Grounded2Attachment s = { std::move(proc), (Grounded2::UWorld **) addr };
		handle                = std::make_unique< Grounded2Attachment >(std::move(s));

		return MUMBLE_PDEC_OK;
	}

	return MUMBLE_PDEC_ERROR_TEMP;
}

void mumble_shutdownPositionalData() {
	handle.reset();
}

bool readGamePositionalDataShort(ProcessWindows &proc, Grounded2Lookup &lookup, LocationRotation &act,
								 LocationRotation &view) {
	// clang-format off
	return proc.peek((procptr_t) &lookup.component->RelativeLocation, act)
	    && proc.peek((procptr_t) &lookup.camera->CameraCachePrivate.POV, view);
	// clang-format on
}

bool readGamePositionalDataLong(ProcessWindows &proc, Grounded2Lookup &lookup, LocationRotation &act,
								LocationRotation &view) {
	struct Grounded2::UGameInstance *instance;
	struct Grounded2::TArray players;
	struct Grounded2::UPlayer *player;
	struct Grounded2::APlayerController *controller;
	struct Grounded2::ACharacter *character;

	// clang-format off
	return proc.peek((procptr_t) &lookup.world->OwningGameInstance, instance)
	    && proc.peek((procptr_t) &instance->LocalPlayers, players)
	    && players.Len
	    && proc.peek((procptr_t) &players.Data[0], player)
	    && proc.peek((procptr_t) &player->PlayerController, controller)
	    && proc.peek((procptr_t) &controller->Character, character)
	    && character
	    && proc.peek((procptr_t) &character->RootComponent, lookup.component)
	    && proc.peek((procptr_t) &lookup.component->RelativeLocation, act)
	    && proc.peek((procptr_t) &controller->PlayerCameraManager, lookup.camera)
	    && proc.peek((procptr_t) &lookup.camera->CameraCachePrivate.POV, view);
	// clang-format on
}

bool mumble_fetchPositionalData(float *avatarPos, float *avatarForward, float *avatarUp, float *cameraPos,
								float *cameraForward, float *cameraUp, const char **contextPtr,
								const char **identityPtr) {
	*contextPtr  = "";
	*identityPtr = "";

	Grounded2Lookup lookup;
	LocationRotation actor;
	LocationRotation view;
	bool ok = false;

	if (!handle) {
		return false;
	}

	if (!handle->proc.peek((procptr_t) handle->worldp, lookup.world)) {
		/* shut down if reading the pointer to the pointer to the world fails,
		 * the game probably quit */
		return false;
	}

	/* if the world address hasn't changed, use addreses from a past lookup */
	if (handle->last.world == lookup.world && readGamePositionalDataShort(handle->proc, lookup, actor, view)) {
		ok = true;
		/* otherwise, try the full pointer chain and save the lookup if it works */
	} else if (readGamePositionalDataLong(handle->proc, lookup, actor, view)) {
		ok           = true;
		handle->last = lookup;
	}

	if (ok) {
		avatarPos[0] = gameToMumbleUnits(actor.Location.X);
		avatarPos[1] = gameToMumbleUnits(actor.Location.Y);
		avatarPos[2] = gameToMumbleUnits(actor.Location.Z);

		rotatorToUnitVectors(actor.Rotation, avatarForward, avatarUp);

		cameraPos[0] = gameToMumbleUnits(view.Location.X);
		cameraPos[1] = gameToMumbleUnits(view.Location.Y);
		cameraPos[2] = gameToMumbleUnits(view.Location.Z);

		rotatorToUnitVectors(view.Rotation, cameraForward, cameraUp);
	} else {
		std::fill_n(avatarPos, 3, 0.f);
		std::fill_n(avatarForward, 3, 0.f);
		std::fill_n(avatarUp, 3, 0.f);

		std::fill_n(cameraPos, 3, 0.f);
		std::fill_n(cameraForward, 3, 0.f);
		std::fill_n(cameraUp, 3, 0.f);
	}

	return true;
}
