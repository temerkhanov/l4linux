#ifndef _ASM_X86_PGTABLE_64_H
#define _ASM_X86_PGTABLE_64_H

#include <linux/const.h>
#include <asm/pgtable_64_types.h>

#ifndef __ASSEMBLY__

/*
 * This file contains the functions and defines necessary to modify and use
 * the x86-64 page table tree.
 */
#include <asm/processor.h>
#include <linux/bitops.h>
#include <linux/threads.h>

extern p4d_t level4_kernel_pgt[512];
extern p4d_t level4_ident_pgt[512];
extern pud_t level3_kernel_pgt[512];
extern pud_t level3_ident_pgt[512];
extern pmd_t level2_kernel_pgt[512];
extern pmd_t level2_fixmap_pgt[512];
extern pmd_t level2_ident_pgt[512];
extern pte_t level1_fixmap_pgt[512];
extern pgd_t init_top_pgt[];

#define swapper_pg_dir init_top_pgt

extern void paging_init(void);

#define pte_ERROR(e)					\
	pr_err("%s:%d: bad pte %p(%016lx)\n",		\
	       __FILE__, __LINE__, &(e), pte_val(e))
#define pmd_ERROR(e)					\
	pr_err("%s:%d: bad pmd %p(%016lx)\n",		\
	       __FILE__, __LINE__, &(e), pmd_val(e))
#define pud_ERROR(e)					\
	pr_err("%s:%d: bad pud %p(%016lx)\n",		\
	       __FILE__, __LINE__, &(e), pud_val(e))

#if CONFIG_PGTABLE_LEVELS >= 5
#define p4d_ERROR(e)					\
	pr_err("%s:%d: bad p4d %p(%016lx)\n",		\
	       __FILE__, __LINE__, &(e), p4d_val(e))
#endif

#define pgd_ERROR(e)					\
	pr_err("%s:%d: bad pgd %p(%016lx)\n",		\
	       __FILE__, __LINE__, &(e), pgd_val(e))

struct mm_struct;

void set_pte_vaddr_p4d(p4d_t *p4d_page, unsigned long vaddr, pte_t new_pte);
void set_pte_vaddr_pud(pud_t *pud_page, unsigned long vaddr, pte_t new_pte);

#ifdef CONFIG_L4
/*
 * L4Linux uses this hook to synchronize the Linux page tables with
 * the real hardware page tables kept by the L4 kernel.
 */
unsigned long l4x_set_pte(struct mm_struct *mm, unsigned long addr, pte_t pteptr, pte_t pteval);
void          l4x_pte_clear(struct mm_struct *mm, unsigned long addr, pte_t ptep);

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
unsigned long l4x_set_pmd(struct mm_struct *mm, unsigned long addr, pmd_t pmdptr, pmd_t pmdval);
void          l4x_pmd_clear(struct mm_struct *mm, unsigned long addr, pmd_t pmdp);
#endif

static inline void __l4x_set_pte(struct mm_struct *mm, unsigned long addr,
                                 pte_t *pteptr, pte_t pteval)
{
	if (pte_val(*pteptr) & _PAGE_PRESENT)
		pteval = native_make_pte(l4x_set_pte(mm, addr, *pteptr, pteval));
	*pteptr = pteval;
}

static inline int
l4x_pmd_present(pmd_t *pmdptr)
{
	return (pmd_val(*pmdptr) & (_PAGE_PRESENT | _PAGE_PSE)) == (_PAGE_PRESENT | _PAGE_PSE);
}

static inline void __l4x_set_pmd(struct mm_struct *mm, unsigned long addr,
                                 pmd_t *pmdptr, pmd_t pmdval)
{
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	if (l4x_pmd_present(pmdptr) && pmd_large(*pmdptr))
		pmdval = native_make_pmd(l4x_set_pmd(mm, addr, *pmdptr, pmdval));
#endif
	*pmdptr = pmdval;
}

static inline void __l4x_set_pud(struct mm_struct *mm, unsigned long addr,
                                 pud_t *pudptr, pud_t pudval)
{
	*pudptr = pudval;
}
#endif /* L4 */


static inline void native_pte_clear(struct mm_struct *mm, unsigned long addr,
				    pte_t *ptep)
{
#ifdef CONFIG_L4
	if (pte_val(*ptep) & _PAGE_PRESENT)
		l4x_pte_clear(mm, addr, *ptep);
#endif /* L4 */
	*ptep = native_make_pte(0);
}

static inline void native_set_pte(pte_t *ptep, pte_t pte)
{
#ifdef CONFIG_L4
	__l4x_set_pte(NULL, 0, ptep, pte);
#else
	*ptep = pte;
#endif /* L4 */
}

static inline void native_set_pte_atomic(pte_t *ptep, pte_t pte)
{
	native_set_pte(ptep, pte);
}

static inline void native_set_pmd(pmd_t *pmdp, pmd_t pmd)
{
#ifdef CONFIG_L4
	__l4x_set_pmd(NULL, 0, pmdp, pmd);
#else
	*pmdp = pmd;
#endif /* L4 */
}

static inline void native_pmd_clear(pmd_t *pmd)
{
#ifdef CONFIG_L4
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	if (l4x_pmd_present(pmd))
		l4x_pmd_clear(NULL, 0, *pmd);
#endif
#endif /* L4 */
	native_set_pmd(pmd, native_make_pmd(0));
}

static inline pte_t native_ptep_get_and_clear(pte_t *xp)
{
#ifdef CONFIG_SMP
	return native_make_pte(xchg(&xp->pte, 0));
#else
	/* native_local_ptep_get_and_clear,
	   but duplicated because of cyclic dependency */
	pte_t ret = *xp;
	native_pte_clear(NULL, 0, xp);
	return ret;
#endif
}

static inline pmd_t native_pmdp_get_and_clear(pmd_t *xp)
{
#ifdef CONFIG_SMP
	return native_make_pmd(xchg(&xp->pmd, 0));
#else
	/* native_local_pmdp_get_and_clear,
	   but duplicated because of cyclic dependency */
	pmd_t ret = *xp;
	native_pmd_clear(xp);
	return ret;
#endif
}

static inline void native_set_pud(pud_t *pudp, pud_t pud)
{
	*pudp = pud;
}

static inline void native_pud_clear(pud_t *pud)
{
	native_set_pud(pud, native_make_pud(0));
}

static inline pud_t native_pudp_get_and_clear(pud_t *xp)
{
#ifdef CONFIG_SMP
	return native_make_pud(xchg(&xp->pud, 0));
#else
	/* native_local_pudp_get_and_clear,
	 * but duplicated because of cyclic dependency
	 */
	pud_t ret = *xp;

	native_pud_clear(xp);
	return ret;
#endif
}

static inline void native_set_p4d(p4d_t *p4dp, p4d_t p4d)
{
	*p4dp = p4d;
}

static inline void native_p4d_clear(p4d_t *p4d)
{
#ifdef CONFIG_X86_5LEVEL
	native_set_p4d(p4d, native_make_p4d(0));
#else
	native_set_p4d(p4d, (p4d_t) { .pgd = native_make_pgd(0)});
#endif
}

static inline void native_set_pgd(pgd_t *pgdp, pgd_t pgd)
{
	*pgdp = pgd;
}

static inline void native_pgd_clear(pgd_t *pgd)
{
	native_set_pgd(pgd, native_make_pgd(0));
}

extern void sync_global_pgds(unsigned long start, unsigned long end);

/*
 * Conversion functions: convert a page and protection to a page entry,
 * and a page entry and page directory to the page they refer to.
 */

/*
 * Level 4 access.
 */
static inline int pgd_large(pgd_t pgd) { return 0; }
#define mk_kernel_pgd(address) __pgd((address) | _KERNPG_TABLE)

/* PUD - Level3 access */

/* PMD  - Level 2 access */

/* PTE - Level 1 access. */

/* x86-64 always has all page tables mapped. */
#define pte_offset_map(dir, address) pte_offset_kernel((dir), (address))
#define pte_unmap(pte) ((void)(pte))/* NOP */

/*
 * Encode and de-code a swap entry
 *
 * |     ...            | 11| 10|  9|8|7|6|5| 4| 3|2|1|0| <- bit number
 * |     ...            |SW3|SW2|SW1|G|L|D|A|CD|WT|U|W|P| <- bit names
 * | OFFSET (14->63) | TYPE (9-13)  |0|X|X|X| X| X|X|X|0| <- swp entry
 *
 * G (8) is aliased and used as a PROT_NONE indicator for
 * !present ptes.  We need to start storing swap entries above
 * there.  We also need to avoid using A and D because of an
 * erratum where they can be incorrectly set by hardware on
 * non-present PTEs.
 */
#define SWP_TYPE_FIRST_BIT (_PAGE_BIT_PROTNONE + 1)
#define SWP_TYPE_BITS 5
/* Place the offset above the type: */
#define SWP_OFFSET_FIRST_BIT (SWP_TYPE_FIRST_BIT + SWP_TYPE_BITS)

#define MAX_SWAPFILES_CHECK() BUILD_BUG_ON(MAX_SWAPFILES_SHIFT > SWP_TYPE_BITS)

#define __swp_type(x)			(((x).val >> (SWP_TYPE_FIRST_BIT)) \
					 & ((1U << SWP_TYPE_BITS) - 1))
#define __swp_offset(x)			((x).val >> SWP_OFFSET_FIRST_BIT)
#define __swp_entry(type, offset)	((swp_entry_t) { \
					 ((type) << (SWP_TYPE_FIRST_BIT)) \
					 | ((offset) << SWP_OFFSET_FIRST_BIT) })
#define __pte_to_swp_entry(pte)		((swp_entry_t) { pte_val((pte)) })
#define __swp_entry_to_pte(x)		((pte_t) { .pte = (x).val })

extern int kern_addr_valid(unsigned long addr);
extern void cleanup_highmap(void);

#define HAVE_ARCH_UNMAPPED_AREA
#define HAVE_ARCH_UNMAPPED_AREA_TOPDOWN

#define pgtable_cache_init()   do { } while (0)
#define check_pgt_cache()      do { } while (0)

#define PAGE_AGP    PAGE_KERNEL_NOCACHE
#define HAVE_PAGE_AGP 1

/* fs/proc/kcore.c */
#define	kc_vaddr_to_offset(v) ((v) & __VIRTUAL_MASK)
#define	kc_offset_to_vaddr(o) ((o) | ~__VIRTUAL_MASK)

#define __HAVE_ARCH_PTE_SAME

#define vmemmap ((struct page *)VMEMMAP_START)

extern void init_extra_mapping_uc(unsigned long phys, unsigned long size);
extern void init_extra_mapping_wb(unsigned long phys, unsigned long size);

#define gup_fast_permitted gup_fast_permitted
static inline bool gup_fast_permitted(unsigned long start, int nr_pages,
		int write)
{
	unsigned long len, end;

	len = (unsigned long)nr_pages << PAGE_SHIFT;
	end = start + len;
	if (end < start)
		return false;
	if (end >> __VIRTUAL_MASK_SHIFT)
		return false;
	return true;
}

#endif /* !__ASSEMBLY__ */
#endif /* _ASM_X86_PGTABLE_64_H */
