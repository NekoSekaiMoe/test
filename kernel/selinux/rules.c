#include <linux/version.h>
#include "sepolicy.h"
#include "selinux.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#define SELINUX_POLICY_INSTEAD_SELINUX_SS
#endif

#ifndef SELINUX_POLICY_INSTEAD_SELINUX_SS
#include <ss/services.h>
#endif

#define KERNEL_SU_DOMAIN "su"
#define ALL NULL

void apply_kernelsu_rules()
{
	struct policydb *db;

	if (!getenforce()) {
		pr_info("SELinux permissive or disabled, don't apply rules.");
		return;
	}

	rcu_read_lock();
#ifdef SELINUX_POLICY_INSTEAD_SELINUX_SS
	struct selinux_policy *policy = rcu_dereference(selinux_state.policy);
	db = &policy->policydb;
#else
	struct selinux_ss *ss = rcu_dereference(selinux_state.ss);
	db = &ss->policydb;
#endif

	ksu_permissive(db, KERNEL_SU_DOMAIN);
	ksu_typeattribute(db, KERNEL_SU_DOMAIN, "mlstrustedsubject");
	ksu_typeattribute(db, KERNEL_SU_DOMAIN, "netdomain");
	ksu_typeattribute(db, KERNEL_SU_DOMAIN, "bluetoothdomain");

	// allow all!
	ksu_allow(db, KERNEL_SU_DOMAIN, ALL, ALL, ALL);

	// allow us do any ioctl
	if (db->policyvers >= POLICYDB_VERSION_XPERMS_IOCTL) {
		ksu_allowxperm(db, KERNEL_SU_DOMAIN, ALL, "blk_file", ALL);
		ksu_allowxperm(db, KERNEL_SU_DOMAIN, ALL, "fifo_file", ALL);
		ksu_allowxperm(db, KERNEL_SU_DOMAIN, ALL, "chr_file", ALL);
	}

	// we need to save allowlist in /data/adb
	ksu_allow(db, "kernel", "adb_data_file", "dir", ALL);
	ksu_allow(db, "kernel", "adb_data_file", "file", ALL);
	// we may need to do mount on shell
	ksu_allow(db, "kernel", "shell_data_file", "file", ALL);

	// our ksud triggered by init
	ksu_allow(db, "init", "adb_data_file", "file", "execute");
	ksu_allow(db, "init", KERNEL_SU_DOMAIN, ALL, ALL);

	// copied from Magisk rules
	// suRights
	ksu_allow(db, "servicemanager", KERNEL_SU_DOMAIN, "dir", "search");
	ksu_allow(db, "servicemanager", KERNEL_SU_DOMAIN, "dir", "read");
	ksu_allow(db, "servicemanager", KERNEL_SU_DOMAIN, "file", "open");
	ksu_allow(db, "servicemanager", KERNEL_SU_DOMAIN, "file", "read");
	ksu_allow(db, "servicemanager", KERNEL_SU_DOMAIN, "process", "getattr");
	ksu_allow(db, ALL, KERNEL_SU_DOMAIN, "process", "sigchld");

	// allowLog
	ksu_allow(db, "logd", KERNEL_SU_DOMAIN, "dir", "search");
	ksu_allow(db, "logd", KERNEL_SU_DOMAIN, "file", "read");
	ksu_allow(db, "logd", KERNEL_SU_DOMAIN, "file", "open");
	ksu_allow(db, "logd", KERNEL_SU_DOMAIN, "file", "getattr");

	// dumpsys
	ksu_allow(db, ALL, KERNEL_SU_DOMAIN, "fd", "use");
	ksu_allow(db, ALL, KERNEL_SU_DOMAIN, "fifo_file", "write");
	ksu_allow(db, ALL, KERNEL_SU_DOMAIN, "fifo_file", "read");
	ksu_allow(db, ALL, KERNEL_SU_DOMAIN, "fifo_file", "open");
	ksu_allow(db, ALL, KERNEL_SU_DOMAIN, "fifo_file", "getattr");

	// bootctl
	ksu_allow(db, "hwservicemanager", KERNEL_SU_DOMAIN, "dir", "search");
	ksu_allow(db, "hwservicemanager", KERNEL_SU_DOMAIN, "file", "read");
	ksu_allow(db, "hwservicemanager", KERNEL_SU_DOMAIN, "file", "open");
	ksu_allow(db, "hwservicemanager", KERNEL_SU_DOMAIN, "process", "getattr");

	// Allow all binder transactions
	ksu_allow(db, ALL, KERNEL_SU_DOMAIN, "binder", ALL);

	rcu_read_unlock();
}