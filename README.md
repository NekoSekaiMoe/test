# KernelSU

A Kernel based root solution for Android GKI.

## Before Reading

Now KernelSU supports old kernel under 5.10, but **THERE WILL NEVER** be a CI for old kernels, because they are not generic.
ANY ISSUES ABOUT HOW TO COMPILE A OLD KERNEL WILL BE CLOSED.

KernelSU is in a early development stage and you should not put it into production enviroment. If you face any issue, feel free to tell us about it!

## Usage

1. Flash a custom kernel with KernelSU, you can build it yourself or [download it from CI](https://github.com/tiann/KernelSU/actions/workflows/build-kernel.yml).
2. Install Manager App and enjoy :)

## Build

### Build GKI Kernel

1. Download the GKI source first, you can refer the [GKI build instruction](https://source.android.com/docs/setup/build/building-kernels)
2. cd `<GKI kernel source dir>`
3. `curl -LSs "https://raw.githubusercontent.com/tiann/KernelSU/main/kernel/setup.sh" | bash -`
4. Build the kernel.

### Build the Manager App

Android Studio / Gradle

### Discussion

[@KernelSU](https://t.me/KernelSU)

## License

[GPL-3](http://www.gnu.org/copyleft/gpl.html)

## Credits

- [kernel-assisted-superuser](https://git.zx2c4.com/kernel-assisted-superuser/about/): the KernelSU idea.
- [genuine](https://github.com/brevent/genuine/): apk v2 signature validation.
- [Diamorphine](https://github.com/m0nad/Diamorphine): some rootkit skills.
- [Magisk](https://github.com/topjohnwu/Magisk): the sepolicy implementation.
