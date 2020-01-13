	.arch armv6k
	.arm

.macro SVC_BEGIN name
	.section .text.\name, "ax", %progbits
	.global \name
	.type \name, %function
	.align 2
\name:
.endm

.macro SVC_END name
	.size	\name, .-\name
.endm

SVC_BEGIN svcSleepThread
	svc 0x0A
	bx  lr
SVC_END svcSleepThread

SVC_BEGIN svcCloseHandle
	svc 0x23
	bx  lr
SVC_END svcCloseHandle

SVC_BEGIN svcConnectToPort
	str r0, [sp, #-0x4]!
	svc 0x2D
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr
SVC_END svcConnectToPort

SVC_BEGIN svcSendSyncRequest
	svc 0x32
	bx  lr
SVC_END svcSendSyncRequest

SVC_BEGIN svcGetProcessId
	str r0, [sp, #-0x4]!
	svc 0x35
	ldr r3, [sp], #4
	str r1, [r3]
	bx  lr
SVC_END svcGetProcessId

SVC_BEGIN svcBreak
	svc 0x3C
	bx  lr
SVC_END svcBreak

SVC_BEGIN svcAcceptSession
	str r0, [sp, #-4]!
	svc 0x4A
	ldr r2, [sp]
	str r1, [r2]
	add sp, sp, #4
	bx  lr
SVC_END svcAcceptSession

SVC_BEGIN svcReplyAndReceive
	str r0, [sp, #-4]!
	svc 0x4F
	ldr r2, [sp]
	str r1, [r2]
	add sp, sp, #4
	bx  lr
SVC_END svcReplyAndReceive

SVC_BEGIN svcBindInterrupt
	svc 0x50
	bx lr
SVC_END svcBindInterrupt

SVC_BEGIN svcUnbindInterrupt
	svc 0x51
	bx lr
SVC_END svcUnbindInterrupt
