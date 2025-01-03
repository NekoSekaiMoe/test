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
ccflags-y += -DKSU_VERSION=$(KSU_VERSION)
else # If there is no .git file, the default version will be passed.
$(warning "KSU_GIT_VERSION not defined! It is better to make KernelSU a git submodule!")
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

ifeq ($(shell grep -q "int path_umount" $(srctree)/fs/namespace.c; echo $$?),0)
ccflags-y += -DKSU_UMOUNT
else
$(info -- Did you know you can backport path_umount to fs/namespace.c from 5.9?)
$(info -- Read: https://kernelsu.org/guide/how-to-integrate-for-non-gki.html#how-to-backport-path-umount)
endif

ccflags-y += -Wno-implicit-function-declaration -Wno-strict-prototypes -Wno-int-conversion -Wno-gcc-compat
ccflags-y += -Wno-declaration-after-statement -Wno-unused-function

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
