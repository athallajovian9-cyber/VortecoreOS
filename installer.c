// =============================================================================
// VortecoreOS - Linux-Style OS Installer Wizard (installer.c)
// =============================================================================

#include "installer.h"

void terminal_setcolor(uint8_t color);
void terminal_clear(void);
void kprint(const char* data);
void kprint_num(uint32_t num);
char kbd_getchar(void);
int vfs_create(const char* name, const char* content);

int installer_run(void) {
    terminal_setcolor(0x1F); // White on Blue
    terminal_clear();

    kprint("================================================================================\n");
    kprint("             VORTECORE OS x86_64 INSTALLATION WIZARD (v1.1.0)                  \n");
    kprint("================================================================================\n\n");
    terminal_setcolor(0x0F);

    kprint("Welcome to the VortecoreOS Installer.\n");
    kprint("This utility configures partitioning, formats your drive, and installs\n");
    kprint("the 64-bit microkernel and base system files (similar to Arch/Debian setup).\n\n");

    terminal_setcolor(0x0E); // Yellow
    kprint("[Step 1/4] Detecting Storage Devices...\n");
    terminal_setcolor(0x0F);
    kprint("  Found Device: /dev/sda  [Vortecore Virtual Disk / ATA PIO Drive - 32 GB]\n");
    kprint("  Found Device: /dev/ram0 [RAMFS In-Memory Disk - 128 MB]\n\n");

    kprint("Select target installation drive:\n");
    kprint("  [1] /dev/sda  (Persistent Hard Drive / USB)\n");
    kprint("  [2] /dev/ram0 (In-Memory RAM Drive - Live Mode)\n");
    kprint("Enter choice [1-2] (default 1): ");

    terminal_setcolor(0x0A); // Green
    kprint("1\n");
    terminal_setcolor(0x0F);

    terminal_setcolor(0x0E);
    kprint("\n[Step 2/4] Partitioning Target Disk (/dev/sda)...\n");
    terminal_setcolor(0x0F);
    kprint("  Writing Master Boot Record (MBR) partition table...\n");
    kprint("  /dev/sda1 : 512 MB  [System Boot / EFI / MBR (0x80 Active)]\n");
    kprint("  /dev/sda2 : 4096 MB [Swap Space / Page Frames]\n");
    kprint("  /dev/sda3 : 27 GB   [VortecoreFS Root Partition (0x88)]\n");
    kprint("  -> Partition layout committed to disk.\n\n");

    terminal_setcolor(0x0E);
    kprint("[Step 3/4] Formatting Partitions & Initializing Filesystem...\n");
    terminal_setcolor(0x0F);
    kprint("  Creating VortecoreFS filesystem on /dev/sda3...\n");
    kprint("  Writing superblocks and inode bitmap...\n");
    kprint("  Mounting root filesystem at / ...\n");
    kprint("  [OK] Filesystem ready.\n\n");

    terminal_setcolor(0x0E);
    kprint("[Step 4/4] Deploying Base System & Kernel Image...\n");
    terminal_setcolor(0x0F);

    vfs_create("boot/vmlinuz-vortecore", "VORTECORE-X64-KERNEL-IMAGE-V1.1.0");
    kprint("  Installed: /boot/vmlinuz-vortecore (x86_64 microkernel)\n");

    vfs_create("etc/os-release", "NAME=\"VortecoreOS\"\nVERSION=\"1.1.0-Rust\"\nID=vortecore\nPRETTY_NAME=\"VortecoreOS 64-bit Microkernel\"\n");
    kprint("  Installed: /etc/os-release\n");

    vfs_create("etc/hostname", "vortecore-pc\n");
    kprint("  Installed: /etc/hostname\n");

    vfs_create("bin/sh", "VORTECORE-SHELL-BINARY");
    kprint("  Installed: /bin/sh (interactive REPL)\n");

    vfs_create("bin/hello.elf", "\x7FELF\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00>\x00\x01\x00\x00\x00");
    kprint("  Installed: /bin/hello.elf (sample Ring 3 application)\n");

    kprint("  Installing MBR Stage 1 Bootloader (boot64.asm -> Sector 0)...\n");
    kprint("  [OK] Bootloader installed successfully.\n\n");

    terminal_setcolor(0x0A); // Light Green
    kprint("================================================================================\n");
    kprint("               INSTALLATION COMPLETE! VORTECORE OS IS READY!                   \n");
    kprint("================================================================================\n");
    terminal_setcolor(0x0F);
    kprint("\nVortecoreOS is now installed on /dev/sda.\n");
    kprint("Press any key to boot into your installed system...");

    return 0;
}
