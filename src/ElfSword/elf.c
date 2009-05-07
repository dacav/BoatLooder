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
    struct hsearch_data namtab; /* Hash optimzier for names */
};

const char *elf_symbol_name(elf_t elf, Elf32_Shdr *shdr, Elf32_Sym *yhdr)
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

void elf_section_content (elf_t elf, Elf32_Shdr *shdr,
                          void **cont, size_t *size)
{
    *cont = (void *)(elf->file.data8b + shdr->sh_offset);
    *size = shdr->sh_size;
}

const char *elf_section_name(elf_t elf, Elf32_Shdr *shdr)
{
    const Elf32_Shdr *names = elf->names;

    if (names != NULL) {
        return (const char *)elf->file.data + names->sh_offset +
                             shdr->sh_name;
    } else {
        return NULL;
    }
}

void elf_symbols_scan(elf_t elf, Elf32_Shdr *shdr, sym_scan_t callback,
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

void elf_sections_scan(elf_t elf, sec_scan_t callback, void *udata)
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
bool check_magic(elf_t elf)
{
    unsigned char *magic;

    magic = ((Elf32_Ehdr *)elf->file.data)->e_ident;
    return magic[EI_MAG0] == ELFMAG0 &&
           magic[EI_MAG1] == ELFMAG1 &&
           magic[EI_MAG2] == ELFMAG2 &&
           magic[EI_MAG3] == ELFMAG3;
}

bool elf_release_file(elf_t elf)
{
    int ret;

    if (elf != NULL) {
        ret  = munmap(elf->file.data, elf->len);
        ret += close(elf->fd);
        hdestroy_r(&elf->namtab);
        free(elf);
        return ret >= 0;
    } else {
        return false;
    }
}

static
bool hash_builder(void *udata, elf_t elf, Elf32_Shdr *shdr)
{
    struct hsearch_data *namtab;
    ENTRY e, *ret;

    namtab = (struct hsearch_data *)udata;
    e.key = (char *)elf_section_name(elf, shdr);
    e.data = (void *)shdr;
    assert(hsearch_r(e, ENTER, &ret, namtab) != 0);

    return true;
}

elf_t elf_map_file(const char *filename)
{
    int fd;
    struct stat buf;
    size_t len;
    uint8_t *secarray;
    elf_t elf;
    Elf32_Ehdr *header;
    struct hsearch_data *namtab;

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
    namtab = &elf->namtab;
    memset(namtab, 0, sizeof(struct hsearch_data));
    assert(hcreate_r(len, namtab));
    elf_sections_scan(elf, hash_builder, (void *)namtab);
    
    return elf;

  fail2:
    munmap(elf->file.data, elf->len);
    close(elf->fd);
  fail1:
    close(fd);
  fail0:
    free(elf);
    return NULL;
}

Elf32_Shdr *elf_section_get(elf_t elf, const char *secname)
{
    ENTRY key = {
        .key = (char *)secname,
        .data = NULL
    };
    ENTRY *entry;

    return hsearch_r(key, FIND, &entry, &elf->namtab) == 0
           ? NULL
           : (Elf32_Shdr *) entry->data;
}

