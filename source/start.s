	.arch armv6k
	.section .init, "ax", %progbits
	.arm
	.align  2
	.global _start
	.syntax unified
	.type   _start, %function
_start:
	bl      GPIOMain
	svc     0x03
	.size   _start, .-_start
