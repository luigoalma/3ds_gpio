#pragma once
#include <3ds/types.h>

// GPIO IO Memory data regions
#define GPIO_REG0 (*(vu16*)0x1EC47000)
#define GPIO_REG1 (*(vu32*)0x1EC47010)
#define GPIO_REG2 (*(vu16*)0x1EC47014)
#define GPIO_REG3 (*(vu32*)0x1EC47020)
#define GPIO_REG4 (*(vu32*)0x1EC47024)
#define GPIO_REG5 (*(vu16*)0x1EC47028)

// Single access bit masks
#define GPIO_MASK0  BIT(0)
#define GPIO_MASK1  BIT(1)
#define GPIO_MASK2  BIT(2)
#define GPIO_MASK3  BIT(3)
#define GPIO_MASK4  BIT(4)
#define GPIO_MASK5  BIT(5)
#define GPIO_MASK6  BIT(6)
#define GPIO_MASK7  BIT(7)
#define GPIO_MASK8  BIT(8)
#define GPIO_MASK9  BIT(9)
#define GPIO_MASK10 BIT(10)
#define GPIO_MASK11 BIT(11)
#define GPIO_MASK12 BIT(12)
#define GPIO_MASK13 BIT(13)
#define GPIO_MASK14 BIT(14)
#define GPIO_MASK15 BIT(15)
#define GPIO_MASK16 BIT(16)
#define GPIO_MASK17 BIT(17)
#define GPIO_MASK18 BIT(18)

// Need to define names per bit mask, depending of what's the true purpose of the bit when reading GPIO IO
// and use them instead of GPIO_MASKX defines directly

// here some based on 3dbrew information
#define GPIO_HID_PAD0   GPIO_MASK0
#define GPIO_IR_SEND    GPIO_MASK10
#define GPIO_IR_RECEIVE GPIO_MASK11
#define GPIO_HID_PAD1   GPIO_MASK14
#define GPIO_WIFI_STATE GPIO_MASK18

// Max possible binds
#define GPIO_BIND_MAX 19

// Service allowed access bits
#define GPIO_CDC_MASK          (GPIO_MASK6      | GPIO_MASK3)
#define GPIO_MCU_MASK          (GPIO_WIFI_STATE | GPIO_MASK15  | GPIO_MASK5)
#define GPIO_HID_MASK          (GPIO_HID_PAD1   | GPIO_MASK9   | GPIO_MASK8 | GPIO_HID_PAD0)
#define GPIO_NWM_MASK          (GPIO_WIFI_STATE | GPIO_MASK5)
#define GPIO_IR_MASK_GE_V2048  (GPIO_IR_RECEIVE | GPIO_IR_SEND | GPIO_MASK9 | GPIO_MASK7 | GPIO_MASK6)
#define GPIO_IR_MASK_GE_V0     (GPIO_IR_RECEIVE | GPIO_IR_SEND | GPIO_MASK7)
#define GPIO_NFC_MASK          (GPIO_MASK16     | GPIO_MASK13  | GPIO_MASK12)
#define GPIO_QTM_MASK          (GPIO_MASK17)

// Accessable categories of bits when accessing IO
#define GPIO_ACCESS_REG0 (GPIO_MASK2  | GPIO_MASK1  | GPIO_HID_PAD0)
#define GPIO_ACCESS_REG1 (GPIO_MASK4  | GPIO_MASK3)
#define GPIO_ACCESS_REG2 (GPIO_MASK5)
#define GPIO_ACCESS_REG3 (GPIO_MASK17 | GPIO_MASK16 | GPIO_MASK15 | GPIO_HID_PAD1 | GPIO_MASK13 | GPIO_MASK12 | GPIO_IR_RECEIVE | GPIO_IR_SEND | GPIO_MASK9 | GPIO_MASK8 | GPIO_MASK7 | GPIO_MASK6)
#define GPIO_ACCESS_REG4 (GPIO_ACCESS_REG3)
#define GPIO_ACCESS_REG5 (GPIO_MASK18)

// Result values
#define GPIO_NOT_AUTHORIZED MAKERESULT(RL_USAGE, RS_INVALIDARG, RM_GPIO, RD_NOT_AUTHORIZED)
#define GPIO_BUSY           MAKERESULT(RL_USAGE, RS_INVALIDARG, RM_GPIO, RD_BUSY)
#define GPIO_NOT_FOUND      MAKERESULT(RL_USAGE, RS_INVALIDARG, RM_GPIO, RD_NOT_FOUND)

// Result values, my additions edition:tm:
#define GPIO_INTERNAL_RANGE MAKERESULT(RL_FATAL, RS_INTERNAL, RM_GPIO, RD_OUT_OF_RANGE)
#define GPIO_CANCELED_RANGE MAKERESULT(RL_FATAL, RS_CANCELED, RM_GPIO, RD_OUT_OF_RANGE)
