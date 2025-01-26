#!/usr/bin/perl

use strict;
use warnings;
use File::Spec;
use File::Path qw(remove_tree);
use File::Basename;

my $GKI_ROOT = getcwd();

# Initialize variables for driver paths
sub initialize_variables {
    my $driver_dir;

    if (-d "$GKI_ROOT/common/drivers") {
        $driver_dir = "$GKI_ROOT/common/drivers";
    } elsif (-d "$GKI_ROOT/drivers") {
        $driver_dir = "$GKI_ROOT/drivers";
    } else {
        die '[ERROR] "drivers/" directory not found.';
    }

    my $driver_makefile = "$driver_dir/Makefile";
    my $driver_kconfig = "$driver_dir/Kconfig";
    return ($driver_dir, $driver_makefile, $driver_kconfig);
}

# Reverts modifications made by this script
sub perform_cleanup {
    print "[+] Cleaning up...\n";

    if (-f '.gitsubmodules' && -f '.git') {
        system('git submodule deinit KernelSU');
        system('git rm -rf .git/modules/KernelSU');
    } else {
        my ($driver_dir, $driver_makefile, $driver_kconfig) = initialize_variables();

        if (-l "$driver_dir/kernelsu") {
            unlink "$driver_dir/kernelsu" and print "[-] Symlink removed.\n";
        }

        if (grep { /kernelsu/ } `cat $driver_makefile`) {
            system("sed -i '/kernelsu/d' $driver_makefile");
            print "[-] Makefile reverted.\n";
        }

        if (grep { /drivers\/kernelsu\/Kconfig/ } `cat $driver_kconfig`) {
            system("sed -i '/drivers\\/kernelsu\\/Kconfig/d' $driver_kconfig");
            print "[-] Kconfig reverted.\n";
        }

        if (-d "$GKI_ROOT/KernelSU") {
            remove_tree("$GKI_ROOT/KernelSU");
            unlink "$GKI_ROOT/.gitsubmodules";
            print "[-] KernelSU directory deleted.\n";
        }
    }
}

# Sets up or update KernelSU environment
sub setup_kernelsu {
    print "[+] Setting up KernelSU...\n";
    print "[!] Warning: This is a modification about non-gki kernel for KernelSU. Use of unofficial modifications may result in damage to the mobile phone software or other losses.\n";

    my ($branch) = @_;

    unless (-d "$GKI_ROOT/KernelSU") {
        my $clone_command = "git clone https://github.com/NekoSekaiMoe/test KernelSU";
        $clone_command .= " -b $branch" if $branch;
        system($clone_command);
        print "[+] Repository cloned.\n";
    }

    chdir "$GKI_ROOT/KernelSU" or die "Failed to change directory: $!";
    system("git checkout .");
    system("git clean -dxf");
    chdir $GKI_ROOT;

    my ($driver_dir, $driver_makefile, $driver_kconfig) = initialize_variables();

    symlink("$GKI_ROOT/KernelSU/kernel", "$driver_dir/kernelsu") or die "Failed to create symlink: $!";
    print "[+] Symlink created.\n";

    unless (grep { /kernelsu/ } `cat $driver_makefile`) {
        open my $makefile, '>>', $driver_makefile or die "Cannot open Makefile: $!";
        print $makefile "\nobj-\$(CONFIG_KSU) += kernelsu/\n";
        close $makefile;
        print "[+] Modified Makefile.\n";
    }

    unless (grep { /source \"drivers\/kernelsu\/Kconfig\"/ } `cat $driver_kconfig`) {
        open my $kconfig, '>>', $driver_kconfig or die "Cannot open Kconfig: $!";
        print $kconfig "\nsource \"drivers/kernelsu/Kconfig\"\n";
        close $kconfig;
        print "[+] Modified Kconfig.\n";
    }

    print '[+] Done.' . "\n";
}

# Get current working directory
sub getcwd {
    return File::Spec->curdir;
}

# Process command-line arguments
if (@ARGV == 0) {
    my ($driver_dir, $driver_makefile, $driver_kconfig) = initialize_variables();
    setup_kernelsu();
} elsif ($ARGV[0] eq '--cleanup') {
    my ($driver_dir, $driver_makefile, $driver_kconfig) = initialize_variables();
    perform_cleanup();
} else {
    my ($driver_dir, $driver_makefile, $driver_kconfig) = initialize_variables();
    setup_kernelsu($ARGV[0]);
}

