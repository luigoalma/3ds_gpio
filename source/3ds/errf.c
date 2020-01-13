#include <memset.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/errf.h>
#include <3ds/ipc.h>

static Handle errfHandle;
static int errfRefCount;

Result errfInit(void)
{
	Result rc = 0;

	if (errfRefCount++) return 0;

	rc = svcConnectToPort(&errfHandle, "err:f");

	if (R_FAILED(rc)) errfExit();
	return rc;
}

void errfExit(void)
{
	if (--errfRefCount)
		return;
	svcCloseHandle(errfHandle);
}

static inline void getCommonErrorData(ERRF_FatalErrInfo* error, Result failure)
{
	error->resCode = failure;
	svcGetProcessId(&error->procId, 0xFFFF8001);
}

void ERRF_ThrowResultNoRet(Result failure)
{
	while (R_FAILED(errfInit())) {
		svcSleepThread(100000000LLU);
	}

	// manually inlined ERRF_Throw and adjusted to make smaller code output
	uint32_t *cmdbuf = getThreadCommandBuffer();
	_memset32_aligned(&cmdbuf[1], 0, sizeof(ERRF_FatalErrInfo));
	ERRF_FatalErrInfo* error = (ERRF_FatalErrInfo*)&cmdbuf[1];
	error->type = ERRF_ERRTYPE_GENERIC;
	error->pcAddr = (u32)__builtin_extract_return_addr(__builtin_return_address(0));
	getCommonErrorData(error, failure);

	svcSendSyncRequest(errfHandle);
	errfExit();

	for (;;) {
		svcSleepThread(10000000000LLU); // lighter loop
	}
}
