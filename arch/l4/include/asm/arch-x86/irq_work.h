#ifndef _ASM_IRQ_WORK_H
#define _ASM_IRQ_WORK_H

#ifndef CONFIG_L4
#include <asm/cpufeature.h>
#endif

static inline bool arch_irq_work_has_interrupt(void)
{
#ifdef CONFIG_L4
	return true;
#else
	return boot_cpu_has(X86_FEATURE_APIC);
#endif
}

#endif /* _ASM_IRQ_WORK_H */
