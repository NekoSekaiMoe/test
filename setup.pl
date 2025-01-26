#!/usr/bin/env perl

use strict;
use warnings;
use File::Path qw(remove_tree);
use Cwd;

my $GKI_ROOT = getcwd();
my ($DRIVER_DIR, $DRIVER_MAKEFILE, $DRIVER_KCONFIG);

# Initialize variables
sub initialize_variables {
    if (-d "$GKI_ROOT/common/drivers") {
        $DRIVER_DIR = "$GKI_ROOT/common/drivers";
    } elsif (-d "$GKI_ROOT/drivers") {
        $DRIVER_DIR = "$GKI_ROOT/drivers";
    } else {
        die '[ERROR] "drivers/" directory not found.';
    }

    $DRIVER_MAKEFILE = "$DRIVER_DIR/Makefile";
    $DRIVER_KCONFIG = "$DRIVER_DIR/Kconfig";
}

# Cleanup function
sub perform_cleanup {
    print "[+] Cleaning up...\n";
    if (-f ".gitsubmodules" && -f ".git") {
        system("git submodule deinit KernelSU");
        system("git rm -rf .git/modules/KernelSU");
    } else {
        if (-l "$DRIVER_DIR/kernelsu") {
            unlink("$DRIVER_DIR/kernelsu");
            print "[-] Symlink removed.\n";
        }
        
        # Process Makefile
        if (open(my $fh, '<', $DRIVER_MAKEFILE)) {
            my $content = do { local $/; <$fh> };
            close($fh);
            if ($content =~ /kernelsu/) {
                $content =~ s/.*kernelsu.*\n//g;
                open(my $out, '>', $DRIVER_MAKEFILE) or die "Cannot write to $DRIVER_MAKEFILE: $!";
                print $out $content;
                close($out);
                print "[-] Makefile reverted.\n";
            }
        }

        # Process Kconfig
        if (open(my $fh, '<', $DRIVER_KCONFIG)) {
            my $content = do { local $/; <$fh> };
            close($fh);
            if ($content =~ /drivers\/kernelsu\/Kconfig/) {
                $content =~ s/.*drivers\/kernelsu\/Kconfig.*\n//g;
                open(my $out, '>', $DRIVER_KCONFIG) or die "Cannot write to $DRIVER_KCONFIG: $!";
                print $out $content;
                close($out);
                print "[-] Kconfig reverted.\n";
            }
        }

        if (-d "$GKI_ROOT/KernelSU") {
            remove_tree("$GKI_ROOT/KernelSU");
            unlink("$GKI_ROOT/.gitsubmodules");
            print "[-] KernelSU directory deleted.\n";
        }
    }
}

# Setup KernelSU
sub setup_kernelsu {
    my $branch = shift;
    
    print "[+] Setting up KernelSU...\n";
    print "[!] Warning: This is an modification about non-gki kernel for KernelSU. Use of unofficial modifications may result in damage to the mobile phone software or other losses.\n";

    if (!-d "$GKI_ROOT/KernelSU") {
        my $clone_cmd = "git clone https://github.com/NekoSekaiMoe/test";
        $clone_cmd .= " -b $branch" if defined $branch;
        $clone_cmd .= " KernelSU";
        system($clone_cmd) == 0 or die "Failed to clone repository: $?";
        print "[+] Repository cloned.\n";
    }

    chdir("$GKI_ROOT/KernelSU") or die "Cannot change to KernelSU directory: $!";
    system("git checkout . && git clean -dxf") == 0 or die "Git operations failed: $?";
    chdir($GKI_ROOT) or die "Cannot change back to root directory: $!";
    chdir("$GKI_ROOT/drivers") or die "Cannot change to drivers directory: $!";

    symlink("$GKI_ROOT/KernelSU/kernel", "$GKI_ROOT/drivers/kernelsu") or die "Failed to create symlink: $!";
    print "[+] Symlink created.\n";

    # Modify Makefile
    open(my $mf, '>>', $DRIVER_MAKEFILE) or die "Cannot open Makefile: $!";
    print $mf "\nobj-\$(CONFIG_KSU) += kernelsu/\n";
    close($mf);
    print "[+] Modified Makefile.\n";

    # Modify Kconfig
    if (open(my $kf, '<', $DRIVER_KCONFIG)) {
        my $content = do { local $/; <$kf> };
        close($kf);
        
        $content =~ s/endmenu/source "drivers\/kernelsu\/Kconfig"\nendmenu/;
        
        open(my $kf_out, '>', $DRIVER_KCONFIG) or die "Cannot write to Kconfig: $!";
        print $kf_out $content;
        close($kf_out);
        print "[+] Modified Kconfig.\n";
    }
    
    print "[+] Done.\n";
}

# Main execution
initialize_variables();

if (@ARGV == 0) {
    setup_kernelsu();
} elsif ($ARGV[0] eq "--cleanup") {
    perform_cleanup();
} else {
    setup_kernelsu($ARGV[0]);
}
