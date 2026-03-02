# UEFI Applications
<h3>Build & Linking</h3>
<h4>Compile:</h4>
<p><pre>
clang -target x86_64-pc-win32-coff -ffreestanding -fshort-wchar -mno-red-zone -fno-stack-protector -fno-builtin -I/usr/include/efi -I/usr/include/efi/x86_64 -c main.c -o main.o</pre></p>
<h4>Linking:</h4>
<p><pre>
lld-link /subsystem:efi_application /entry:efi_main /out:BOOTX64.EFI main.o</pre></p>
