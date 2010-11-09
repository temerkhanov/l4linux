#
# Makefile for x86 specific library files.
#

b := ../../../x86/lib

inat_tables_script = $(srctree)/arch/x86/tools/gen-insn-attr-x86.awk
inat_tables_maps = $(srctree)/arch/x86/lib/x86-opcode-map.txt
quiet_cmd_inat_tables = GEN     $@
      cmd_inat_tables = $(AWK) -f $(inat_tables_script) $(inat_tables_maps) > $@ || rm -f $@

$(obj)/inat-tables.c: $(inat_tables_script) $(inat_tables_maps)
	$(call cmd,inat_tables)

$(obj)/$(b)/inat.o: $(obj)/inat-tables.c

clean-files := inat-tables.c

obj-$(CONFIG_SMP) += msr-smp.o cache-smp.o

KBUILD_CFLAGS += -Iarch/x86/lib

lib-y := $(b)/delay.o

lib-y += ../memcpy_user.o ../uaccess.o ../pte.o
lib-y += memcpy.o
lib-$(CONFIG_INSTRUCTION_DECODER) += $(b)/insn.o $(b)/inat.o

obj-y += msr.o msr-reg.o msr-reg-export.o

ifeq ($(CONFIG_X86_32),y)
        obj-y += atomic64_32.o
        lib-y += $(b)/atomic64_cx8_32.o
	lib-y += checksum.o
        lib-y += $(b)/strstr_32.o
        lib-y += $(b)/semaphore_32.o $(b)/string_32.o
	lib-y += $(b)/cmpxchg.o
ifneq ($(CONFIG_X86_CMPXCHG64),y)
        lib-y += $(b)/cmpxchg8b_emu.o $(b)/atomic64_386_32.o
endif
        lib-$(CONFIG_X86_USE_3DNOW) += $(b)/mmx_32.o
else
        obj-y += iomap_copy_64.o
        lib-y += csum-partial_64.o csum-copy_64.o csum-wrappers_64.o
        lib-y += thunk_64.o clear_page_64.o copy_page_64.o
        lib-y += memmove_64.o memset_64.o
        lib-y += copy_user_64.o rwlock_64.o copy_user_nocache_64.o
	lib-$(CONFIG_RWSEM_XCHGADD_ALGORITHM) += rwsem_64.o
endif

obj-y                  += proc.o
obj-$(CONFIG_L4_DEBUG) += ../debugfs.o

msr-y                  := $(b)/msr.o
msr-smp-y              := $(b)/msr-smp.o
msr-reg-y              := $(b)/msr-reg.o
msr-reg-export-y       := $(b)/msr-reg-export.o
proc-y                 := ../proc.o
atomic64_32-y          := $(b)/atomic64_32.o
cache-smp-y            := $(b)/cache-smp.o

create_dir:
	$(Q)mkdir -p $(obj)/../../../x86/lib

PHONY += create_dir

$(obj)/../../../x86/lib/delay.o: create_dir