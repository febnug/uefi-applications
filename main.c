#include <efi.h>
#include <efilib.h>

EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
    EFI_INPUT_KEY efi_input_key;
    EFI_STATUS efi_status;

    InitializeLib(image, systab);
    Print(L"    (0w0)/ (^V^)/ (@.@)/    \n");
    Print(L"   Your boys just want to   \n");
    Print(L" dap you up before you boot.\n");
    Print(L"\n\n");
    Print(L"    Press any key to dap.\n");
    WaitForSingleEvent(ST->ConIn->WaitForKey, 0);
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"\n\n");

    efi_status = uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &efi_input_key);

    Print(L"You dapped: ScanCode [%02xh] UnicodeChar [%02xh] CallRtStatus [%02xh]\n",
        efi_input_key.ScanCode, efi_input_key.UnicodeChar, efi_status);

    return EFI_SUCCESS;
}
