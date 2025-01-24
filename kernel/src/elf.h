#ifndef __ASA_ELF
#define __ASA_ELF

#include "stdint.h"

typedef u64 Elf64_Off;
typedef u64 Elf64_Addr;
typedef u64 Elf64_Xword;
typedef u16 Elf64_Half;
typedef u32 Elf64_Word;


enum {
    EI_MAG0 = 0, // File identification
    EI_MAG1 = 1, //
    EI_MAG2 = 2, //
    EI_MAG3 = 3, //
    EI_CLASS = 4,  //File class
    EI_DATA = 5, // Data encoding
    EI_VERSION = 6, // File version
    EI_OSABI = 7, // OS/ABI identification
    EI_ABIVERSION = 8, // ABI version
    EI_PAD = 9, //Start of padding bytes
    EI_NIDENT = 16 //Size of e_ident[]
};

enum {
    ET_NONE = 0, // No file type
    ET_REL = 1, // Relocatable object file
    ET_EXEC = 2, // Executable file
    ET_DYN = 3, // Shared object file
    ET_CORE = 4, // Core file
    ET_LOOS = 0xFE00, // Environment-specific use
    ET_HIOS = 0xFEFF, //
    ET_LOPROC = 0xFF00, // Processor-specific use
    ET_HIPROC = 0xFFFF, //
};

enum {
    ELFCLASS32 = 1, // 32-bit objects
    ELFCLASS64 = 2, // 64-bit objects
};

enum {
    ELFDATA2LSB = 1, // Object file data structures are little-endian
    ELFDATA2MSB = 2, // Object file data structures are big-endian
};

enum {
    ELFOSABI_SYSV = 0, // System V ABI
    ELFOSABI_HPUX = 1, // HP-UX operating system
    ELFOSABI_STANDALONE = 255, // Standalone (embedded)
};

// ph_type
enum {
    PT_NULL = 0, // Unused entry
    PT_LOAD = 1, // Loadable segment
    PT_DYNAMIC = 2, // Dynamic linking tables
    PT_INTERP = 3, // Program interpreter path name
    PT_NOTE = 4, // Note sections
    PT_SHLIB = 5, // Reserved
    PT_PHDR = 6, // Program header table
    PT_LOOS = 0x60000000, // Environment-specific use
    PT_HIOS = 0x6FFFFFFF, //
    PT_LOPROC = 0x70000000, // Processor-specific use
    PT_HIPROC = 0x7FFFFFFF, //
    PT_GNU_RELRO = 0x6474e552, // some gnu extension
};


// p_flags
enum {
    PF_X = 0x1, // Execute permission
    PF_W = 0x2, // Write permission
    PF_R = 0x4, // Read permission
    PF_MASKOS = 0x00FF0000, // These flag bits are reserved for environment specific use
    PF_MASKPROC = 0xFF000000, // These flag bits are reserved for processor specific use
};


// Section Types, sh_type
enum {
    SHT_NULL = 0, // Marks an unused section header
    SHT_PROGBITS = 1, // Contains information defined by the program
    SHT_SYMTAB = 2, // Contains a linker symbol table
    SHT_STRTAB = 3, // Contains a string table
    SHT_RELA = 4, // Contains “Rela” type relocation entries
    SHT_HASH = 5, // Contains a symbol hash table
    SHT_DYNAMIC = 6, // Contains dynamic linking tables
    SHT_NOTE = 7, // Contains note information
    SHT_NOBITS = 8, // Contains uninitialized space; does not occupy any space in the file
    SHT_REL = 9, // Contains “Rel” type relocation entries
    SHT_SHLIB = 10, // Reserved
    SHT_DYNSYM = 11, // Contains a dynamic loader symbol table
    SHT_LOOS = 0x60000000, // Environment-specific use
    SHT_HIOS = 0x6FFFFFFF, //
    SHT_LOPROC = 0x70000000, // Processor-specific use
    SHT_HIPROC = 0x7FFFFFFF, //
};

// Table 9. Section Attributes, sh_flags
enum {
    SHF_WRITE = 0x1, // Section contains writable data
    SHF_ALLOC = 0x2, // Section is allocated in memory image of program
    SHF_EXECINSTR = 0x4, // Section contains executable instructions
    SHF_MASKOS = 0x0F000000, // Environment-specific use
    SHF_MASKPROC = 0xF0000000, // Processor-specific use
};


typedef struct
{
    unsigned char e_ident[EI_NIDENT]; /* ELF identification */
    Elf64_Half e_type; /* Object file type */
    Elf64_Half e_machine; /* Machine type */
    Elf64_Word e_version; /* Object file version */
    Elf64_Addr e_entry; /* Entry point address */
    Elf64_Off e_phoff; /* Program header offset */
    Elf64_Off e_shoff; /* Section header offset */
    Elf64_Word e_flags; /* Processor-specific flags */
    Elf64_Half e_ehsize; /* ELF header size */
    Elf64_Half e_phentsize; /* Size of program header entry */
    Elf64_Half e_phnum; /* Number of program header entries */
    Elf64_Half e_shentsize; /* Size of section header entry */
    Elf64_Half e_shnum; /* Number of section header entries */
    Elf64_Half e_shstrndx; /* Section name string table index */
} Elf64_Ehdr;

typedef struct
{
    Elf64_Word sh_name; /* Section name */
    Elf64_Word sh_type; /* Section type */
    Elf64_Xword sh_flags; /* Section attributes */
    Elf64_Addr sh_addr; /* Virtual address in memory */
    Elf64_Off sh_offset; /* Offset in file */
    Elf64_Xword sh_size; /* Size of section */
    Elf64_Word sh_link; /* Link to other section */
    Elf64_Word sh_info; /* Miscellaneous information */
    Elf64_Xword sh_addralign; /* Address alignment boundary */
    Elf64_Xword sh_entsize; /* Size of entries, if section has table */
} Elf64_Shdr;


typedef struct
{
    Elf64_Word p_type; /* Type of segment */
    Elf64_Word p_flags; /* Segment attributes */
    Elf64_Off p_offset; /* Offset in file */
    Elf64_Addr p_vaddr; /* Virtual address in memory */
    Elf64_Addr p_paddr; /* Reserved */
    Elf64_Xword p_filesz; /* Size of segment in file */
    Elf64_Xword p_memsz; /* Size of segment in memory */
    Elf64_Xword p_align; /* Alignment of segment */
} Elf64_Phdr;


typedef struct {
    u64 elf_module_start;
    Elf64_Ehdr header;

    Elf64_Shdr* s_headers;
    u64 s_headers_len;

    Elf64_Phdr* p_headers;
    u64 p_headers_len;
} Elf64;


Elf64 elf_parse(u8* buf, u64 len);

#endif
