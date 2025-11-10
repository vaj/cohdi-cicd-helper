// SPDX-License-Identifier: GPL-2.0+
/*
 * restrict_kmod: restrict loading of kernel modules
 *
 * Copyright (C) 2025 VA Linux Systems Japan K.K.
 */

#include <linux/module.h>
#include <linux/livepatch.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/nsproxy.h>
#include <linux/sysctl.h>
#include <linux/version.h>
#include <net/net_namespace.h>

static bool restricted = 1;

int rk_module_enforce_rwx_sections(Elf_Ehdr *, Elf_Shdr *,
                                   char *, struct module *);

/* copied from kernel/module/strict_rwx.c , except for the first if(){}*/
int rk_module_enforce_rwx_sections(Elf_Ehdr *hdr, Elf_Shdr *sechdrs,
                                   char *secstrings, struct module *mod)
{
	const unsigned long shf_wx = SHF_WRITE | SHF_EXECINSTR;
	int i;

#ifdef RKDEBUG
	if (current) {
		int i = 0;
		struct task_struct *p = current;
		char *spc = "        <=";
		while (p && i < 10) {
			printk("rk: %s%s[%d] netns %lx, init %lx\n",
			       &spc[10-i],
			       p->comm, p->pid,
			       (long int)current->nsproxy->net_ns,
			       (long int)&init_net);
			i++;
			p = p->real_parent;
			if (p->pid == 1) break;
		}
	} else {
		printk("rk: called from irq context.\n");
	}
#endif
	if (restricted && current &&
	    current->nsproxy->net_ns != &init_net) {
		printk("rk: %s[%d] is going to load a module; blocking it\n",
		       current->comm, current->pid);
		return -EPERM;
	}

	if (!IS_ENABLED(CONFIG_STRICT_MODULE_RWX))
		return 0;

	for (i = 0; i < hdr->e_shnum; i++) {
		if ((sechdrs[i].sh_flags & shf_wx) == shf_wx) {
			pr_err("%s: section %s (index %d) has invalid WRITE|EXEC flags\n",
			       mod->name, secstrings + sechdrs[i].sh_name, i);
			return -ENOEXEC;
		}
	}

	return 0;
}

static struct ctl_table sw[] = {
	{
		.procname = "restrict_kmod",
		.data     = &restricted,
		.maxlen   = sizeof(bool),
		.mode     = 0644,
		.proc_handler = proc_dobool,
	},
	{}
};
static struct ctl_table_header *sw_header;

/* and below lines are mostly copied from livepatch-sample.c */
static struct klp_func funcs[] = {
	{
		.old_name = "module_enforce_rwx_sections",
		.new_func = rk_module_enforce_rwx_sections,
	}, { }
};

static struct klp_object objs[] = {
	{
		/* name being NULL means vmlinux */
		.funcs = funcs,
	}, { }
};

static struct klp_patch patch = {
	.mod = THIS_MODULE,
	.objs = objs,
};

static int restrict_kmod_init(void)
{
	sw_header = register_sysctl("kernel", sw);
	if (sw_header == 0) {
		printk("rk: Failed to register sysctl\n");
		return -ENOMEM;
	}
	return klp_enable_patch(&patch);
}

static void restrict_kmod_exit(void)
{
	unregister_sysctl_table(sw_header);
}

module_init(restrict_kmod_init);
module_exit(restrict_kmod_exit);
MODULE_LICENSE("GPL");
MODULE_INFO(livepatch, "Y");
