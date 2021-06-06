/*
 * VDSO implementation for AArch64 and vector page setup for AArch32.
 *
 * Copyright (C) 2012 ARM Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Will Deacon <will.deacon@arm.com>
 */

#include <linux/kernel.h>
#include <linux/clocksource.h>
#include <linux/elf.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/gfp.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/slab.h>
#include <linux/timekeeper_internal.h>
#include <linux/vmalloc.h>

#include <asm/cacheflush.h>
#include <asm/signal32.h>
#include <asm/vdso.h>
#include <asm/vdso_datapage.h>

<<<<<<< HEAD   (f93102 Revert "arch: arm64: configs: exynos8890_*_defconfig: enable)
extern char vdso_start, vdso_end;
static unsigned long vdso_pages;
static struct page **vdso_pagelist;
=======
struct vdso_mappings {
	unsigned long num_code_pages;
	struct vm_special_mapping data_mapping;
	struct vm_special_mapping code_mapping;
};
>>>>>>> CHANGE (5709ca FROMLIST: BACKPORT: [PATCH 3/6] arm64: Refactor vDSO init/se)

/*
 * The vDSO data page.
 */
static union {
	struct vdso_data	data;
	u8			page[PAGE_SIZE];
} vdso_data_store __page_aligned_data;
struct vdso_data *vdso_data = &vdso_data_store.data;

#ifdef CONFIG_COMPAT
/*
 * Create and map the vectors page for AArch32 tasks.
 */
static struct page *vectors_page[1];

static int __init alloc_vectors_page(void)
{
	extern char __kuser_helper_start[], __kuser_helper_end[];
	extern char __aarch32_sigret_code_start[], __aarch32_sigret_code_end[];

	int kuser_sz = __kuser_helper_end - __kuser_helper_start;
	int sigret_sz = __aarch32_sigret_code_end - __aarch32_sigret_code_start;
	unsigned long vpage;

	vpage = get_zeroed_page(GFP_ATOMIC);

	if (!vpage)
		return -ENOMEM;

	/* kuser helpers */
	memcpy((void *)vpage + 0x1000 - kuser_sz, __kuser_helper_start,
		kuser_sz);

	/* sigreturn code */
	memcpy((void *)vpage + AARCH32_KERN_SIGRET_CODE_OFFSET,
               __aarch32_sigret_code_start, sigret_sz);

	flush_icache_range(vpage, vpage + PAGE_SIZE);
	vectors_page[0] = virt_to_page(vpage);

	return 0;
}
arch_initcall(alloc_vectors_page);

int aarch32_setup_vectors_page(struct linux_binprm *bprm, int uses_interp)
{
	struct mm_struct *mm = current->mm;
	unsigned long addr = AARCH32_VECTORS_BASE;
	static const struct vm_special_mapping spec = {
		.name	= "[vectors]",
		.pages	= vectors_page,

	};
	void *ret;

	down_write(&mm->mmap_sem);
	current->mm->context.vdso = (void *)addr;

	/* Map vectors page at the high address. */
	ret = _install_special_mapping(mm, addr, PAGE_SIZE,
				       VM_READ|VM_EXEC|VM_MAYREAD|VM_MAYEXEC,
				       &spec);

	up_write(&mm->mmap_sem);

	return PTR_ERR_OR_ZERO(ret);
}
#endif /* CONFIG_COMPAT */

<<<<<<< HEAD   (f93102 Revert "arch: arm64: configs: exynos8890_*_defconfig: enable)
static struct vm_special_mapping vdso_spec[2];

static int __init vdso_init(void)
=======
static int __init vdso_mappings_init(const char *name,
				     const char *code_start,
				     const char *code_end,
				     struct vdso_mappings *mappings)
>>>>>>> CHANGE (5709ca FROMLIST: BACKPORT: [PATCH 3/6] arm64: Refactor vDSO init/se)
{
<<<<<<< HEAD   (f93102 Revert "arch: arm64: configs: exynos8890_*_defconfig: enable)
	int i;
=======
	unsigned long i, vdso_pages;
	struct page **vdso_pagelist;
>>>>>>> CHANGE (5709ca FROMLIST: BACKPORT: [PATCH 3/6] arm64: Refactor vDSO init/se)
	unsigned long pfn;

<<<<<<< HEAD   (f93102 Revert "arch: arm64: configs: exynos8890_*_defconfig: enable)
	if (memcmp(&vdso_start, "\177ELF", 4)) {
		pr_err("vDSO is not a valid ELF object!\n");
=======
	if (memcmp(code_start, "\177ELF", 4)) {
		pr_err("%s is not a valid ELF object!\n", name);
>>>>>>> CHANGE (5709ca FROMLIST: BACKPORT: [PATCH 3/6] arm64: Refactor vDSO init/se)
		return -EINVAL;
	}

<<<<<<< HEAD   (f93102 Revert "arch: arm64: configs: exynos8890_*_defconfig: enable)
	vdso_pages = (&vdso_end - &vdso_start) >> PAGE_SHIFT;
	pr_info("vdso: %ld pages (%ld code @ %p, %ld data @ %p)\n",
		vdso_pages + 1, vdso_pages, &vdso_start, 1L, vdso_data);
=======
	vdso_pages = (code_end - code_start) >> PAGE_SHIFT;
	pr_info("%s: %ld pages (%ld code @ %p, %ld data @ %p)\n",
		name, vdso_pages + 1, vdso_pages, code_start, 1L,
		vdso_data);
>>>>>>> CHANGE (5709ca FROMLIST: BACKPORT: [PATCH 3/6] arm64: Refactor vDSO init/se)

	/*
	 * Allocate space for storing pointers to the vDSO code pages + the
	 * data page. The pointers must have the same lifetime as the mappings,
	 * which are static, so there is no need to keep track of the pointer
	 * array to free it.
	 */
	vdso_pagelist = kmalloc_array(vdso_pages + 1, sizeof(struct page *),
				      GFP_KERNEL);
	if (vdso_pagelist == NULL)
		return -ENOMEM;

	/* Grab the vDSO data page. */
	vdso_pagelist[0] = phys_to_page(__pa_symbol(vdso_data));

	/* Grab the vDSO code pages. */
<<<<<<< HEAD   (f93102 Revert "arch: arm64: configs: exynos8890_*_defconfig: enable)
	pfn = sym_to_pfn(&vdso_start);
=======
	pfn = sym_to_pfn(code_start);
>>>>>>> CHANGE (5709ca FROMLIST: BACKPORT: [PATCH 3/6] arm64: Refactor vDSO init/se)

	for (i = 0; i < vdso_pages; i++)
		vdso_pagelist[i + 1] = pfn_to_page(pfn + i);

	/* Populate the special mapping structures */
<<<<<<< HEAD   (f93102 Revert "arch: arm64: configs: exynos8890_*_defconfig: enable)
	vdso_spec[0] = (struct vm_special_mapping) {
		.name	= "[vvar]",
		.pages	= vdso_pagelist,
	};

	vdso_spec[1] = (struct vm_special_mapping) {
		.name	= "[vdso]",
		.pages	= &vdso_pagelist[1],
=======
	mappings->data_mapping = (struct vm_special_mapping) {
		.name	= "[vvar]",
		.pages	= &vdso_pagelist[0],
>>>>>>> CHANGE (5709ca FROMLIST: BACKPORT: [PATCH 3/6] arm64: Refactor vDSO init/se)
	};

	mappings->code_mapping = (struct vm_special_mapping) {
		.name	= "[vdso]",
		.pages	= &vdso_pagelist[1],
	};

	mappings->num_code_pages = vdso_pages;
	return 0;
}

static struct vdso_mappings vdso_mappings __ro_after_init;

static int __init vdso_init(void)
{
	extern char vdso_start[], vdso_end[];

	return vdso_mappings_init("vdso", vdso_start, vdso_end,
				  &vdso_mappings);
}
arch_initcall(vdso_init);

static int vdso_setup(struct mm_struct *mm,
		      const struct vdso_mappings *mappings)
{
	unsigned long vdso_base, vdso_text_len, vdso_mapping_len;
	void *ret;

	vdso_text_len = mappings->num_code_pages << PAGE_SHIFT;
	/* Be sure to map the data page */
	vdso_mapping_len = vdso_text_len + PAGE_SIZE;

	vdso_base = get_unmapped_area(NULL, 0, vdso_mapping_len, 0, 0);
	if (IS_ERR_VALUE(vdso_base))
		return PTR_ERR_OR_ZERO(ERR_PTR(vdso_base));
	ret = _install_special_mapping(mm, vdso_base, PAGE_SIZE,
				       VM_READ|VM_MAYREAD,
				       &mappings->data_mapping);
	if (IS_ERR(ret))
		return PTR_ERR_OR_ZERO(ret);

	vdso_base += PAGE_SIZE;
	ret = _install_special_mapping(mm, vdso_base, vdso_text_len,
				       VM_READ|VM_EXEC|
				       VM_MAYREAD|VM_MAYWRITE|VM_MAYEXEC,
				       &mappings->code_mapping);
	if (!IS_ERR(ret))
		mm->context.vdso = (void *)vdso_base;

	return PTR_ERR_OR_ZERO(ret);
}

int arch_setup_additional_pages(struct linux_binprm *bprm, int uses_interp)
{
	struct mm_struct *mm = current->mm;
	int ret;

        down_write(&mm->mmap_sem);

	ret = vdso_setup(mm, &vdso_mappings);

	up_write(&mm->mmap_sem);
	return ret;
}

/*
 * Update the vDSO data page to keep in sync with kernel timekeeping.
 */
void update_vsyscall(struct timekeeper *tk)
{
	struct timespec xtime_coarse;
	u32 use_syscall = strcmp(tk->tkr_mono.clock->name, "arch_sys_counter");

	++vdso_data->tb_seq_count;
	smp_wmb();

	xtime_coarse = __current_kernel_time();
	vdso_data->use_syscall			= use_syscall;
	vdso_data->xtime_coarse_sec		= xtime_coarse.tv_sec;
	vdso_data->xtime_coarse_nsec		= xtime_coarse.tv_nsec;
	vdso_data->wtm_clock_sec		= tk->wall_to_monotonic.tv_sec;
	vdso_data->wtm_clock_nsec		= tk->wall_to_monotonic.tv_nsec;

	if (!use_syscall) {
		/* tkr_mono.cycle_last == tkr_raw.cycle_last */
		vdso_data->cs_cycle_last	= tk->tkr_mono.cycle_last;
		vdso_data->raw_time_sec		= tk->raw_sec;
		vdso_data->raw_time_nsec	= tk->tkr_raw.xtime_nsec;
		vdso_data->xtime_clock_sec	= tk->xtime_sec;
		vdso_data->xtime_clock_nsec	= tk->tkr_mono.xtime_nsec;
		/* tkr_raw.xtime_nsec == 0 */
		vdso_data->cs_mono_mult		= tk->tkr_mono.mult;
		vdso_data->cs_raw_mult		= tk->tkr_raw.mult;
		/* tkr_mono.shift == tkr_raw.shift */
		vdso_data->cs_shift		= tk->tkr_mono.shift;
	}

	smp_wmb();
	++vdso_data->tb_seq_count;
}

void update_vsyscall_tz(void)
{
	vdso_data->tz_minuteswest	= sys_tz.tz_minuteswest;
	vdso_data->tz_dsttime		= sys_tz.tz_dsttime;
}
