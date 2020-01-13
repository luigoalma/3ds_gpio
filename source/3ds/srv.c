#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/srv.h>
#include <3ds/svc.h>
#include <3ds/ipc.h>

static inline int str_len_and_copy(char* out, const char* name) {
	int s = 0;
	while (*name) {
		*out = *name;
		if(++s == 8)
			return s;
		name++;
		out++;
	}
	*out = 0;
	return s;
}

static Handle srvHandle;
static int srvRefCount;

Result srvInit(void)
{
	Result rc = 0;

	if (srvRefCount++) return 0;

	rc = svcConnectToPort(&srvHandle, "srv:");
	if (R_SUCCEEDED(rc)) rc = srvRegisterClient();

	if (R_FAILED(rc)) srvExit();
	return rc;
}

void srvExit(void)
{
	if (--srvRefCount) return;

	if (srvHandle != 0) svcCloseHandle(srvHandle);
	srvHandle = 0;
}

Result srvRegisterClient(void)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,0,2); // 0x10002
	cmdbuf[1] = IPC_Desc_CurProcessHandle();

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	return cmdbuf[1];
}

Result srvEnableNotification(Handle* semaphoreOut)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,0,0); // 0x20000

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	if(semaphoreOut) *semaphoreOut = cmdbuf[3];

	return cmdbuf[1];
}

Result srvRegisterService(Handle* out, const char* name, int maxSessions)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3,4,0); // 0x30100
	cmdbuf[3] = str_len_and_copy((char*) &cmdbuf[1], name);
	cmdbuf[4] = maxSessions;

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	if(out) *out = cmdbuf[3];

	return cmdbuf[1];
}

Result srvUnregisterService(const char* name)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4,3,0); // 0x400C0
	cmdbuf[3] = str_len_and_copy((char*) &cmdbuf[1], name);

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	return cmdbuf[1];
}

Result srvReceiveNotification(u32* notificationIdOut)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xB,0,0); // 0xB0000

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	if(notificationIdOut) *notificationIdOut = cmdbuf[2];

	return cmdbuf[1];
}
