#include <3ds/ipc.h>
#include <3ds/os.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/types.h>
#include <gpio.h>
#include <err.h>
#include <memset.h>

#define OS_REMOTE_SESSION_CLOSED MAKERESULT(RL_STATUS,    RS_CANCELED, RM_OS, 26)
#define OS_INVALID_HEADER        MAKERESULT(RL_PERMANENT, RS_WRONGARG, RM_OS, 47)
#define OS_INVALID_IPC_PARAMATER MAKERESULT(RL_PERMANENT, RS_WRONGARG, RM_OS, 48)

static const char* const GPIO_ServiceNames[] = {"gpio:CDC", "gpio:MCU", "gpio:HID", "gpio:NWM", "gpio:IR", "gpio:NFC", "gpio:QTM"};
static const u32 GPIO_ServiceBitmasks_V2048[] = {GPIO_CDC_MASK, GPIO_MCU_MASK, GPIO_HID_MASK, GPIO_NWM_MASK, GPIO_IR_MASK_GE_V2048, GPIO_NFC_MASK, GPIO_QTM_MASK};
static const u32 GPIO_ServiceBitmasks_V0[] = {GPIO_CDC_MASK, GPIO_MCU_MASK, GPIO_HID_MASK, GPIO_NWM_MASK, GPIO_IR_MASK_GE_V0};
static __attribute__((section(".data.TerminationFlag"))) bool TerminationFlag = false;

inline static void HandleSRVNotification() {
	u32 id;
	Err_FailedThrow(srvReceiveNotification(&id));
	if (id == 0x100)
		TerminationFlag = true;
}

// reset maybe?
// but also this only existed since v2048, >= 8.0.0-18
// in any sense related to NFC or QTM?
// or perhaps a fix in GPIO behaviour?
// BIT(7) is only referenced and allowed access to IR
inline static void GPIO_InitIO() {
	GPIO_REG3 |= BIT(23);
	GPIO_REG3 &= ~BIT(7);
}

#define IF_MASK_INTERRUPT(_mask, _interrupt) else if (mask == _mask) *interrupt = _interrupt
// interrupts that can never happen due to access bits, using the following definition
// mainly left for documentary reason
// gcc will optimize it out initially
#define IF_DEADMASK_INTERRUPT(_mask, _interrupt) else if (0) *interrupt = _interrupt

Result GPIO_MaskToInterrupt(u32 mask, u8* interrupt) {
	if(0) {} //dummy for the else if chain
	IF_DEADMASK_INTERRUPT(GPIO_MASK1,  0x63);  // Touchscreen
	IF_DEADMASK_INTERRUPT(GPIO_MASK2,  0x60);  // Shell opened
	IF_MASK_INTERRUPT    (GPIO_MASK3,  0x64);  // Headphone jack plugged in/out
	IF_DEADMASK_INTERRUPT(GPIO_MASK4,  0x66);
	IF_MASK_INTERRUPT    (GPIO_MASK6,  0x68);  // IR
	IF_MASK_INTERRUPT    (GPIO_MASK7,  0x69);
	IF_MASK_INTERRUPT    (GPIO_MASK8,  0x6A);
	IF_MASK_INTERRUPT    (GPIO_MASK9,  0x6B);
	IF_MASK_INTERRUPT    (GPIO_MASK10, 0x6C);
	IF_MASK_INTERRUPT    (GPIO_MASK11, 0x6D);
	IF_MASK_INTERRUPT    (GPIO_MASK12, 0x6E);
	IF_MASK_INTERRUPT    (GPIO_MASK13, 0x6F);
	IF_MASK_INTERRUPT    (GPIO_MASK14, 0x70);
	IF_MASK_INTERRUPT    (GPIO_MASK15, 0x71); // MCU (HOME/POWER pressed/released or WiFi switch pressed)
	IF_MASK_INTERRUPT    (GPIO_MASK16, 0x72);
	IF_MASK_INTERRUPT    (GPIO_MASK17, 0x73); // related to QTM? only allowed binding on gpio:QTM
	else return GPIO_NOT_FOUND;
	return 0;
}

static Handle GPIO_BindHandles[GPIO_BIND_MAX] = {0};
static u32 GPIO_BindHandleStoreUsage = 0;

inline static bool GPIO_IsBindFree(u32 mask) {
	return !(GPIO_BindHandleStoreUsage & mask);
}

inline static void GPIO_ReleaseBind(u8 bit) {
	GPIO_BindHandleStoreUsage &= ~BIT(bit);
	Err_FailedThrow(svcCloseHandle(GPIO_BindHandles[bit]));
}

inline static void GPIO_StoreBind(Handle bind, u8 bit) {
	GPIO_BindHandles[bit] = bind;
	GPIO_BindHandleStoreUsage |= BIT(bit);
}

inline static u32 Read_GPIO16(vu16* io, u32 mask, u32 access_mask, s8 left_shift) {
	u32 value = *io;
	mask &= access_mask;
	if (left_shift < 0)
		value >>= -left_shift;
	else
		value <<= left_shift;
	return value & mask;
}

inline static void Write_GPIO16(vu16* io, u32 value, u32 mask, u32 access_mask, s8 left_shift) {
	mask &= access_mask;
	if (left_shift < 0) {
		value >>= -left_shift;
		mask  >>= -left_shift;
	} else {
		value <<= left_shift;
		mask  <<= left_shift;
	}
	*io = (*io & ~mask) | (value & mask);
}

inline static u32 Read_GPIO32(vu32* io, u32 mask, u32 access_mask, s8 left_shift) {
	u32 value = *io;
	mask &= access_mask;
	if (left_shift < 0)
		value >>= -left_shift;
	else
		value <<= left_shift;
	return value & mask;
}

inline static void Write_GPIO32(vu32* io, u32 value, u32 mask, u32 access_mask, s8 left_shift) {
	mask &= access_mask;
	if (left_shift < 0) {
		value >>= -left_shift;
		mask  >>= -left_shift;
	} else {
		value <<= left_shift;
		mask  <<= left_shift;
	}
	*io = (*io & ~mask) | (value & mask);
}

// names for the IPC functions based off on 3dbrew named them

static Result GPIO_GetRegPart1(u32 service_bitmask, u32 mask, u32* value) {
	*value = 0;
	if (mask & ~service_bitmask)
		return GPIO_NOT_AUTHORIZED;
	if (mask & ~(GPIO_ACCESS_REG1 | GPIO_ACCESS_REG3))
		return GPIO_NOT_FOUND;
	if (mask & GPIO_ACCESS_REG1) {
		*value |= Read_GPIO32(&GPIO_REG1, mask, GPIO_ACCESS_REG1, -5);
	}
	if (mask & GPIO_ACCESS_REG3) {
		*value |= Read_GPIO32(&GPIO_REG3, mask, GPIO_ACCESS_REG3, -10);
	}

	return 0;
}

static Result GPIO_SetRegPart1(u32 service_bitmask, u32 mask, u32 value) {
	if (mask & ~service_bitmask)
		return GPIO_NOT_AUTHORIZED;
	if (mask & ~(GPIO_ACCESS_REG1 | GPIO_ACCESS_REG3))
		return GPIO_NOT_FOUND;
	if (mask & GPIO_ACCESS_REG1) {
		Write_GPIO32(&GPIO_REG1, value, mask, GPIO_ACCESS_REG1, 5);
	}
	if (mask & GPIO_ACCESS_REG3) {
		Write_GPIO32(&GPIO_REG3, value, mask, GPIO_ACCESS_REG3, 10);
	}

	return 0;
}

static Result GPIO_GetRegPart2(u32 service_bitmask, u32 mask, u32* value) {
	*value = 0;
	if (mask & ~service_bitmask)
		return GPIO_NOT_AUTHORIZED;
	if (mask & ~(GPIO_ACCESS_REG1 | GPIO_ACCESS_REG4))
		return GPIO_NOT_FOUND;

	if (mask & GPIO_ACCESS_REG1) {
		*value |= Read_GPIO32(&GPIO_REG1, mask, GPIO_ACCESS_REG1, -13);
	}
	if (mask & GPIO_ACCESS_REG4) {
		*value |= Read_GPIO32(&GPIO_REG4, mask, GPIO_ACCESS_REG4, 6);
	}

	return 0;
}

static Result GPIO_SetRegPart2(u32 service_bitmask, u32 mask, u32 value) {
	if (mask & ~service_bitmask)
		return GPIO_NOT_AUTHORIZED;
	if (mask & ~(GPIO_ACCESS_REG1 | GPIO_ACCESS_REG4))
		return GPIO_NOT_FOUND;

	if (mask & GPIO_ACCESS_REG1) {
		Write_GPIO32(&GPIO_REG1, value, mask, GPIO_ACCESS_REG1, 13);
	}
	if (mask & GPIO_ACCESS_REG4) {
		Write_GPIO32(&GPIO_REG4, value, mask, GPIO_ACCESS_REG4, -6);
	}

	return 0;
}

static Result GPIO_GetInterruptMask(u32 service_bitmask, u32 mask, u32* value) {
	*value = 0;
	if (mask & ~service_bitmask)
		return GPIO_NOT_AUTHORIZED;
	if (mask & ~(GPIO_ACCESS_REG1 | GPIO_ACCESS_REG4))
		return GPIO_NOT_FOUND;

	if (mask & GPIO_ACCESS_REG1) {
		*value |= Read_GPIO32(&GPIO_REG1, mask, GPIO_ACCESS_REG1, -21);
	}
	if (mask & GPIO_ACCESS_REG4) {
		*value |= Read_GPIO32(&GPIO_REG4, mask, GPIO_ACCESS_REG4, -10);
	}

	return 0;
}

static Result GPIO_SetInterruptMask(u32 service_bitmask, u32 mask, u32 value) {
	if (mask & ~service_bitmask)
		return GPIO_NOT_AUTHORIZED;
	if (mask & ~(GPIO_ACCESS_REG1 | GPIO_ACCESS_REG4))
		return GPIO_NOT_FOUND;

	if (mask & GPIO_ACCESS_REG1) {
		Write_GPIO32(&GPIO_REG1, value, mask, GPIO_ACCESS_REG1, 21);
	}
	if (mask & GPIO_ACCESS_REG4) {
		Write_GPIO32(&GPIO_REG4, value, mask, GPIO_ACCESS_REG4, 10);
	}

	return 0;
}

static Result GPIO_GetGPIOData(u32 service_bitmask, u32 mask, u32* value) {
	*value = 0;
	if (mask & ~service_bitmask)
		return GPIO_NOT_AUTHORIZED;
	if (mask & ~(GPIO_ACCESS_REG0 | GPIO_ACCESS_REG1 | GPIO_ACCESS_REG2 | GPIO_ACCESS_REG3 | GPIO_ACCESS_REG5))
		return GPIO_NOT_FOUND;

	if (mask & GPIO_ACCESS_REG0) {
		*value |= Read_GPIO16(&GPIO_REG0, mask, GPIO_ACCESS_REG0, 0);
	}
	if (mask & GPIO_ACCESS_REG1) {
		*value |= Read_GPIO32(&GPIO_REG1, mask, GPIO_ACCESS_REG1, 3);
	}
	if (mask & GPIO_ACCESS_REG2) {
		*value |= Read_GPIO16(&GPIO_REG2, mask, GPIO_ACCESS_REG2, 5);
	}
	if (mask & GPIO_ACCESS_REG3) {
		*value |= Read_GPIO32(&GPIO_REG3, mask, GPIO_ACCESS_REG3, 6);
	}
	if (mask & GPIO_ACCESS_REG5) {
		*value |= Read_GPIO16(&GPIO_REG5, mask, GPIO_ACCESS_REG5, 18);
	}

	return 0;
}

static Result GPIO_SetGPIOData(u32 service_bitmask, u32 mask, u32 value) {
	if (mask & ~service_bitmask)
		return GPIO_NOT_AUTHORIZED;
	if (mask & ~(GPIO_ACCESS_REG1 | GPIO_ACCESS_REG2 | GPIO_ACCESS_REG3 | GPIO_ACCESS_REG5))
		return GPIO_NOT_FOUND;

	if (mask & GPIO_ACCESS_REG1) {
		Write_GPIO32(&GPIO_REG1, value, mask, GPIO_ACCESS_REG1, -3);
	}
	if (mask & GPIO_ACCESS_REG2) {
		Write_GPIO16(&GPIO_REG2, value, mask, GPIO_ACCESS_REG2, -5);
	}
	if (mask & GPIO_ACCESS_REG3) {
		Write_GPIO32(&GPIO_REG3, value, mask, GPIO_ACCESS_REG3, -6);
	}
	if (mask & GPIO_ACCESS_REG5) {
		Write_GPIO16(&GPIO_REG5, value, mask, GPIO_ACCESS_REG5, -18);
	}

	return 0;
}

static Result GPIO_BindInterrupt(u32 service_bitmask, u32 mask, Handle bind, s32 priority) {
	if (!GPIO_IsBindFree(mask)) {
		Err_FailedThrow(svcCloseHandle(bind));
		return GPIO_BUSY;
	}

	if (mask & ~service_bitmask) {
		Err_FailedThrow(svcCloseHandle(bind));
		return GPIO_NOT_AUTHORIZED;
	}

	u8 interrupt;
	Result res = GPIO_MaskToInterrupt(mask, &interrupt);
	if (R_FAILED(res)) {
		Err_FailedThrow(svcCloseHandle(bind));
		return res;
	}

	res = svcBindInterrupt(interrupt, bind, priority, false);
	Err_FailedThrow(res);

	u8 bit;
	for (bit = 0; mask != BIT(bit); bit++) {}

	GPIO_StoreBind(bind, bit);

	return res;
}

// v2048 -> v3073: They added svcCloseHandle calls on failed exits
static Result GPIO_UnbindInterrupt(u32 service_bitmask, u32 mask, Handle bind) {
	if (GPIO_IsBindFree(mask)) {
		Err_FailedThrow(svcCloseHandle(bind));
		return GPIO_BUSY;
	}

	if (mask & ~service_bitmask) {
		Err_FailedThrow(svcCloseHandle(bind));
		return GPIO_NOT_AUTHORIZED;
	}

	u8 interrupt;
	Result res = GPIO_MaskToInterrupt(mask, &interrupt);
	if (R_FAILED(res)) {
		Err_FailedThrow(svcCloseHandle(bind));
		return res;
	}

	res = svcUnbindInterrupt(interrupt, bind);
	Err_FailedThrow(res);

	u8 bit;
	for (bit = 0; mask != BIT(bit); bit++) {}

	GPIO_ReleaseBind(bit);
	Err_FailedThrow(svcCloseHandle(bind));

	return res;
}

static void GPIO_IPCSession(u32 service_bitmask) {
	u32* cmdbuf = getThreadCommandBuffer();
	u32 value;

	switch (cmdbuf[0] >> 16) {
	case 0x1:
		cmdbuf[0] = IPC_MakeHeader(0x1, 2, 0);
		cmdbuf[1] = GPIO_GetRegPart1(service_bitmask, cmdbuf[1], &value);
		cmdbuf[2] = value;
		break;
	case 0x2:
		cmdbuf[0] = IPC_MakeHeader(0x2, 1, 0);
		cmdbuf[1] = GPIO_SetRegPart1(service_bitmask, cmdbuf[2], cmdbuf[1]);
		break;
	case 0x3:
		cmdbuf[0] = IPC_MakeHeader(0x3, 2, 0);
		cmdbuf[1] = GPIO_GetRegPart2(service_bitmask, cmdbuf[1], &value);
		cmdbuf[2] = value;
		break;
	case 0x4:
		cmdbuf[0] = IPC_MakeHeader(0x4, 1, 0);
		cmdbuf[1] = GPIO_SetRegPart2(service_bitmask, cmdbuf[2], cmdbuf[1]);
		break;
	case 0x5:
		cmdbuf[0] = IPC_MakeHeader(0x5, 2, 0);
		cmdbuf[1] = GPIO_GetInterruptMask(service_bitmask, cmdbuf[1], &value);
		cmdbuf[2] = value;
		break;
	case 0x6:
		cmdbuf[0] = IPC_MakeHeader(0x6, 1, 0);
		cmdbuf[1] = GPIO_SetInterruptMask(service_bitmask, cmdbuf[2], cmdbuf[1]);
		break;
	case 0x7:
		cmdbuf[0] = IPC_MakeHeader(0x7, 2, 0);
		cmdbuf[1] = GPIO_GetGPIOData(service_bitmask, cmdbuf[1], &value);
		cmdbuf[2] = value;
		break;
	case 0x8:
		cmdbuf[0] = IPC_MakeHeader(0x8, 1, 0);
		cmdbuf[1] = GPIO_SetGPIOData(service_bitmask, cmdbuf[2], cmdbuf[1]);
		break;
	case 0x9:
		if (cmdbuf[0] != IPC_MakeHeader(0x9, 2, 2) || cmdbuf[3] != IPC_Desc_SharedHandles(1)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
			break;
		}
		cmdbuf[0] = IPC_MakeHeader(0x9, 1, 0);
		cmdbuf[1] = GPIO_BindInterrupt(service_bitmask, cmdbuf[1], cmdbuf[4], (s32)cmdbuf[2]);
		break;
	case 0xA:
		if (cmdbuf[0] != IPC_MakeHeader(0xA, 1, 2) || cmdbuf[2] != IPC_Desc_SharedHandles(1)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
			break;
		}
		cmdbuf[0] = IPC_MakeHeader(0xA, 1, 0);
		cmdbuf[1] = GPIO_UnbindInterrupt(service_bitmask, cmdbuf[1], cmdbuf[3]);
		break;
	default:
		cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
		cmdbuf[1] = OS_INVALID_HEADER;
	}
}

static void GPIO_BindClosedSessionClean(u32 service_bitmask) {
	for(u8 i = 0; i < GPIO_BIND_MAX; i++) {
		u8 interrupt;
		u32 mask = BIT(i);
		if (!(service_bitmask & mask) || GPIO_IsBindFree(mask) || R_FAILED(GPIO_MaskToInterrupt(mask, &interrupt)))
			continue;

		Err_FailedThrow(svcUnbindInterrupt(interrupt, GPIO_BindHandles[i]));
		GPIO_ReleaseBind(i);
	}
}

static inline void initBSS() {
	extern void* __bss_start__;
	extern void* __bss_end__;
	_memset32_aligned(__bss_start__, 0, (size_t)__bss_end__ - (size_t)__bss_start__);
}

void GPIOMain() {
	initBSS();
	GPIO_InitIO();

	bool is_pre_8x = osGetFirmVersion() < SYSTEM_VERSION(2, 44, 6);
	const u32* GPIO_ServiceBitmasks = is_pre_8x ? GPIO_ServiceBitmasks_V0 : GPIO_ServiceBitmasks_V2048;
	const s32 SERVICE_COUNT = is_pre_8x ? 5 : 7;
	const s32 INDEX_MAX = SERVICE_COUNT * 2 + 1; // 11 pre 8.0, 15 post 8.0
	const s32 REMOTE_SESSION_INDEX = SERVICE_COUNT + 1; // 6 pre 8.0, 8 post 8.0

	Handle session_handles[15];

	u32 service_indexes[7];

	s32 handle_count = SERVICE_COUNT + 1;

	Err_FailedThrow(srvInit());

	for (int i = 0; i < SERVICE_COUNT; i++)
		Err_FailedThrow(srvRegisterService(&session_handles[i + 1], GPIO_ServiceNames[i], 1));

	Err_FailedThrow(srvEnableNotification(&session_handles[0]));

	Handle target = 0;
	s32 target_index = -1;
	for (;;) {
		s32 index;

		if (!target) {
			if (TerminationFlag && handle_count == REMOTE_SESSION_INDEX)
				break;
			else
				*getThreadCommandBuffer() = 0xFFFF0000;
		}

		Result res = svcReplyAndReceive(&index, session_handles, handle_count, target);
		s32 last_target_index = target_index;
		target = 0;
		target_index = -1;

		if (R_FAILED(res)) {

			if (res != OS_REMOTE_SESSION_CLOSED)
				Err_Throw(res);

			else if (index == -1) {
				if (last_target_index == -1)
					Err_Throw(GPIO_CANCELED_RANGE);
				else
					index = last_target_index;
			}

			else if (index >= handle_count)
				Err_Throw(GPIO_CANCELED_RANGE);

			svcCloseHandle(session_handles[index]);
			GPIO_BindClosedSessionClean(GPIO_ServiceBitmasks[service_indexes[index - REMOTE_SESSION_INDEX]]);
			handle_count--;
			for (s32 i = index - REMOTE_SESSION_INDEX; i < handle_count - REMOTE_SESSION_INDEX; i++) {
				session_handles[REMOTE_SESSION_INDEX + i] = session_handles[REMOTE_SESSION_INDEX + i + 1];
				service_indexes[i] = service_indexes[i + 1];
			}

			continue;
		}

		if (index == 0)
			HandleSRVNotification();

		else if (index >= 1 && index < REMOTE_SESSION_INDEX) {
			Handle newsession = 0;
			Err_FailedThrow(svcAcceptSession(&newsession, session_handles[index]));

			if (handle_count >= INDEX_MAX) {
				svcCloseHandle(newsession);
				continue;
			}

			session_handles[handle_count] = newsession;
			service_indexes[handle_count - REMOTE_SESSION_INDEX] = index - 1;
			handle_count++;

		} else if (index >= REMOTE_SESSION_INDEX && index < INDEX_MAX) {
			GPIO_IPCSession(GPIO_ServiceBitmasks[service_indexes[index - REMOTE_SESSION_INDEX]]);
			target = session_handles[index];
			target_index = index;

		} else {
			Err_Throw(GPIO_INTERNAL_RANGE);
		}
	}

	for (int i = 0; i < SERVICE_COUNT; i++) {
		Err_FailedThrow(srvUnregisterService(GPIO_ServiceNames[i]));
		svcCloseHandle(session_handles[i + 1]);
	}

	svcCloseHandle(session_handles[0]);

	srvExit();
}
