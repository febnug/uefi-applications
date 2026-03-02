#include <efi.h>

void *memcpy(void *dest, const void *src, unsigned long n)
{
    unsigned char *d = dest;
    const unsigned char *s = src;

    for (unsigned long i = 0; i < n; i++)
        d[i] = s[i];

    return dest;
}

static CHAR16* memory_type_to_str(UINT32 type)
{
    switch (type)
    {
        case EfiReservedMemoryType: return L"Reserved";
        case EfiLoaderCode: return L"LoaderCode";
        case EfiLoaderData: return L"LoaderData";
        case EfiBootServicesCode: return L"BootCode";
        case EfiBootServicesData: return L"BootData";
        case EfiRuntimeServicesCode: return L"RuntimeCode";
        case EfiRuntimeServicesData: return L"RuntimeData";
        case EfiConventionalMemory: return L"Conventional";
        case EfiUnusableMemory: return L"Unusable";
        case EfiACPIReclaimMemory: return L"ACPI Reclaim";
        case EfiACPIMemoryNVS: return L"ACPI NVS";
        case EfiMemoryMappedIO: return L"MMIO";
        case EfiMemoryMappedIOPortSpace: return L"MMIO Port";
        case EfiPalCode: return L"PalCode";
        default: return L"Unknown";
    }
}

static void print_hex(EFI_SYSTEM_TABLE *SystemTable, UINT64 value)
{
    CHAR16 buffer[17];
    CHAR16 hex[] = L"0123456789ABCDEF";

    for (int i = 0; i < 16; i++)
    {
        buffer[15 - i] = hex[value & 0xF];
        value >>= 4;
    }

    buffer[16] = L'\0';
    SystemTable->ConOut->OutputString(SystemTable->ConOut, buffer);
}

static void print_dec(EFI_SYSTEM_TABLE *SystemTable, UINT64 value)
{
    CHAR16 buffer[21]; // max 20 digit + null
    UINTN i = 0;

    if (value == 0)
    {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"0");
        return;
    }

    while (value > 0)
    {
        buffer[i++] = L'0' + (value % 10);
        value /= 10;
    }

    // reverse
    for (UINTN j = 0; j < i / 2; j++)
    {
        CHAR16 tmp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = tmp;
    }

    buffer[i] = L'\0';

    SystemTable->ConOut->OutputString(SystemTable->ConOut, buffer);
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_BOOT_SERVICES *BootServices = SystemTable->BootServices;
    EFI_STATUS Status;

    // =========================
    // SET TEXT COLOR TO WHITE
    // =========================
    SystemTable->ConOut->SetAttribute(
        SystemTable->ConOut,
        EFI_WHITE | (EFI_BLACK << 4)
    );

    // =========================
    // SET RESOLUTION 1280x1024
    // =========================
/*
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

    Status = BootServices->LocateProtocol(&gopGuid, NULL, (VOID**)&Gop);

    if (!EFI_ERROR(Status)) {
        for (UINT32 i = 0; i < Gop->Mode->MaxMode; i++)
        {
            EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
            UINTN SizeOfInfo;

            Status = Gop->QueryMode(Gop, i, &SizeOfInfo, &info);

            if (!EFI_ERROR(Status)) {
                if (info->HorizontalResolution == 1280 &&
                    info->VerticalResolution == 1024)
                {
                    Gop->SetMode(Gop, i);
                    break;
                }
            }
        }

        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info = Gop->Mode->Info;

        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Resolution: ");
        print_hex(SystemTable, info->HorizontalResolution);
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L" x ");
        print_hex(SystemTable, info->VerticalResolution);
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\r\n\r\n");
    }
*/
    // =========================
    // MEMORY MAP
    // =========================

    UINTN MemoryMapSize = 0;
    EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;

    BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap,
                               &MapKey, &DescriptorSize,
                               &DescriptorVersion);

    MemoryMapSize += DescriptorSize * 8;

    BootServices->AllocatePool(EfiLoaderData,
                               MemoryMapSize,
                               (VOID**)&MemoryMap);

    BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap,
                               &MapKey, &DescriptorSize,
                               &DescriptorVersion);

    UINTN EntryCount = MemoryMapSize / DescriptorSize;
    UINT64 totalRam = 0;
UINT64 totalPhysicalRam = 0;
UINT64 totalUsableRam = 0;

for (UINTN i = 0; i < EntryCount; i++)
{
    EFI_MEMORY_DESCRIPTOR *desc =
        (EFI_MEMORY_DESCRIPTOR*)((UINT8*)MemoryMap + (i * DescriptorSize));

    UINT64 size = desc->NumberOfPages * 4096;

    // ===== PRINT DETAIL =====
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Type: ");
    SystemTable->ConOut->OutputString(SystemTable->ConOut, memory_type_to_str(desc->Type));

    SystemTable->ConOut->OutputString(SystemTable->ConOut, L" | Start: 0x");
    print_hex(SystemTable, desc->PhysicalStart);

    SystemTable->ConOut->OutputString(SystemTable->ConOut, L" | Size: 0x");
    print_hex(SystemTable, size);

    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\r\n");

    // ===== HITUNG PHYSICAL RAM =====
    if (desc->Type == EfiConventionalMemory ||
        desc->Type == EfiBootServicesCode ||
        desc->Type == EfiBootServicesData ||
        desc->Type == EfiLoaderCode ||
        desc->Type == EfiLoaderData ||
        desc->Type == EfiRuntimeServicesCode ||
        desc->Type == EfiRuntimeServicesData)
    {
        totalPhysicalRam += size;
    }

    // ===== HITUNG USABLE RAM =====
    if (desc->Type == EfiConventionalMemory)
    {
        totalUsableRam += size;
    }
}

    // ============================
    // HITUNG USABLE RAM (Free)
    // ============================

//    if (desc->Type == EfiConventionalMemory)
//    {
//        totalUsableRam += size;
//    }
//}


SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Physical RAM: ");

print_dec(SystemTable, totalPhysicalRam / (1024 * 1024));
SystemTable->ConOut->OutputString(SystemTable->ConOut, L" MB (0x");
print_hex(SystemTable, totalPhysicalRam);
SystemTable->ConOut->OutputString(SystemTable->ConOut, L")\r\n");

SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Usable RAM:   ");

print_dec(SystemTable, totalUsableRam / (1024 * 1024));
SystemTable->ConOut->OutputString(SystemTable->ConOut, L" MB (0x");
print_hex(SystemTable, totalUsableRam);
SystemTable->ConOut->OutputString(SystemTable->ConOut, L")\r\n\n");
SystemTable->ConOut->OutputString(SystemTable->ConOut, L"fnlqxz's on UEFI!\r\n");


    while (1);
    return EFI_SUCCESS;
}
