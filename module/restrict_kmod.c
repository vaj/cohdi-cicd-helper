// SPDX-License-Identifier: GPL-2.0+
/*
 * restrict_kmod: restrict loading of kernel modules
 *
 * Copyright (C) 2025 VA Linux Systems Japan K.K.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/livepatch.h>
#include <linux/printk.h>
#include <linux/module_signature.h>
#include <linux/nsproxy.h>
#include <linux/sysctl.h>
#include <net/net_namespace.h>

static bool restricted = 1;
int rk_mod_check_sig(const struct module_signature *, size_t, const char *);

/* copied from kernel/module_signature.c , except for the first if(){}*/
int rk_mod_check_sig(const struct module_signature *ms, size_t file_len,
		     const char *name)
{
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

	if (be32_to_cpu(ms->sig_len) >= file_len - sizeof(*ms))
		return -EBADMSG;

	if (ms->id_type != PKEY_ID_PKCS7) {
		pr_err("%s: not signed with expected PKCS#7 message\n",
		       name);
		return -ENOPKG;
	}

	if (ms->algo != 0 ||
	    ms->hash != 0 ||
	    ms->signer_len != 0 ||
	    ms->key_id_len != 0 ||
	    ms->__pad[0] != 0 ||
	    ms->__pad[1] != 0 ||
	    ms->__pad[2] != 0) {
		pr_err("%s: PKCS#7 signature info has unexpected non-zero params\n",
		       name);
		return -EBADMSG;
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
		.old_name = "mod_check_sig",
		.new_func = rk_mod_check_sig,
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
}

module_init(restrict_kmod_init);
module_exit(restrict_kmod_exit);
MODULE_LICENSE("GPL");
MODULE_INFO(livepatch, "Y");
