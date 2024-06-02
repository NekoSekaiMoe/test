#!/usr/bin/env python3
import os
import subprocess
import sys

class KernelSUSetup:
    def __init__(self):
        self.GKI_ROOT = os.getcwd()
        self.DRIVER_DIR = ''
        self.DRIVER_MAKEFILE = ''
        self.DRIVER_KCONFIG = ''
        self.initialize_variables()

    def initialize_variables(self):
        if os.path.isdir(os.path.join(self.GKI_ROOT, "common", "drivers")):
            self.DRIVER_DIR = os.path.join(self.GKI_ROOT, "common", "drivers")
        elif os.path.isdir(os.path.join(self.GKI_ROOT, "drivers")):
            self.DRIVER_DIR = os.path.join(self.GKI_ROOT, "drivers")
        else:
            print('[ERROR] "drivers/" directory not found.')
            sys.exit(127)

        self.DRIVER_MAKEFILE = os.path.join(self.DRIVER_DIR, "Makefile")
        self.DRIVER_KCONFIG = os.path.join(self.DRIVER_DIR, "Kconfig")

    def perform_cleanup(self):
        print("[+] Cleaning up...")
        if os.path.isfile('.gitsubmodules') and os.path.isfile('.git'):
            subprocess.run(["git", "submodule", "deinit", "KernelSU"], check=True)
            subprocess.run(["git", "rm", "-rf", ".git/modules/KernelSU"], check=True)
        else:
            kernelsu_path = os.path.join(self.DRIVER_DIR, "kernelsu")
            if os.path.islink(kernelsu_path):
                os.remove(kernelsu_path)
                print("[-] Symlink removed.")

            self.remove_line_if_exists(self.DRIVER_MAKEFILE, 'kernelsu')
            self.remove_line_if_exists(self.DRIVER_KCONFIG, 'drivers/kernelsu/Kconfig')

            kernelsu_dir = os.path.join(self.GKI_ROOT, "KernelSU")
            if os.path.isdir(kernelsu_dir):
                subprocess.run(["rm", "-rf", kernelsu_dir, ".gitsubmodules"], check=True)
                print("[-] KernelSU directory deleted.")

    def setup_kernelsu(self, branch='main'):
        print("[+] Setting up KernelSU...")
        print("[!] Warning: This is an modification about non-gki kernel for KernelSU. Use of unofficial modifications may result in damage to the mobile phone software or other losses.")
        kernelsu_path = os.path.join(self.GKI_ROOT, "KernelSU")
        if not os.path.isdir(kernelsu_path):
            subprocess.run(["git", "clone", "https://github.com/NekoSekaiMoe/test", "-b", branch, "KernelSU"], check=True)
            print("[+] Repository cloned.")
        os.chdir("KernelSU")
        subprocess.run(["git", "checkout", "."], check=True)
        subprocess.run(["git", "clean", "-dxf"], check=True)
        os.chdir("..")

        symlink_dest = os.path.relpath(os.path.join(kernelsu_path, "kernel"), self.DRIVER_DIR)
        symlink_src = os.path.join(self.DRIVER_DIR, "kernelsu")
        os.symlink(symlink_dest, symlink_src)
        print("[+] Symlink created.")

        self.append_line_if_not_exists(self.DRIVER_MAKEFILE, "\nobj-$(CONFIG_KSU) += kernelsu/\n")
        self.insert_line_if_not_exists(self.DRIVER_KCONFIG, "source \"drivers/kernelsu/Kconfig\"", "/endmenu/i\\")

        print('[+] Done.')

    def remove_line_if_exists(self, file, pattern):
        with open(file, "r") as f:
            lines = f.readlines()
        with open(file, "w") as f:
            for line in lines:
                if pattern not in line:
                    f.write(line)
                else:
                    print(f"[-] {pattern} removed.")

    def append_line_if_not_exists(self, file, content):
        with open(file, "r+") as f:
            lines = f.readlines()
            if content not in lines:
                f.write(content)
                print("[+] Modified Makefile.")

    def insert_line_if_not_exists(self, file, content, pattern):
        content_inserted = False
        with open(file, "r") as f:
            lines = f.readlines()

        with open(file, "w") as f:
            for line in lines:
                f.write(line)
                if pattern in line and not content_inserted:
                    f.write(content + "n")
                    content_inserted = True
                    break

        if not content_inserted:
            with open(file, "a") as f:
                f.write(content + "n")

def main():
    if len(sys.argv) > 1 and sys.argv[1] == "--cleanup":
        setup = KernelSUSetup()
        setup.perform_cleanup()
    else:
        branch = sys.argv[1] if len(sys.argv) > 1 else "main"
        setup = KernelSUSetup()
        setup.setup_kernelsu(branch)

if __name__ == "__main__":
    main()
