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
#ifndef __ELF_H__
#define __ELF_H__

#include <stdbool.h>
#include <stdlib.h>
#include "elf_specification.h"

/** ELF structure information holding data type */
typedef struct elf_struct * Elf;

/** ELF file mapper
 *
 * Produces an Elf object by mapping the given file in memory
 *
 * @param filename The name of the ELF file to be mapped;
 * @return an Elf object or NULL on failure (i.e. invalid file).
 */
Elf elf_map_file(const char *filename);

/** ELF file releaser
 *
 * Frees the Elf object
 *
 * @param elf The Elf object to be freed;
 * @return true on success, false on failure.
 */
bool elf_release_file(Elf elf);

/** Returns a const pointer to the first byte of the ELF file mapping
 *
 * @param elf The ELF file;
 * @return a const pointer to the first byte.
 */
const uint8_t * elf_get_content(Elf elf);

/** Section getter
 *
 * Retrieves a section by searching the given name on the sections hash
 * table
 *
 * @note The hash table is part of the Elf structure: its content is
 *       mapped by elf_map_file
 *
 * @param elf The Elf object;
 * @param secname The name of the section;
 * @return A pointer to the section header or NULL if there's no such
 *         section
 */
Elf32_Shdr * elf_section_get(Elf elf, const char *secname);

/** Section name getter
 *
 * Retrieves the name of the given section from the ELF string table
 *
 * @param elf The Elf object;
 * @param shdr The section header;
 * @return The section name or NULL if the ELF file doesn't have a string
 *         table.
 */
const char * elf_section_name(Elf elf, Elf32_Shdr *shdr);

/** Section content retriever
 *
 * Returns the position of the section's content and its length through
 * parameters side effect.
 *
 * @param elf The Elf object;
 * @param shdr The section header;
 * @param cont The pointer to be moved on the section content;
 * @param size Will contain the size of the section content in bytes.
 */
void elf_section_content(Elf elf, Elf32_Shdr *shdr, void **cont,
                         size_t *size);

/** Symbol name getter
 *
 * Retrieves the name of the given symbol from the correct string table
 *
 * @param elf The Elf object;
 * @param shdr The section holding the symbol;
 * @param yhdr The symbol header;
 * @return The symbol name or NULL on failure.
 */
const char * elf_symbol_name(Elf elf, Elf32_Shdr *shdr,
                             Elf32_Sym *yhdr);

/** Iteration function for section scanning
 *
 * @param udata User data;
 * @param elf The Elf object;
 * @param shdr A pointer to the section header;
 * @return If false the iteration will be stopped.
 */
typedef bool (*SecScan)(void *udata, Elf elf, Elf32_Shdr *shdr);

/** Scans through the sections
 *
 * The callback function provided (@see SecScan) gets called for each
 * section in the ELF file
 *
 * @note If the callback returns false the iteration will be stopped.
 *
 * @param elf The Elf object;
 * @param callback The callback to be called;
 * @param udata User data for the callback.
 */
void elf_sections_scan(Elf elf, SecScan callback, void *udata);

/** Iteration function for section's symbols scanning
 *
 * @param udata User data;
 * @param elf The Elf object;
 * @param shdr A pointer to the section header;
 * @param yhdr A pointer to the symbol header;
 * @return If false the iteration will be stopped.
 */
typedef bool (*SymScan)(void *udata, Elf elf, Elf32_Shdr *shdr,
                           Elf32_Sym *yhdr);

/** Scans through a section's symbols
 *
 * The callback function provided (@see SymScan) gets called for each
 * symbol in the given section.
 *
 * @note If the callback returns false the iteration will be stopped.
 *
 * @param elf The Elf object;
 * @param shdr The section header;
 * @param callback The callback to be called;
 * @param udata User data for the callback.
 */
void elf_symbols_scan(Elf elf, Elf32_Shdr *shdr, SymScan callback,
                      void *udata);

/** Iteration function for program header's entry scanning
 *
 * @param udata User data;
 * @param elf The Elf object;
 * @param phdr The program header entry
 * @return If false the iteration will be stopped.
 */
typedef bool (*PHeaderScan)(void *udata, Elf elf, Elf32_Phdr *phdr);

/** Scans through the program header array
 *
 * The callback function provided (@see PHeaderScan) gets called for each
 * entry in the program header
 *
 * @param elf The Elf object;
 * @param callback The callback to be called;
 * @param udata User data for the callback.
 * @return false if the ELF file doesn't have a program header, true
 *         oterwise
 */
bool elf_progheader_scan(Elf elf, PHeaderScan callback, void *udata);

#endif /* __ELF_H__ */
