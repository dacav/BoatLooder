/*
 * Copyright 2009 Giovanni Simoni
 *
 * This file is part of ElfSword.
 *
 * ElfSword is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ElfSword is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ElfSword.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __ELF_SPECIFICATION_H__
#define __ELF_SPECIFICATION_H__

#include <stdint.h>

/* This file specifies data structures representing the internal of an ELF
 * binary file. For further information please refer to ELF documentation
 */

/* -------------------------------------------------------------------- */
/* Elf specification data types                                         */
/* -------------------------------------------------------------------- */

typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef int32_t Elf32_Sword;
typedef uint32_t Elf32_Word;

/* -------------------------------------------------------------------- */
/* ELF File header                                                      */
/* -------------------------------------------------------------------- */

#define EI_NINDENT 16
typedef struct {
    unsigned char   e_ident[EI_NINDENT]; /* Magic number */
    Elf32_Half      e_type;              /* Object file type */
    Elf32_Half      e_machine;           /* Architecture */
    Elf32_Word      e_version;           /* Object file version */
    Elf32_Addr      e_entry;             /* VMA for process starting */
    Elf32_Off       e_phoff;             /* Program header table offset */
    Elf32_Off       e_shoff;             /* Section header table offset */
    Elf32_Word      e_flags;             /* Processor flags */
    Elf32_Half      e_ehsize;            /* Header size */
    Elf32_Half      e_phentsize;         /* Header table entry size */
    Elf32_Half      e_phnum;             /* Header number of entries */
    Elf32_Half      e_sheentsize;        /* Section header's size */
    Elf32_Half      e_shnum;             /* Section hdr number of entries */
    Elf32_Half      e_shstrndx;          /* Section hdr string table */
} Elf32_Ehdr;

/* e_ident indexes for information positions. This includes magic
 * number for ELF, class, data, version and pad
 */
enum {
    EI_MAG0 = 0,
    EI_MAG1 = 1,
    EI_MAG2 = 2,
    EI_MAG3 = 3,
    EI_CLASS = 4,
    EI_DATA = 5,
    EI_VERSION = 6,
    EI_PAD = 7
};

/* e_ident values for EI_MAG0-3 */
enum {
    ELFMAG0 = 0x7f,
    ELFMAG1 = 'E',
    ELFMAG2 = 'L',
    ELFMAG3 = 'F'
};

/* e_ident values for EI_CLASS */
enum {
    ELFCLASSNONE = 0,                    /* Invalid */
    ELFCLASS32 = 1,                      /* 32-bit object */
    ELFCLASS64 = 2                       /* 64-bit object */
};

/* e_ident values for EI_DATA */
enum {
    ELFDATANONE = 0,                     /* Invalid */
    ELFDATA2LSB = 1,                     /* Least significative */
    ELFDATA2MSB = 2                      /* Most significative */
};

/* e_type field values */
enum {
    ET_NONE = 0,                         /* No file type */
    ET_REL = 1,                          /* Relocatable */
    ET_EXEC = 2,                         /* Executable */
    ET_DYN = 3,                          /* Shared object */
    ET_CORE = 4,                         /* Core file */
    ET_LOPROC = 0xff00,                  /* Processor specific */
    ET_HIPROC = 0xffff                   /* Processor specific */
};

/* e_machine filed values */
enum {
    EM_NONE = 0,                         /* No machine */
    EM_M32 = 1,                          /* AT&T WE 32100 */
    EM_SPARC = 2,                        /* SPARC */
    EM_386 = 3,                          /* Intel 80386 */
    EM_68K = 4,                          /* Motorola 68000 */
    EM_88K = 5,                          /* Motorola 88000 */
    EM_860 = 7,                          /* Intel 80860 */
    EM_MIPS = 8,                         /* MIS RS3000 Big Endian */
    EM_MIPS_RS4_BE = 10                  /* MIS RS3000 Big Endian */
};

/* e_version field values */
enum {
    EV_NONE = 0,                         /* Invalid version */
    EV_CURRENT = 1                       /* Current version */
};

/* -------------------------------------------------------------------- */
/* Section header                                                       */
/* -------------------------------------------------------------------- */

/* The section headers are stored as an array of memory chunks. The file
 * offset of the array, the size of each element and the number of
 * elements are stored into the main ELF header. */

/* The following headers indexes are reserved */
enum {
    SHN_UNDEF = 0,                       /* Undefined section */
    SHN_LORESERVE = 0xff00,              /* Lower bound (inclusive) of the */
                                         /* reserved range */
    SHN_LOPROC = 0xff00,                 /* Processor specific low bound */
    SHN_HIPROC = 0xff1f,                 /* Processor specific high bound */
    SHN_ABS = 0xfff1,                    /* Absoulute references values */
    SHN_COMMON = 0xfff2,                 /* Common symbols */
    SHN_HIRESERVE = 0xffff               /* Upper bound (inclusive) of the */
                                         /* reserved range */
};

typedef struct {
    Elf32_Word      sh_name;             /* Index of the name */
    Elf32_Word      sh_type;             /* Type of section */
    Elf32_Word      sh_flags;            /* Section flags */
    Elf32_Addr      sh_addr;             /* Virtual memory address (VMA) */
    Elf32_Off       sh_offset;           /* Referred section position */
    Elf32_Word      sh_size;             /* Referred section size */
    Elf32_Word      sh_link;             /* Link for other section headers */
    Elf32_Word      sh_info;             /* Extra informations */
    Elf32_Word      sh_addralign;        /* Alignment constraints */
    Elf32_Word      sh_entsize;          /* Size of section's sub-entries */
                                         /* (0 if there's no sub-entries) */
} Elf32_Shdr;

/* sh_type field values */
enum {
    SHT_NULL = 0,                        /* Section header inactive */
    SHT_PROGBITS = 1,                    /* Program defined content */
    SHT_SYMTAB = 2,                      /* Contains link editing symbols */
    SHT_STRTAB = 3,                      /* Contains a string table */
    SHT_RELA = 4,                        /* Contains relocation entries */
    SHT_HASH = 5,                        /* Contains a symbol hash table */
    SHT_DYNAMIC = 6,                     /* Information for dynamic linking */
    SHT_NOTE = 7,                        /* Note section */
    SHT_NOBITS = 8,                      /* Contains no data, conceptual */
                                         /* offset */
    SHT_REL = 9,                         /* Contains relocation entries
                                          * without addends */
    SHT_SHLIB = 10,                      /* Reserved */
    SHT_DYNSYM = 11,                     /* Contains link editing symbols */
    SHT_LOPROC = 0x70000000,             /* Lower bound (inclusive) for */
                                         /* processor specific types */
    SHT_HIPROC = 0x7FFFFFFF,             /* Upper bound (inclusive) for */
                                         /* processor specific types */
    SHT_LOUSER = 0x80000000,             /* Lower bound (inclusive) for */
                                         /* application programs */
    SHT_HIUSER = 0xFFFFFFFF              /* Upper bound (inclusive) for */
                                         /* papplication programs */
};

/* sh_flags field values, used as bitwise or */
enum {
    SHF_WRITE = 0x1,                     /* Writable by process */
    SHF_ALLOC = 0x2,                     /* Occupies memory only during */
                                         /* execution */
    SHF_EXECINSTR = 0x4,                 /* Executable code */
    SHF_MASKPROC = 0xf0000000            /* Mask for processor specific */
                                         /* flags */
};

/* -------------------------------------------------------------------- */
/* Symbols header                                                       */
/* -------------------------------------------------------------------- */

/* As for section headers, symbol headers are stored as an array of memory
 * chunks.
 */

typedef struct {
    Elf32_Word      st_name;            /* Name index */
    Elf32_Addr      st_value;           /* Symbol value */
    Elf32_Word      st_size;            /* Size in bytes */
    unsigned char   st_info;            /* Symbol type and binding */
                                        /* attributes */
    unsigned char   st_other;           /* No meaning */
    Elf32_Half      st_shndx;           /* Belonging section index */
} Elf32_Sym;

/* st_info manipulation macros */
#define ELF32_ST_BIND(i)    ((i)>>4)
#define ELF32_ST_TYPE(i)    ((i)&0xf)
#define ELF32_ST_INFO(b,t)  (((b)<<4)+((t)&0xf))

/* st_info binding values */
enum {
    STB_LOCAL = 0,                      /* Locally scoped symbol */
    STB_GLOBAL = 1,                     /* Globally scoped symbol */
    STB_WEAK = 2,                       /* Globally scoped, lower */
                                        /* precedence */
    STB_LOPROC = 13,                    /* Low bound (inclusive) for */
                                        /* processor specific binding */
    STB_HIPROC = 15                     /* High bound (inclusive) for */
                                        /* processor specific binding */
};

/* st_info type values */
enum {
    STT_NOTYPE = 0,                     /* Not specified */
    STT_OBJECT = 1,                     /* Data object */
    STT_FUNC = 2,                       /* Function or executable */
    STT_SECTION = 3,                    /* Section (used for relocation) */
    STT_FILE = 4,                       /* Source file associated with the */
                                        /* object file */
    STT_LOPROC = 13,                    /* Low bound (inclusive) for */
                                        /* processor specific type */
    STT_HIPROC = 15                     /* High bound (inclusive) for */
                                        /* processor specific type */
};

/* -------------------------------------------------------------------- */
/* Program header                                                       */
/* -------------------------------------------------------------------- */

typedef struct {
    Elf32_Word      p_type;             /* Type of segment */
    Elf32_Off       p_offset;           /* Offset from the file's
                                         * beginning of the first byte of
                                         * the segment */
    Elf32_Addr      p_vaddr;            /* Virtual address of the first
                                         * byte */
    Elf32_Addr      p_paddr;            /* Reserved for phisical address */
    Elf32_Word      p_filesz;           /* Size of the segment's file image */
    Elf32_Word      p_memsz;            /* Size of the segment's memory
                                         * image */
    Elf32_Word      p_flags;            /* Segment flags */
    Elf32_Word      p_align;            /* Segment alignment */
} Elf32_Phdr;

/* p_type field values */
enum {
    PT_NULL = 0,                        /* Array element unused */
    PT_LOAD = 1,                        /* Loadable segment */
    PT_DYNAMIC = 2,                     /* Element specifying dynamic
                                         * linking info */
    PT_INTERP = 3,                      /* Null terminated path of
                                         * interpreter */
    PT_NOTE = 4,                        /* Auxiliary information */
    PT_SHLIB = 5,                       /* Reserved */
    PT_PHDR = 6,                        /* Location/size of the program
                                         * header itself */
    PT_LOPROC = 0x70000000,             /* Processor specific reserved */
    PT_HIPROC = 0x7fffffff              /* Processor specific reserved */
};

/* p_flags field values */
enum {
    PF_X = 0x01,                        /* Execute permission */
    PF_W = 0x02,                        /* Write permission */
    PF_R = 0x04,                        /* Read permission */
    PF_MASKPROC = 0xf0000000            /* Mask for processor specific */
};

/* -------------------------------------------------------------------- */
/* Program header                                                       */
/* -------------------------------------------------------------------- */

typedef struct {
    Elf32_Sword d_tag;                  /* Interpretation controller */
    union {
        Elf32_Word d_val;               /* Integer representation */
        Elf32_Addr d_ptr;               /* Pointer representation */
    } d_un;
} Elf32_Dyn;

/* d_tag field values */
enum {
    DT_NULL = 0,                        /* End of dynamic array; Ignore
                                         * d_un */
    DT_NEEDED = 1,                      /* Use d_val */
    DT_PLTRELSZ = 2,                    /* Use d_val */
    DT_PLTGOT = 3,                      /* Use d_ptr */
    DT_HASH = 4,                        /* Use d_ptr */
    DT_STRTAB = 5,                      /* Use d_ptr */
    DT_SYMTAB = 6,                      /* Use d_ptr */
    DT_RELA = 7,                        /* Use d_ptr */
    DT_RELASZ = 8,                      /* Use d_val */
    DT_RELAENT = 9,                     /* Use d_val */
    DT_STRSZ = 10,                      /* Use d_val */
    DT_SYMENT = 11,                     /* Use d_val */
    DT_INIT = 12,                       /* Use d_ptr */
    DT_FINI = 13,                       /* Use d_ptr */
    DT_SONAME = 14,                     /* Use d_val */
    DT_RPATH = 15,                      /* Use d_val */
    DT_SYMBOLIC = 16,                   /* Ignore d_un */
    DT_REL = 17,                        /* Use d_ptr */
    DT_RELSZ = 18,                      /* Use d_val */
    DT_RELENT = 19,                     /* Use d_val */
    DT_PLTREL = 20,                     /* Use d_val */
    DT_DEBUG = 21,                      /* Use d_ptr */
    DT_TEXTREL = 22,                    /* Ignore d_un */
    DT_JMPREL = 23,                     /* Use d_ptr */
    DT_BIND_NOW = 24,                   /* Ignore d_un */
    DT_LOPROC = 0x70000000,             /* Use d_val */
    DT_HIPROC = 0x7fffffff              /* Use d_val */
};

#endif /* __ELF_SPECIFICATION_H__ */
