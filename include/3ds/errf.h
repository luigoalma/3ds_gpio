/**
 * @file errf.h
 * @brief Error Display API
 */

#pragma once

#include <3ds/types.h>

/// Types of errors that can be thrown by err:f.
typedef enum {
	ERRF_ERRTYPE_GENERIC      = 0, ///< For generic errors. Shows miscellaneous info.
	ERRF_ERRTYPE_MEM_CORRUPT  = 1, ///< Same output as generic, but informs the user that "the System Memory has been damaged".
	ERRF_ERRTYPE_CARD_REMOVED = 2, ///< Displays the "The Game Card was removed." message.
	ERRF_ERRTYPE_EXCEPTION    = 3, ///< For exceptions, or more specifically 'crashes'. union data should be exception_data.
	ERRF_ERRTYPE_FAILURE      = 4, ///< For general failure. Shows a message. union data should have a string set in failure_mesg
	ERRF_ERRTYPE_LOGGED       = 5, ///< Outputs logs to NAND in some cases.
} ERRF_ErrType;

/// Types of 'Exceptions' thrown for ERRF_ERRTYPE_EXCEPTION
typedef enum {
	ERRF_EXCEPTION_PREFETCH_ABORT = 0, ///< Prefetch Abort
	ERRF_EXCEPTION_DATA_ABORT     = 1, ///< Data abort
	ERRF_EXCEPTION_UNDEFINED      = 2, ///< Undefined instruction
	ERRF_EXCEPTION_VFP            = 3, ///< VFP (floating point) exception.
} ERRF_ExceptionType;

typedef struct {
	ERRF_ExceptionType type; ///< Type of the exception. One of the ERRF_EXCEPTION_* values.
	u8  reserved[3];
	u32 fsr;                ///< ifsr (prefetch abort) / dfsr (data abort)
	u32 far;                ///< pc = ifar (prefetch abort) / dfar (data abort)
	u32 fpexc;
	u32 fpinst;
	u32 fpinst2;
} ERRF_ExceptionInfo;

typedef struct {
	ERRF_ExceptionInfo excep;   ///< Exception info struct
	CpuRegisters regs;          ///< CPU register dump.
} ERRF_ExceptionData;

typedef struct {
	ERRF_ErrType type; ///< Type, one of the ERRF_ERRTYPE_* enum
	u8  revHigh;       ///< High revison ID
	u16 revLow;        ///< Low revision ID
	u32 resCode;       ///< Result code
	u32 pcAddr;        ///< PC address at exception
	u32 procId;        ///< Process ID.
	u64 titleId;       ///< Title ID.
	u64 appTitleId;    ///< Application Title ID.
	union {
		ERRF_ExceptionData exception_data; ///< Data for when type is ERRF_ERRTYPE_EXCEPTION
		char failure_mesg[0x60];             ///< String for when type is ERRF_ERRTYPE_FAILURE
	} data;                                ///< The different types of data for errors.
} ERRF_FatalErrInfo;

/// Initializes ERR:f. Unless you plan to call ERRF_Throw yourself, do not use this.
Result errfInit(void);

/// Exits ERR:f. Unless you plan to call ERRF_Throw yourself, do not use this.
void errfExit(void);

void ERRF_ThrowResultNoRet(Result failure) __attribute__((noreturn, noinline));
