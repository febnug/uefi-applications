# UEFI Applications

<h3>Prerequisite</h3>
<h4>Bikin struktur:</h4>
<p><pre>
mkdir -p esp/EFI/BOOT
cp BOOTX64.EFI esp/EFI/BOOT/
</pre></p>
<h4>Bikin image FAT:</h4>
<p><pre>
dd if=/dev/zero of=fat.img bs=1M count=64
mkfs.vfat fat.img
mmd -i fat.img ::/EFI
mmd -i fat.img ::/EFI/BOOT
mcopy -i fat.img BOOTX64.EFI ::/EFI/BOOT/
</pre></p>
<h3>Build & Linking</h3>
<h4>Compile:</h4>
<p><pre>
clang -target x86_64-pc-win32-coff -ffreestanding -fshort-wchar -mno-red-zone -fno-stack-protector -fno-builtin -I/usr/include/efi -I/usr/include/efi/x86_64 -c main.c -o main.o</pre></p>
<h4>Linking:</h4>
<p><pre>
lld-link /subsystem:efi_application /entry:efi_main /out:BOOTX64.EFI main.o</pre></p>
<h3>Run</h3>
<h4>Copy first:</h4>
<p><pre>cp BOOTX64.EFI ./esp/EFI/BOOT/</pre></p>
<p><pre>
qemu-system-x86_64 -drive if=pflash,format=raw,readonly=on,file=/usr/share/ovmf/OVMF.fd -drive format=raw,file=fat:rw:esp -device virtio-vga -display gtk -m 512M
</pre></p>
<p><b>mungkin ada error, perlu fix dependensinya</b></p>
