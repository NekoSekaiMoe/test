use std::{collections::HashMap, path::Path};

use crate::{
    defs,
    utils::{ensure_clean_dir, mount_image},
};
use anyhow::{bail, Result};
use sys_mount::{FilesystemType, Mount, MountFlags};

fn mount_partition(partition: &str, lowerdir: &mut Vec<String>) {
    if lowerdir.is_empty() {
        println!("partition: {partition} lowerdir is empty");
        return;
    }

    // if /partition is a symlink and linked to /system/partition, then we don't need to overlay it separately
    if Path::new(&format!("/{}", partition)).read_link().is_ok() {
        println!("partition: {} is a symlink", partition);
        return;
    }
    // add /partition as the lowerest dir
    let lowest_dir = format!("/{partition}");
    lowerdir.push(lowest_dir.clone());

    let lowerdir = lowerdir.join(":");
    println!("partition: {partition} lowerdir: {lowerdir}");

    if let Err(err) = Mount::builder()
        .fstype(FilesystemType::from("overlay"))
        .flags(MountFlags::RDONLY)
        .data(&format!("lowerdir={lowerdir}"))
        .mount("overlay", lowest_dir)
    {
        println!("mount partition: {partition} overlay failed: {err}");
    }
}

pub fn do_systemless_mount(module_dir: &str) -> Result<()> {
    // construct overlay mount params
    let dir = std::fs::read_dir(module_dir);
    let Ok(dir) = dir else {
            bail!("open {} failed", defs::MODULE_DIR);
        };

    let mut system_lowerdir: Vec<String> = Vec::new();

    let partition = vec!["vendor", "product", "system_ext", "odm", "oem"];
    let mut partition_lowerdir: HashMap<String, Vec<String>> = HashMap::new();
    for ele in &partition {
        partition_lowerdir.insert(ele.to_string(), Vec::new());
    }

    for entry in dir.flatten() {
        let module = entry.path();
        if !module.is_dir() {
            continue;
        }
        let disabled = module.join(defs::DISABLE_FILE_NAME).exists();
        if disabled {
            println!("module: {} is disabled, ignore!", module.display());
            continue;
        }

        let module_system = Path::new(&module).join("system");
        if !module_system.as_path().exists() {
            println!("module: {} has no system overlay.", module.display());
            continue;
        }
        system_lowerdir.push(format!("{}", module_system.display()));

        for part in &partition {
            // if /partition is a mountpoint, we would move it to $MODPATH/$partition when install
            // otherwise it must be a symlink and we don't need to overlay!
            let part_path = Path::new(&module).join(part);
            if !part_path.exists() {
                continue;
            }
            if let Some(v) = partition_lowerdir.get_mut(*part) {
                v.push(format!("{}", part_path.display()));
            }
        }
    }

    // mount /system first
    mount_partition("system", &mut system_lowerdir);

    // mount other partitions
    for (k, mut v) in partition_lowerdir {
        mount_partition(&k, &mut v);
    }

    Ok(())
}

pub fn on_post_data_fs() -> Result<()> {
    crate::ksu::report_post_fs_data();
    let module_update_img = defs::MODULE_UPDATE_IMG;
    let module_img = defs::MODULE_IMG;
    let module_dir = defs::MODULE_DIR;
    let module_update_flag = Path::new(defs::WORKING_DIR).join(defs::UPDATE_FILE_NAME);

    // modules.img is the default image
    let mut target_update_img = &module_img;

    // we should clean the module mount point if it exists
    ensure_clean_dir(module_dir)?;

    if Path::new(module_update_img).exists() {
        if module_update_flag.exists() {
            // if modules_update.img exists, and the the flag indicate this is an update
            // this make sure that if the update failed, we will fallback to the old image
            // if we boot succeed, we will rename the modules_update.img to modules.img #on_boot_complete
            target_update_img = &module_update_img;
            // And we should delete the flag immediately
            std::fs::remove_file(module_update_flag)?;
        } else {
            // if modules_update.img exists, but the flag not exist, we should delete it
            std::fs::remove_file(module_update_img)?;
        }
    }

    if !Path::new(target_update_img).exists() {
        // no image exist, do nothing for module!
        return Ok(());
    }

    println!("mount {} to {}", target_update_img, module_dir);
    mount_image(target_update_img, module_dir)?;

    // load sepolicy.rule
    if (crate::module::load_sepolicy_rule().is_err()) {
        println!("load sepolicy.rule failed");
    }

    // mount systemless overlay
    if let Err(e) = do_systemless_mount(module_dir) {
        println!("do systemless mount failed: {}", e);
    }

    // module mounted, exec modules post-fs-data scripts
    if !crate::utils::is_safe_mode() {
        // todo: Add timeout
        let _ = crate::module::exec_post_fs_data();
        let _ = crate::module::load_system_prop();
    } else {
        println!("safe mode, skip module post-fs-data scripts");
    }

    Ok(())
}

pub fn on_services() -> Result<()> {
    // exec modules service.sh scripts
    if !crate::utils::is_safe_mode() {
        let _ = crate::module::exec_services();
    } else {
        println!("safe mode, skip module service scripts");
    }

    Ok(())
}

pub fn on_boot_completed() -> Result<()> {
    crate::ksu::report_boot_complete();
    let module_update_img = Path::new(defs::MODULE_UPDATE_IMG);
    let module_img = Path::new(defs::MODULE_IMG);
    if module_update_img.exists() {
        // this is a update and we successfully booted
        std::fs::rename(module_update_img, module_img)?;
    }
    Ok(())
}

pub fn daemon() -> Result<()> {
    Ok(())
}

pub fn install() -> Result<()> {
    let src = "/proc/self/exe";
    let dst = defs::DAEMON_PATH;

    std::fs::copy(src, dst)?;
    Ok(())
}
