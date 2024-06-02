#!/bin/sh
set -eu

GKI_ROOT=$(pwd)

initialize_variables() {
    if test -d "$GKI_ROOT/common/drivers"; then
         DRIVER_DIR="$GKI_ROOT/common/drivers"
    elif test -d "$GKI_ROOT/drivers"; then
         DRIVER_DIR="$GKI_ROOT/drivers"
    else
         echo '[ERROR] "drivers/" directory not found.'
         exit 127
    fi

    DRIVER_MAKEFILE="$DRIVER_DIR/Makefile"
    DRIVER_KCONFIG="$DRIVER_DIR/Kconfig"
}

# Reverts modifications made by this script
perform_cleanup() {
    echo "[+] Cleaning up..."
    if [ -f .gitsubmodules ] && [ -f .git ]; then
        git submodule deinit KernelSU
        git rm -rf .git/modules/KernelSU
    else
        [ -L "$DRIVER_DIR/kernelsu" ] && rm "$DRIVER_DIR/kernelsu" && echo "[-] Symlink removed."
        grep -q "kernelsu" "$DRIVER_MAKEFILE" && sed -i '/kernelsu/d' "$DRIVER_MAKEFILE" && echo "[-] Makefile reverted."
        grep -q "drivers/kernelsu/Kconfig" "$DRIVER_KCONFIG" && sed -i '/drivers\/kernelsu\/Kconfig/d' "$DRIVER_KCONFIG" && echo "[-] Kconfig reverted."
        if [ -d "$GKI_ROOT/KernelSU" ]; then
            rm -rf "$GKI_ROOT/KernelSU .gitsubmodules" && echo "[-] KernelSU directory deleted."
        fi
    fi
}

# Sets up or update KernelSU environment
setup_kernelsu() {
    echo "[+] Setting up KernelSU..."
    printf "[!] Warning: This is an modification about non-gki kernel for KernelSU. Use of unofficial modifications may result in damage to the mobile phone software or other losses."
    test -d "$GKI_ROOT/KernelSU" || git clone https://github.com/NekoSekaiMoe/test -b "$1" "KernelSU" && echo "[+] Repository cloned."
    cd KernelSU && git checkout . && git clean -dxf && cd ..
    ln -sf "$(realpath --relative-to="$DRIVER_DIR" "$GKI_ROOT/KernelSU/kernel")" "kernelsu" && echo "[+] Symlink created."

    # Add entries in Makefile and Kconfig if not already existing
    grep -q "kernelsu" "$DRIVER_MAKEFILE" || printf "\nobj-\$(CONFIG_KSU) += kernelsu/\n" >> "$DRIVER_MAKEFILE" && echo "[+] Modified Makefile."
    grep -q "source \"drivers/kernelsu/Kconfig\"" "$DRIVER_KCONFIG" || sed -i "/endmenu/i\source \"drivers/kernelsu/Kconfig\"" "$DRIVER_KCONFIG" && echo "[+] Modified Kconfig."
    echo '[+] Done.'
}

# Process command-line arguments
main() {
    if [ "$#" -eq 0 ]; then
        initialize_variables
        setup_kernelsu
    elif [ "$1" = "--cleanup" ]; then
        initialize_variables
        perform_cleanup
    else
        initialize_variables
        setup_kernelsu "$@"
    fi
}

main
