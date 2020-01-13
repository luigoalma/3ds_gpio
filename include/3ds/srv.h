/**
 * @file srv.h
 * @brief Service API.
 */
#pragma once

/// Initializes the service API.
Result srvInit(void);

/// Exits the service API.
void srvExit(void);

/// Registers the current process as a client to the service API.
Result srvRegisterClient(void);

/**
 * @brief Enables service notificatios, returning a notification semaphore.
 * @param semaphoreOut Pointer to output the notification semaphore to.
 */
Result srvEnableNotification(Handle* semaphoreOut);

/**
 * @brief Registers the current process as a service.
 * @param out Pointer to write the service handle to.
 * @param name Name of the service.
 * @param maxSessions Maximum number of sessions the service can handle.
 */
Result srvRegisterService(Handle* out, const char* name, int maxSessions);

/**
 * @brief Unregisters the current process as a service.
 * @param name Name of the service.
 */
Result srvUnregisterService(const char* name);

/**
 * @brief Receives a notification.
 * @param notificationIdOut Pointer to output the ID of the received notification to.
 */
Result srvReceiveNotification(u32* notificationIdOut);
