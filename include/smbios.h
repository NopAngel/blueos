#ifndef _BLUEOS_SMBIOS_H
#define _BLUEOS_SMBIOS_H

void get_bios_info(char *bios_version, int bios_size, char *full_name,
                   int full_name_size);
void get_machine_full_name(char *out);

#endif /* _BLUEOS_SMBIOS_H */
