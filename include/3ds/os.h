/**
 * @file os.h
 * @brief OS related stuff.
 */
#pragma once
#include "svc.h"

/// Packs a system version from its components.
#define SYSTEM_VERSION(major, minor, revision) \
	(((major)<<24)|((minor)<<16)|((revision)<<8))

/**
 * @brief Gets the system's FIRM version.
 * @return The system's FIRM version.
 *
 * This can be used to compare system versions easily with @ref SYSTEM_VERSION.
 */
static inline u32 osGetFirmVersion(void)
{
	return (*(vu32*)0x1FF80060) & ~0xFF;
}
