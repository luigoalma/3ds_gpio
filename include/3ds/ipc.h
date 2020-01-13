/**
 * @file ipc.h
 * @brief Inter Process Communication helpers
 */
#pragma once

#include <3ds/types.h>

/**
 * @brief Creates a command header to be used for IPC
 * @param command_id       ID of the command to create a header for.
 * @param normal_params    Size of the normal parameters in words. Up to 63.
 * @param translate_params Size of the translate parameters in words. Up to 63.
 * @return The created IPC header.
 *
 * Normal parameters are sent directly to the process while the translate parameters might go through modifications and checks by the kernel.
 * The translate parameters are described by headers generated with the IPC_Desc_* functions.
 *
 * @note While #normal_params is equivalent to the number of normal parameters, #translate_params includes the size occupied by the translate parameters headers.
 */
static inline u32 IPC_MakeHeader(u16 command_id, unsigned normal_params, unsigned translate_params)
{
	return ((u32) command_id << 16) | (((u32) normal_params & 0x3F) << 6) | (((u32) translate_params & 0x3F) << 0);
}

/**
 * @brief Creates a header to share handles
 * @param number The number of handles following this header. Max 64.
 * @return The created shared handles header.
 *
 * The #number next values are handles that will be shared between the two processes.
 *
 * @note Zero values will have no effect.
 */
static inline u32 IPC_Desc_SharedHandles(unsigned number)
{
	return ((u32)(number - 1) << 26);
}

/**
 * @brief Returns the code to ask the kernel to fill the handle with the current process handle.
 * @return The code to request the current process handle.
 *
 * The next value is a placeholder that will be replaced by the current process handle by the kernel.
 */
static inline u32 IPC_Desc_CurProcessHandle(void)
{
	return 0x20;
}
