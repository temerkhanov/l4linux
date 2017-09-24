#pragma once

#include <asm/thread_info.h>

static inline
unsigned long l4x_current_stack_pointer(void)
{
	return current_stack_pointer();
}
