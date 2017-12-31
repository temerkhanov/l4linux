
#include <linux/linkage.h>

ENTRY(stub32_clone)
	/*
	 * The 32-bit clone ABI is: clone(..., int tls_val, int *child_tidptr).
	 * The 64-bit clone ABI is: clone(..., int *child_tidptr, int tls_val).
	 *
	 * The native 64-bit kernel's sys_clone() implements the latter,
	 * so we need to swap arguments here before calling it:
	 */
	xchg	%r8, %rcx
	jmp	sys_clone
ENDPROC(stub32_clone)


.macro mkstub name, compat
	ALIGN
GLOBAL(stub32_\name)
	jmp \compat
.endm

mkstub fork, sys_fork
mkstub vfork, sys_vfork
mkstub execve, compat_sys_execve
mkstub execveat, compat_sys_execveat
mkstub rt_sigreturn, sys32_rt_sigreturn
mkstub sigreturn, sys32_sigreturn

/* For traps.c */
ENTRY(entry_SYSENTER_compat)
	nop
GLOBAL(__end_entry_SYSENTER_compat)
ENDPROC(entry_SYSENTER_compat)
