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
typedef struct elf_struct * elf_t;

/** ELF file mapper
 *
 * Produces an elf_t object by mapping the given file in memory
 *
 * @param filename The name of the ELF file to be mapped;
 * @return an elf_t object or NULL on failure (i.e. invalid file).
 */
elf_t elf_map_file(const char *filename);

/** ELF file releaser
 *
 * Frees the elf_t object
 *
 * @param elf The elf_t object to be freed;
 * @return true on success, false on failure.
 */
bool elf_release_file(elf_t elf);

/** Section getter
 *
 * Retrieves a section by searching the given name on the sections hash
 * table
 *
 * @note The hash table is part of the elf_t structure: its content is
 *       mapped by elf_map_file
 *
 * @param elf The elf_t object;
 * @param secname The name of the section;
 * @return A pointer to the section header or NULL if there's no such
 *         section
 */
Elf32_Shdr * elf_section_get(elf_t elf, const char *secname);

/** Section name getter
 *
 * Retrieves the name of the given section from the ELF string table
 *
 * @param elf The elf_t object;
 * @param shdr The section header;
 * @return The section name or NULL if the ELF file doesn't have a string
 *         table.
 */
const char * elf_section_name(elf_t elf, Elf32_Shdr *shdr);

/** Section content retriever
 *
 * Returns the position of the section's content and its length through
 * parameters side effect.
 *
 * @param elf The elf_t object;
 * @param shdr The section header;
 * @param cont The pointer to be moved on the section content;
 * @param size Will contain the size of the section content in bytes.
 */
void elf_section_content(elf_t elf, Elf32_Shdr *shdr, void **cont,
                         size_t *size);

/** Symbol name getter
 *
 * Retrieves the name of the given symbol from the correct string table
 *
 * @param elf The elf_t object;
 * @param shdr The section holding the symbol;
 * @param yhdr The symbol header;
 * @return The symbol name or NULL on failure.
 */
const char * elf_symbol_name(elf_t elf, Elf32_Shdr *shdr,
                             Elf32_Sym *yhdr);

/** Iteration function for section scanning
 *
 * @param udata User data;
 * @param elf The elf_t object;
 * @param shdr A pointer to the section header;
 * @return If false the iteration will be stopped.
 */
typedef bool (*sec_scan_t)(void *udata, elf_t elf, Elf32_Shdr *shdr);

/** Iteration function for section's symbols scanning
 *
 * @param udata User data;
 * @param elf The elf_t object;
 * @param shdr A pointer to the section header;
 * @param yhdr A pointer to the symbol header;
 * @return If false the iteration will be stopped.
 */
typedef bool (*sym_scan_t)(void *udata, elf_t elf, Elf32_Shdr *shdr,
                           Elf32_Sym *yhdr);

/** Scans through the sections
 *
 * The callback function provided (@see sec_scan_t) gets called for each
 * section in the ELF file
 *
 * @note If the callback returns false the iteration will be stopped.
 *
 * @param elf The elf_t object;
 * @param callback The callback to be called;
 * @param udata User data for the callback.
 */
void elf_sections_scan(elf_t elf, sec_scan_t callback, void *udata);

/** Scans through a section's symbols
 *
 * The callback function provided (@see sym_scan_t) gets called for each
 * symbol in the given section.
 *
 * @note If the callback returns false the iteration will be stopped.
 *
 * @param elf The elf_t object;
 * @param shdr The section header;
 * @param callback The callback to be called;
 * @param udata User data for the callback.
 */
void elf_symbols_scan(elf_t elf, Elf32_Shdr *shdr, sym_scan_t callback,
                      void *udata);

#endif /* __ELF_H__ */
