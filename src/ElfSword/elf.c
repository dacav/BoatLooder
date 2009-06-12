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
#include "elf.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <search.h>

/* Elf mapping type */
struct elf_struct {

    /* Allocated data */
    union {
        void *data;             /* Memory mapped file */
        uint8_t *data8b;        /* 8 bit pointer */
        Elf32_Ehdr *header;     /* Elf header */
    } file;
    size_t len;                 /* File size */
    int fd;                     /* File descriptor */

    /* Auxiliary data */
    Elf32_Shdr *names;          /* Section for name resolving */
    struct hsearch_data sectab; /* Hash optimzier for sections */
    struct hsearch_data symtab; /* Hash optimzier for symbols */

    int symtab_magic;           /* Magic counter for symbol table: */

    /*
       ON MAGIC COUNTER:

       While building the stdlib hash tables, it's necessary to provide
       the size of the hash. If the hash table is overflowing there's no
       way to put another element in.

       The hash table is constructed in three steps:

       - The first time "elf_section_get" is called, a complete scan is
         performed, and while searching the algorithm will count the
         number of registred symbols. This number, multiplied by -1, is
         registred into the symtab_magic field of this structure;

       - On the second search, since the number of symbols is known, the
         search function builds the hash table. Since the number of
         symbols is still negative the algorithm will scan all symbols and
         store the names into the structure;

       - From the third search on the hash table is used.

     */
};

const char *elf_symbol_name(Elf elf, Elf32_Shdr *shdr, Elf32_Sym *yhdr)
{
    const Elf32_Word sh_type = shdr->sh_type;
    const Elf32_Ehdr *header = elf->file.header;

    if (sh_type != SHT_SYMTAB && sh_type != SHT_DYNSYM)
        return NULL;
    if (yhdr->st_name == 0)
        return NULL;

    /* shdr->sh_link contains the index of the associated string table.
     * Moving on the correct table */
    shdr = (Elf32_Shdr *) ((elf->file.data8b + header->e_shoff)
                           + (header->e_sheentsize * shdr->sh_link));
    return (const char *)elf->file.data + shdr->sh_offset + yhdr->st_name;
}

void elf_section_content (Elf elf, Elf32_Shdr *shdr,
                          void **cont, size_t *size)
{
    if (cont != NULL)
        *cont = (void *)(elf->file.data8b + shdr->sh_offset);
    if (size != NULL)
        *size = shdr->sh_size;
}

const char *elf_section_name(Elf elf, Elf32_Shdr *shdr)
{
    const Elf32_Shdr *names = elf->names;

    if (names != NULL) {
        return (const char *)elf->file.data + names->sh_offset +
                             shdr->sh_name;
    } else {
        return NULL;
    }
}

void elf_symbols_scan(Elf elf, Elf32_Shdr *shdr, SymScan callback,
                      void *udata)
{
    const Elf32_Word sh_type = shdr->sh_type;
    Elf32_Sym *cursor;
    size_t nentr;

    if (sh_type != SHT_SYMTAB && sh_type != SHT_DYNSYM)
        return;

    nentr = shdr->sh_size / sizeof(Elf32_Sym);
    cursor = (Elf32_Sym *)(elf->file.data8b + shdr->sh_offset);
    while (nentr --) {
        if (!callback(udata, elf, shdr, cursor))
            break;
        cursor ++;
    }
}

void elf_sections_scan(Elf elf, SecScan callback, void *udata)
{
    Elf32_Ehdr *header;
    Elf32_Shdr *cursor;
    uint32_t sec_count;
    size_t sec_size;

    header = elf->file.header;
    cursor = (Elf32_Shdr *)(elf->file.data8b + header->e_shoff);
    sec_count = header->e_shnum;
    sec_size = header->e_sheentsize;

    while (sec_count --) {
        if (!callback(udata, elf, cursor))
            break;
        cursor = (Elf32_Shdr *)((uint8_t *)cursor + sec_size);
    }
}

static
bool check_magic(Elf elf)
{
    unsigned char *magic;

    magic = ((Elf32_Ehdr *)elf->file.data)->e_ident;
    return magic[EI_MAG0] == ELFMAG0 &&
           magic[EI_MAG1] == ELFMAG1 &&
           magic[EI_MAG2] == ELFMAG2 &&
           magic[EI_MAG3] == ELFMAG3;
}

bool elf_release_file(Elf elf)
{
    int ret;

    if (elf != NULL) {
        ret  = munmap(elf->file.data, elf->len);
        ret += close(elf->fd);
        hdestroy_r(&elf->sectab);
        if (elf->symtab_magic > 0)
            hdestroy_r(&elf->symtab);
        free(elf);
        return ret >= 0;
    } else {
        return false;
    }
}

static
bool hash_builder(void *udata, Elf elf, Elf32_Shdr *shdr)
{
    struct hsearch_data *sectab;
    ENTRY e, *ret;

    sectab = (struct hsearch_data *)udata;
    e.key = (char *)elf_section_name(elf, shdr);
    e.data = (void *)shdr;
    assert(hsearch_r(e, ENTER, &ret, sectab) != 0);

    return true;
}

Elf elf_map_file(const char *filename)
{
    int fd;
    struct stat buf;
    size_t len;
    uint8_t *secarray;
    Elf elf;
    Elf32_Ehdr *header;
    struct hsearch_data *sectab;

    /* Control structure allocation */
    elf = malloc(sizeof(struct elf_struct));
    assert(elf != NULL);

    /* File mapping */
    fd = open(filename, O_RDONLY);
    if (fd == -1)
        goto fail0;
    if (fstat(fd, &buf) == -1)
        goto fail1;
    elf->len = len = buf.st_size;
    elf->file.data = mmap(NULL, len, PROT_READ, MAP_SHARED, fd, 0);
    elf->fd = fd;

    /* Magic number checking */
    if (!check_magic(elf))
        goto fail2;
   
    /* Section names retriving */
    header = elf->file.header;
    len = header->e_sheentsize;
    secarray = (elf->file.data8b + header->e_shoff);
    if (header->e_shstrndx == SHN_UNDEF)
        elf->names = NULL;
    else
        elf->names = (Elf32_Shdr *) (secarray + header->e_shstrndx * len);

    /* Hash for name optimizations */
    sectab = &elf->sectab;
    memset(sectab, 0, sizeof(struct hsearch_data));
    assert(hcreate_r(len, sectab));
    elf_sections_scan(elf, hash_builder, (void *)sectab);

    /* Setting sections magic index */
    elf->symtab_magic = 0;
    
    return elf;

  fail2:
    munmap(elf->file.data, elf->len);
  fail1:
    close(fd);
  fail0:
    free(elf);
    return NULL;
}

const uint8_t * elf_get_content(Elf elf)
{
    return elf->file.data8b;
}

Elf32_Shdr *elf_section_get(Elf elf, const char *secname)
{
    ENTRY key = {
        .key = (char *)secname,
        .data = NULL
    };
    ENTRY *entry;

    return hsearch_r(key, FIND, &entry, &elf->sectab) == 0
           ? NULL
           : (Elf32_Shdr *) entry->data;
}


bool elf_progheader_scan(Elf elf, PHeaderScan callback, void *udata)
{
    size_t nents, size;
    Elf32_Ehdr *header;
    Elf32_Phdr *cursor;

    header = elf->file.header;
    nents = header->e_phnum;
    if (nents == 0)
        return false;
    size = header->e_phentsize;
    cursor = (Elf32_Phdr *)(elf->file.data8b + header->e_phoff);
    while (nents--) {
        if (!callback(udata, elf, cursor))
            break;
        cursor = (Elf32_Phdr *)((uint8_t *)cursor + size);
    }

    return true;
}

struct track {
    Elf32_Sym *ret;
    int oldmagic;
    const char *name;
};

static
bool sym_scanner(void *udata, Elf elf, Elf32_Shdr *shdr,
                 Elf32_Sym *yhdr)
{
    struct track *t;
    const char *sym_name;
    ENTRY e, *ret;

    t = (struct track *)udata;

    /* sym_name may be null if the symbol has no name */
    if ((sym_name = elf_symbol_name(elf, shdr, yhdr)) != NULL) {
        if (strcmp(t->name, sym_name) == 0) {
            /* We found the searched symbol */
            t->ret = yhdr;
        }
        if (t->oldmagic == 0) {
            /* We are still counting elements */
            elf->symtab_magic ++;
        } else {
            /* We are filling the table with symbol names */
            e.key = (char *)sym_name;third
            e.data = (void *)yhdr;
            assert(hsearch_r(e, ENTER, &ret, &elf->symtab));
        }
    }
    return true;
}

Elf32_Sym *elf_symbol_get(Elf elf, const char *name)
{
    Elf32_Shdr *symtab;
    struct hsearch_data *htab;
    ENTRY key, *entry;
    struct track t;

    symtab = elf_section_get(elf, ".symtab");
    if (symtab == NULL)
        return NULL;

    t.oldmagic = elf->symtab_magic;
    if (t.oldmagic <= 0) {
        if (t.oldmagic != 0) {
            /* We know the number of elements; building hash table */
            htab = &elf->symtab;
            memset(htab, 0, sizeof(struct hsearch_data));
            assert(hcreate_r(-t.oldmagic, htab));
        }
        t.name = name;
        t.ret = NULL;
        elf_symbols_scan(elf, symtab, sym_scanner, (void *)&t);
        elf->symtab_magic *= -1;
        return t.ret;
    } else {
        /* Elements are already stored into the hash table, so we can use
         * it */
        key.key = (char *)name;
        key.data = NULL;
        return hsearch_r(key, FIND, &entry, &elf->symtab) == 0
               ? NULL
               : (Elf32_Sym *) entry->data;
    }
}

