#include <linux/dcache.h>
#include <linux/security.h>
#include <asm/current.h>
#include <linux/cred.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/kprobes.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/task_stack.h>
#else
#include <linux/sched.h>
#endif

#include "objsec.h"
#include "allowlist.h"
#include "arch.h"
#include "klog.h" // IWYU pragma: keep
#include "ksud.h"
#include "kernel_compat.h"

#define SU_PATH "/system/bin/su"
#define SH_PATH "/system/bin/sh"

extern void escape_to_root();

static void __user *userspace_stack_buffer(const void *d, size_t len)
{
	/* To avoid having to mmap a page in userspace, just write below the stack
   * pointer. */
	char __user *p = (void __user *)current_user_stack_pointer() - len;

	return copy_to_user(p, d, len) ? NULL : p;
}

static char __user *sh_user_path(void)
{
	static const char sh_path[] = "/system/bin/sh";

	return userspace_stack_buffer(sh_path, sizeof(sh_path));
}

static char __user *ksud_user_path(void)
{
	static const char ksud_path[] = KSUD_PATH;

	return userspace_stack_buffer(ksud_path, sizeof(ksud_path));
}

int ksu_handle_faccessat(int *dfd, const char __user **filename_user, int *mode,
			 int *__unused_flags)
{
	const char su[] = SU_PATH;

	if (!ksu_is_allow_uid(current_uid().val)) {
		return 0;
	}

	char path[sizeof(su) + 1];
	memset(path, 0, sizeof(path));
	ksu_strncpy_from_user_nofault(path, *filename_user, sizeof(path));

	if (unlikely(!memcmp(path, su, sizeof(su)))) {
		pr_info("faccessat su->sh!\n");
		*filename_user = sh_user_path();
	}

	return 0;
}

int ksu_handle_stat(int *dfd, const char __user **filename_user, int *flags)
{
	// const char sh[] = SH_PATH;
	const char su[] = SU_PATH;

	if (!ksu_is_allow_uid(current_uid().val)) {
		return 0;
	}

	if (unlikely(!filename_user)) {
		return 0;
	}

	char path[sizeof(su) + 1];
	memset(path, 0, sizeof(path));
// Remove this later!! we use syscall hook, so this will never happen!!!!!
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0) && 0
	// it becomes a `struct filename *` after 5.18
	// https://elixir.bootlin.com/linux/v5.18/source/fs/stat.c#L216
	const char sh[] = SH_PATH;
	struct filename *filename = *((struct filename **)filename_user);
	if (IS_ERR(filename)) {
		return 0;
	}
	if (likely(memcmp(filename->name, su, sizeof(su))))
		return 0;
	pr_info("vfs_statx su->sh!\n");
	memcpy((void *)filename->name, sh, sizeof(sh));
#else
	ksu_strncpy_from_user_nofault(path, *filename_user, sizeof(path));

	if (unlikely(!memcmp(path, su, sizeof(su)))) {
		pr_info("newfstatat su->sh!\n");
		*filename_user = sh_user_path();
	}
#endif

	return 0;
}

// the call from execve_handler_pre won't provided correct value for __never_use_argument, use them after fix execve_handler_pre, keeping them for consistence for manually patched code
int ksu_handle_execveat_sucompat(int *fd, struct filename **filename_ptr,
				 void *__never_use_argv, void *__never_use_envp,
				 int *__never_use_flags)
{
	struct filename *filename;
	const char sh[] = KSUD_PATH;
	const char su[] = SU_PATH;

	if (unlikely(!filename_ptr))
		return 0;

	filename = *filename_ptr;
	if (IS_ERR(filename)) {
		return 0;
	}

	if (likely(memcmp(filename->name, su, sizeof(su))))
		return 0;

	if (!ksu_is_allow_uid(current_uid().val))
		return 0;

	pr_info("do_execveat_common su found\n");
	memcpy((void *)filename->name, sh, sizeof(sh));

	escape_to_root();

	return 0;
}

int ksu_handle_execve_sucompat(int *fd, const char __user **filename_user,
			       void *__never_use_argv, void *__never_use_envp,
			       int *__never_use_flags)
{
	const char su[] = SU_PATH;
	char path[sizeof(su) + 1];

	if (unlikely(!filename_user))
		return 0;

	memset(path, 0, sizeof(path));
	ksu_strncpy_from_user_nofault(path, *filename_user, sizeof(path));

	if (likely(memcmp(path, su, sizeof(su))))
		return 0;

	if (!ksu_is_allow_uid(current_uid().val))
		return 0;

	pr_info("sys_execve su found\n");
	*filename_user = ksud_user_path();

	escape_to_root();

	return 0;
}

#ifdef CONFIG_KPROBES

__maybe_unused static int faccessat_handler_pre(struct kprobe *p,
						struct pt_regs *regs)
{
	int *dfd = (int *)&PT_REGS_PARM1(regs);
	const char __user **filename_user = (const char **)&PT_REGS_PARM2(regs);
	int *mode = (int *)&PT_REGS_PARM3(regs);
	// Both sys_ and do_ is C function
	int *flags = (int *)&PT_REGS_CCALL_PARM4(regs);

	return ksu_handle_faccessat(dfd, filename_user, mode, flags);
}

static int faccessat_handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	struct pt_regs *real_regs = PT_REAL_REGS(regs);
	int *dfd = (int *)&PT_REGS_PARM1(real_regs);
	const char __user **filename_user =
		(const char **)&PT_REGS_PARM2(real_regs);
	int *mode = (int *)&PT_REGS_PARM3(real_regs);

	return ksu_handle_faccessat(dfd, filename_user, mode, NULL);
}

__maybe_unused static int newfstatat_handler_pre(struct kprobe *p,
						 struct pt_regs *regs)
{
	int *dfd = (int *)&PT_REGS_PARM1(regs);
	const char __user **filename_user = (const char **)&PT_REGS_PARM2(regs);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
	// static int vfs_statx(int dfd, const char __user *filename, int flags, struct kstat *stat, u32 request_mask)
	int *flags = (int *)&PT_REGS_PARM3(regs);
#else
	// int vfs_fstatat(int dfd, const char __user *filename, struct kstat *stat,int flag)
	int *flags = (int *)&PT_REGS_CCALL_PARM4(regs);
#endif

	return ksu_handle_stat(dfd, filename_user, flags);
}

static int newfstatat_handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	struct pt_regs *real_regs = PT_REAL_REGS(regs);
	int *dfd = (int *)&PT_REGS_PARM1(real_regs);
	const char __user **filename_user =
		(const char **)&PT_REGS_PARM2(real_regs);
	int *flags = (int *)&PT_REGS_SYSCALL_PARM4(real_regs);

	return ksu_handle_stat(dfd, filename_user, flags);
}

// https://elixir.bootlin.com/linux/v5.10.158/source/fs/exec.c#L1864
static int execve_handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	int *fd = (int *)&PT_REGS_PARM1(regs);
	struct filename **filename_ptr =
		(struct filename **)&PT_REGS_PARM2(regs);

	return ksu_handle_execveat_sucompat(fd, filename_ptr, NULL, NULL, NULL);
}

#ifdef CONFIG_KSU_SELINUX_NOENFORCING
static loff_t enforce_handler_pos_val;

static int enforce_handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	loff_t **pos_ptr = (loff_t **)&PT_REGS_CCALL_PARM4(regs);
	enforce_handler_pos_val = **pos_ptr;
	return 0;
}
static void enforce_handler_post(struct kprobe *p, struct pt_regs *regs, unsigned long i)
{
	char __user **buf_ptr = (char **)&PT_REGS_PARM2(regs);
	size_t *count_ptr = (size_t *)&PT_REGS_PARM3(regs);
	loff_t **pos_ptr = (loff_t **)&PT_REGS_CCALL_PARM4(regs);
	**pos_ptr = enforce_handler_pos_val;
 	simple_read_from_buffer(*buf_ptr, *count_ptr, *pos_ptr, "1", 1);
}
#endif

static int execve_handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	struct pt_regs *real_regs = PT_REAL_REGS(regs);
	const char __user **filename_user =
		(const char **)&PT_REGS_PARM1(real_regs);

	return ksu_handle_execve_sucompat(AT_FDCWD, filename_user, NULL, NULL,
					  NULL);
}

static struct kprobe faccessat_kp = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
	.symbol_name = "do_faccessat",
#else
	.symbol_name = "sys_faccessat",
#endif
	.pre_handler = faccessat_handler_pre,
};

static struct kprobe newfstatat_kp = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
	.symbol_name = "vfs_statx",
#else
	.symbol_name = "vfs_fstatat",
#endif
	.pre_handler = newfstatat_handler_pre,
};

#ifdef CONFIG_KSU_SELINUX_NOENFORCING
static struct kprobe sel_read_enforce_kp = {
	.symbol_name = "sel_read_enforce",
	.pre_handler = enforce_handler_pre,
	.post_handler = enforce_handler_post,
};
#endif

static struct kprobe execve_compat_kp = {
	.symbol_name = SYS_EXECVE_COMPAT_SYMBOL,
	.pre_handler = sys_execve_handler_pre,
};

static struct kprobe pts_unix98_lookup_kp = { .symbol_name =
						      "pts_unix98_lookup",
					      .pre_handler =
						      pts_unix98_lookup_pre };

#endif

static struct kprobe *init_kprobe(const char *name,
				  kprobe_pre_handler_t handler)
{
	struct kprobe *kp = kzalloc(sizeof(struct kprobe), GFP_KERNEL);
	if (!kp)
		return NULL;
	kp->symbol_name = name;
	kp->pre_handler = handler;

	int ret = register_kprobe(kp);
	pr_info("sucompat: register_%s kprobe: %d\n", name, ret);
	if (ret) {
		kfree(kp);
		return NULL;
	}

	return kp;
}

static void destroy_kprobe(struct kprobe **kp_ptr)
{
	struct kprobe *kp = *kp_ptr;
	if (!kp)
		return;
	unregister_kprobe(kp);
	synchronize_rcu();
	kfree(kp);
	*kp_ptr = NULL;
}

static struct kprobe *su_kps[4];


// sucompat: permited process can execute 'su' to gain root access.
void ksu_sucompat_init()
{
#ifdef CONFIG_KPROBES
	su_kps[0] = init_kprobe(SYS_EXECVE_SYMBOL, execve_handler_pre);
	su_kps[1] = init_kprobe(SYS_FACCESSAT_SYMBOL, faccessat_handler_pre);
	su_kps[2] = init_kprobe(SYS_NEWFSTATAT_SYMBOL, newfstatat_handler_pre);
	su_kps[3] = init_kprobe("pts_unix98_lookup", pts_unix98_lookup_pre);
#endif
}

void ksu_sucompat_exit()
{
#ifdef CONFIG_KPROBES
	unregister_kprobe(&fstatat64_kp);
	unregister_kprobe(&faccessat_kp);
	for (int i = 0; i < ARRAY_SIZE(su_kps); i++) {
		destroy_kprobe(&su_kps[i]);
	}
#else // We still have non-GKI support!
	void ksu_sucompat_init() {}
	void ksu_sucompat_exit() {}
	#ifdef CONFIG_KSU_SELINUX_NOENFORCING
	ret = register_kprobe(&sel_read_enforce_kp);
	pr_info("sucompat: sel_read_enforce_kp: %d\n", ret);
	#endif
#endif
}

