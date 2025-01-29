kernelsu-objs := ksu.o
kernelsu-objs += allowlist.o
kernelsu-objs += apk_sign.o
kernelsu-objs += sucompat.o
kernelsu-objs += throne_tracker.o
kernelsu-objs += core_hook.o
kernelsu-objs += ksud.o
kernelsu-objs += embed_ksud.o
kernelsu-objs += kernel_compat.o

kernelsu-objs += selinux/selinux.o
kernelsu-objs += selinux/sepolicy.o
kernelsu-objs += selinux/rules.o
kernelsu-objs += selinux/kernel_compat.o

kernelsu-objs += libsepol/src/node_record.o
kernelsu-objs += libsepol/src/nodes.o
kernelsu-objs += libsepol/src/optimize.o
kernelsu-objs += libsepol/src/polcaps.o
kernelsu-objs += libsepol/src/policydb.o
kernelsu-objs += libsepol/src/policydb_convert.o
kernelsu-objs += libsepol/src/policydb_public.o
kernelsu-objs += libsepol/src/policydb_validate.o
kernelsu-objs += libsepol/src/port_record.o
kernelsu-objs += libsepol/src/ports.o
kernelsu-objs += libsepol/src/services.o
kernelsu-objs += libsepol/src/sidtab.o
kernelsu-objs += libsepol/src/symtab.o
kernelsu-objs += libsepol/src/user_record.o
kernelsu-objs += libsepol/src/users.o
kernelsu-objs += libsepol/src/util.o
kernelsu-objs += libsepol/src/write.o
kernelsu-objs += libsepol/src/inet_ntop.o
kernelsu-objs += libsepol/src/link.o
kernelsu-objs += libsepol/src/mls.o
kernelsu-objs += libsepol/src/module.o
kernelsu-objs += libsepol/src/assertion.o
kernelsu-objs += libsepol/src/avrule_block.o
kernelsu-objs += libsepol/src/avtab.o
kernelsu-objs += libsepol/src/boolean_record.o
kernelsu-objs += libsepol/src/booleans.o
kernelsu-objs += libsepol/src/conditional.o
kernelsu-objs += libsepol/src/constraint.o
kernelsu-objs += libsepol/src/context.o
kernelsu-objs += libsepol/src/context_record.o
kernelsu-objs += libsepol/src/debug.o
kernelsu-objs += libsepol/src/ebitmap.o
kernelsu-objs += libsepol/src/expand.o
kernelsu-objs += libsepol/src/handle.o
kernelsu-objs += libsepol/src/hashtab.o
kernelsu-objs += libsepol/src/hierarchy.o

ccflags-y += -I$(srctree)/security/selinux -I$(srctree)/security/selinux/include
ccflags-y += -I$(objtree)/security/selinux -include $(srctree)/include/uapi/asm-generic/errno.h
ccflags-y += -I./libsepol/include

obj-$(CONFIG_KSU) += kernelsu.o

# .git is a text file while the module is imported by 'git submodule add'.
ifeq ($(shell test -e $(srctree)/$(src)/../.git; echo $$?),0)
$(shell cd $(srctree)/$(src); /usr/bin/env PATH="$$PATH":/usr/bin:/usr/local/bin [ -f ../.git/shallow ] && git fetch --unshallow)
KSU_GIT_VERSION := $(shell cd $(srctree)/$(src); /usr/bin/env PATH="$$PATH":/usr/bin:/usr/local/bin git rev-list --count HEAD)
# ksu_version: major * 10000 + git version + 200 for historical reasons
$(eval KSU_VERSION=$(shell expr 10000 + $(KSU_GIT_VERSION) + 200))
$(info -- KernelSU version: $(KSU_VERSION))
$(info -- Kernel version: $(KERNELRELEASE))
ccflags-y += -DKSU_VERSION=$(KSU_VERSION) -DKSU_UMOUNT -DKSU_COMPAT_GET_CRED_RCU
else # If there is no .git file, the default version will be passed.
$(info "KSU_GIT_VERSION not defined! It is better to make KernelSU a git submodule!")
ccflags-y += -DKSU_VERSION=16
endif

ifeq ($(shell grep -q " current_sid(void)" $(srctree)/security/selinux/include/objsec.h; echo $$?),0)
ccflags-y += -DKSU_COMPAT_HAS_CURRENT_SID
endif

ifeq ($(shell grep -q "struct selinux_state " $(srctree)/security/selinux/include/security.h; echo $$?),0)
ccflags-y += -DKSU_COMPAT_HAS_SELINUX_STATE
endif

ifdef KSU_MANAGER_PACKAGE
ccflags-y += -DKSU_MANAGER_PACKAGE=\"$(KSU_MANAGER_PACKAGE)\"
$(info -- KernelSU Manager package name: $(KSU_MANAGER_PACKAGE))
endif

ccflags-y += -Wno-implicit-function-declaration -Wno-strict-prototypes -Wno-int-conversion -Wno-gcc-compat
ccflags-y += -Wno-declaration-after-statement -Wno-unused-function -Wno-error

# Determine the appropriate atomic function and apply patch accordingly
ifeq ($(shell grep -q "atomic_inc_not_zero" $(srctree)/kernel/cred.c; echo $$?),0)
ATOMIC_INC_FUNC = atomic_inc_not_zero
else ifeq ($(shell grep -q "atomic_long_inc_not_zero" $(srctree)/kernel/cred.c; echo $$?),0)
ATOMIC_INC_FUNC = atomic_long_inc_not_zero
else
$(info -- KernelSU: Neither atomic_inc_not_zero nor atomic_long_inc_not_zero found in kernel/cred.c)
endif

ifeq ($(shell grep -q "strncpy_from_user_nofault" $(srctree)/include/linux/uaccess.h; echo $$?),0)
ccflags-y += -DKSU_STRNCPY_FROM_USER_NOFAULT
endif

ifeq ($(shell grep -q "ssize_t kernel_read" $(srctree)/fs/read_write.c; echo $$?),0)
ccflags-y += -DKSU_KERNEL_READ
endif

ifeq ($(shell grep "ssize_t kernel_write" $(srctree)/fs/read_write.c | grep -q "const void" ; echo $$?),0)
ccflags-y += -DKSU_KERNEL_WRITE
endif

# Inform which function is being patched
$(info -- KernelSU: Using $(ATOMIC_INC_FUNC) in get_cred_rcu patch.)
# Add the get_cred_rcu function to cred.h if not already present
ifneq ($(shell grep -Eq "^static inline const struct cred \*get_cred_rcu" $(srctree)/include/linux/cred.h; echo $$?),0)
$(info -- KernelSU: adding function 'static inline const struct cred *get_cred_rcu(const struct cred *cred);' to $(srctree)/include/linux/cred.h)
GET_CRED_RCPU = static inline const struct cred *get_cred_rcu(const struct cred *cred)\n\
{\n\t\
        struct cred *nonconst_cred = (struct cred *) cred;\n\t\
        if (!cred)\n\t\t\
                return NULL;\n\t\
        if (!$(ATOMIC_INC_FUNC)(&nonconst_cred->usage))\n\t\t\
                return NULL;\n\t\
        validate_creds(cred);\n\t\
        return cred;\n\
}\n
$(shell grep -qF "$(GET_CRED_RCPU)" $(srctree)/include/linux/cred.h || sed -i '/^static inline void put_cred/i $(GET_CRED_RCPU)' $(srctree)/include/linux/cred.h)

$(info -- KernelSU: modifying 'get_task_cred' function in $(srctree)/kernel/cred.c)
$(shell sed -i "s/!$(ATOMIC_INC_FUNC)(&((struct cred \*)cred)->usage)/!get_cred_rcu(cred)/g" $(srctree)/kernel/cred.c)
endif

ifneq ($(shell grep -Eq "^static int can_umount" $(srctree)/fs/namespace.c; echo $$?),0)
$(info -- KernelSU: adding function 'static int can_umount(const struct path *path, int flags);' to $(srctree)/fs/namespace.c)
CAN_UMOUNT = static int can_umount(const struct path *path, int flags)\n\
{\n\t\
        struct mount *mnt = real_mount(path->mnt);\n\t\
        if (flags & ~(MNT_FORCE | MNT_DETACH | MNT_EXPIRE | UMOUNT_NOFOLLOW))\n\t\t\
                return -EINVAL;\n\t\
        if (!may_mount())\n\t\t\
                return -EPERM;\n\t\
        if (path->dentry != path->mnt->mnt_root)\n\t\t\
                return -EINVAL;\n\t\
        if (!check_mnt(mnt))\n\t\t\
                return -EINVAL;\n\t\
        if (mnt->mnt.mnt_flags & MNT_LOCKED)\n\t\t\
                return -EINVAL;\n\t\
        if (flags & MNT_FORCE && !capable(CAP_SYS_ADMIN))\n\t\t\
                return -EPERM;\n\t\
        return 0;\n\
}\n
$(shell sed -i '/^static bool is_mnt_ns_file/i $(CAN_UMOUNT)' $(srctree)/fs/namespace.c;)
endif

ifneq ($(shell grep -Eq "^int path_umount" $(srctree)/fs/namespace.c; echo $$?),0)
$(info -- KernelSU: adding function 'int path_umount(struct path *path, int flags);' to $(srctree)/fs/namespace.c)
PATH_UMOUNT = int path_umount(struct path *path, int flags)\n\
{\n\t\
        struct mount *mnt = real_mount(path->mnt);\n\t\
        int ret;\n\t\
        ret = can_umount(path, flags);\n\t\
        if (!ret)\n\t\t\
                ret = do_umount(mnt, flags);\n\t\
        dput(path->dentry);\n\t\
        mntput_no_expire(mnt);\n\t\
        return ret;\n\
}\n
$(shell sed -i '/^static bool is_mnt_ns_file/i $(PATH_UMOUNT)' $(srctree)/fs/namespace.c;)
endif

ifneq ($(shell grep -Eq "^int path_umount" $(srctree)/fs/internal.h; echo $$?),0)
$(shell sed -i '/^extern void __init mnt_init/a int path_umount(struct path *path, int flags);' $(srctree)/fs/internal.h;)
$(info -- KernelSU: adding 'int path_umount(struct path *path, int flags);' to $(srctree)/fs/internal.h)
endif

ifeq ($(shell test -e $(srctree)/fs/susfs.c; echo $$?),0)
ifdef KSU_SUSFS
ccflags-y += -DKSU_SUSFS
endif
ifdef KSU_SUSFS_SUS_PATH
ccflags-y += -DKSU_SUSFS_SUS_PATH
endif
ifdef KSU_SUSFS_SUS_MOUNT
ccflags-y += -DKSU_SUSFS_SUS_MOUNT
endif
ifdef KSU_SUSFS_SUS_MOUNT_MNT_ID_REORDER
ccflags-y += -DKSU_SUSFS_SUS_MOUNT_MNT_ID_REORDER
endif
ifdef KSU_SUSFS_SUS_KSTAT
ccflags-y += -DKSU_SUSFS_SUS_KSTAT
endif
ifdef KSU_SUSFS_SUS_MAPS
ccflags-y += -DKSU_SUSFS_SUS_MAPS
endif
ifdef KSU_SUSFS_SUS_PROC_FD_LINK
ccflags-y += -DKSU_SUSFS_SUS_PROC_FD_LINK
endif
ifdef KSU_SUSFS_SUS_MEMFD
ccflags-y += -DKSU_SUSFS_SUS_MEMFD
endif
ifdef KSU_SUSFS_TRY_UMOUNT
ccflags-y += -DKSU_SUSFS_TRY_UMOUNT
endif
ifdef KSU_SUSFS_SPOOF_UNAME
ccflags-y += -DKSU_SUSFS_SPOOF_UNAME
endif
ifdef KSU_SUSFS_ENABLE_LOG
ccflags-y += -DKSU_SUSFS_ENABLE_LOG
endif
else
$(info -- You have not integrate susfs in your kernel.)
$(info -- Read: https://gitlab.com/simonpunk/susfs4ksu)
endif

# Keep a new line here!! Because someone may append config
