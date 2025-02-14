#include <stdio.h>

#include "elf.h"


void assert(bool is_true, char * msg) {
    if (!is_true) {
        ksp("ASSERT FAILED! %s\n", msg);
        while (1) {}
    }
}


Elf64 elf_parse(u8* buf, u64 len) {

    Elf64 result = {0};

    u8* start = buf;
    u8* current = start;

    assert(len > sizeof(Elf64_Ehdr), "Not an ELF File (length check)!");

    Elf64_Ehdr header = {0};
    memcpy(&header, current, sizeof(Elf64_Ehdr));
    current += sizeof(Elf64_Ehdr);
    // u8* header_end = current;

    assert(header.e_ident[EI_MAG0] == '\x7f' &&
            header.e_ident[EI_MAG1] == 'E' &&
            header.e_ident[EI_MAG2] == 'L' &&
            header.e_ident[EI_MAG3] == 'F', "Not an ELF file (header check)!");
    assert(header.e_ident[EI_CLASS] == ELFCLASS64, "only ELFCLASS64 supported!");
    assert(header.e_ident[EI_DATA] == ELFDATA2LSB, "only ELFDATA2LSB supported!");
    assert(header.e_ident[EI_OSABI] == ELFOSABI_SYSV, "only ELFOSABI_SYSV supported!");

    assert(header.e_type == ET_EXEC || header.e_type == ET_DYN, "we only support ET_EXEC or ET_DYN for now!");
    assert(header.e_phoff > 0, "we expect ph_off to be present!");
    assert(header.e_phnum > 0, "we expect ph_num to be present!");
    assert(header.e_shoff > 0, "we expect sh_off to be present!");
    assert(header.e_shnum > 0, "we expect sh_num to be present!");
    assert(header.e_phentsize == sizeof(Elf64_Phdr), "Elf64_Phdr is not of correct size as in the elf file!");
    assert(header.e_shentsize == sizeof(Elf64_Shdr), "Elf64_Shdr is not of correct size as in the elf file!");

    current = start + header.e_phoff;
    Elf64_Phdr* pheaders = (Elf64_Phdr*)current;

    // for (int i = 0; i < header.e_phnum; i++) {
    //     Elf64_Phdr pheader = pheaders[i];
    //     // printf("(%d/%d) pheader type: 0x%x\n", i, header.e_phnum, pheader.p_type);
    // }

    current = start + header.e_shoff;
    Elf64_Shdr* sheaders = (Elf64_Shdr*)current;
    // Elf64_Shdr* shstr = sheaders + header.e_shstrndx; // get the sheader at the index specified in the header.
    // u64 shstroffset = (u64)start + shstr->sh_offset; // get the section offset and add it the start.

    // for (int i = 0; i < header.e_shnum; i++) {
    //     Elf64_Shdr sheader = sheaders[i];
    //     u8* name = (u8*)(shstroffset + sheader.sh_name);

    //     // printf("(%d/%d) section:[%s] type: 0x%x name: 0x%x flags: 0x%lx virtual: 0x%lx sh_offset: 0x%lx sh_link: 0x%x\n",
    //     //  i, 
    //     //  header.e_shnum,
    //     //  name,
    //     //  sheader.sh_type, 
    //     //  sheader.sh_name, 
    //     //  sheader.sh_flags, 
    //     //  sheader.sh_addr,
    //     //  sheader.sh_offset,
    //     //  sheader.sh_link
    //     //  );
    // }

    // printf("diff: 0x%lx\n", (u64) (header_end - start) );
    // printf("e_entry: 0x%lx\n", header.e_entry );
    // printf("e_shoff: 0x%lx\n", header.e_shoff );
    // printf("e_phoff: 0x%lx\n", header.e_phoff );

    result.header = header;
    result.elf_module_start = (u64) buf;
    result.s_headers = sheaders;
    result.s_headers_len = header.e_phnum;
    result.p_headers = pheaders;
    result.p_headers_len = header.e_shnum;


    return result;


}
