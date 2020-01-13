#pragma once
#include <3ds/errf.h>
#include <3ds/types.h>
#include <3ds/result.h>

#define Err_Throw(failure) ERRF_ThrowResultNoRet(failure)

#define Err_FailedThrow(failure) do {Result __tmp = failure; if (R_FAILED(__tmp)) Err_Throw(__tmp);} while(0)
