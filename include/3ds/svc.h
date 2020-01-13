/**
 * @file svc.h
 * @brief Syscall wrappers.
 */
#pragma once

#include "types.h"

/**
 * @brief Gets the thread local storage buffer.
 * @return The thread local storage bufger.
 */
static inline void* getThreadLocalStorage(void)
{
	void* ret;
	__asm__ ("mrc p15, 0, %[data], c13, c0, 3" : [data] "=r" (ret));
	return ret;
}

/**
 * @brief Gets the thread command buffer.
 * @return The thread command bufger.
 */
static inline u32* getThreadCommandBuffer(void)
{
	return (u32*)((u8*)getThreadLocalStorage() + 0x80);
}

/**
 * @brief Gets the ID of a process.
 * @param[out] out Pointer to output the process ID to.
 * @param handle Handle of the process to get the ID of.
 */
Result svcGetProcessId(u32 *out, Handle handle);

/**
 * @brief Connects to a port.
 * @param[out] out Pointer to output the port handle to.
 * @param portName Name of the port.
 */
Result svcConnectToPort(volatile Handle* out, const char* portName);

/**
 * @brief Puts the current thread to sleep.
 * @param ns The minimum number of nanoseconds to sleep for.
 */
void svcSleepThread(s64 ns);

/**
 * @brief Sends a synchronized request to a session handle.
 * @param session Handle of the session.
 */
Result svcSendSyncRequest(Handle session);

/**
 * @brief Accepts a session.
 * @param[out] session Pointer to output the created session handle to.
 * @param port Handle of the port to accept a session from.
 */
Result svcAcceptSession(Handle* session, Handle port);

/**
 * @brief Replies to and receives a new request.
 * @param index Pointer to the index of the request.
 * @param handles Session handles to receive requests from.
 * @param handleCount Number of handles.
 * @param replyTarget Handle of the session to reply to.
 */
Result svcReplyAndReceive(s32* index, const Handle* handles, s32 handleCount, Handle replyTarget);

/**
 * @brief Binds an event or semaphore handle to an ARM11 interrupt.
 * @param interruptId Interrupt identfier (see https://www.3dbrew.org/wiki/ARM11_Interrupts).
 * @param eventOrSemaphore Event or semaphore handle to bind to the given interrupt.
 * @param priority Priority of the interrupt for the current process.
 * @param isManualClear Indicates whether the interrupt has to be manually cleared or not (= level-high active).
 */
Result svcBindInterrupt(u32 interruptId, Handle eventOrSemaphore, s32 priority, bool isManualClear);

/**
 * @brief Unbinds an event or semaphore handle from an ARM11 interrupt.
 * @param interruptId Interrupt identfier, see (see https://www.3dbrew.org/wiki/ARM11_Interrupts).
 * @param eventOrSemaphore Event or semaphore handle to unbind from the given interrupt.
 */
Result svcUnbindInterrupt(u32 interruptId, Handle eventOrSemaphore);

/**
 * @brief Closes a handle.
 * @param handle Handle to close.
 */
Result svcCloseHandle(Handle handle);

/// Stop point, does nothing if the process is not attached (as opposed to 'bkpt' instructions)
#define SVC_STOP_POINT __asm__ volatile("svc 0xFF");
