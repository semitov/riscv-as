/*  semitov-riscv-as, Small RISC-V Assembler.
	Copyright (C) 2025 SemiTO-V Student Group <semitofive@gmail.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include "writer.h"
#include "error.h"
#include "instruction.h"

#include <elf.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// String offset inside shstrtab
#define STR_OFF_NULL 0
#define STR_OFF_TEXT 1
#define STR_OFF_DATA 7
#define STR_OFF_BSS 13
#define STR_OFF_SHSTRTAB 18

// Section header indexes
enum { SH_IDX_NULL = 0, SH_IDX_TEXT, SH_IDX_DATA, SH_IDX_BSS, SH_IDX_SHSTRTAB, SH_NUM_SECTIONS };

static void fill_ident32(unsigned char *ident) {
	memset(ident, 0, EI_NIDENT);

	ident[EI_MAG0] = ELFMAG0;
	ident[EI_MAG1] = ELFMAG1;
	ident[EI_MAG2] = ELFMAG2;
	ident[EI_MAG3] = ELFMAG3;
	// 32-bits
	ident[EI_CLASS] = ELFCLASS32;
	// Little-endian
	ident[EI_DATA] = ELFDATA2LSB;
	ident[EI_VERSION] = EV_CURRENT;
	ident[EI_OSABI] = ELFOSABI_NONE;
	ident[EI_ABIVERSION] = 0;
}

static void fill_elf_header(Elf32_Ehdr *ehdr, uint32_t entry_point, uint32_t shoff) {
	memset(ehdr, 0, sizeof(Elf32_Ehdr));
	fill_ident32(ehdr->e_ident);

	ehdr->e_type = ET_EXEC;
	ehdr->e_machine = EM_RISCV;
	ehdr->e_version = EV_CURRENT;
	ehdr->e_entry = entry_point;
	ehdr->e_flags = 0;
	ehdr->e_ehsize = sizeof(Elf32_Ehdr);
	ehdr->e_phoff = sizeof(Elf32_Ehdr);
	ehdr->e_phentsize = sizeof(Elf32_Phdr);
	// One load section
	ehdr->e_phnum = 1;
	ehdr->e_shoff = shoff;
	ehdr->e_shentsize = sizeof(Elf32_Shdr);
	ehdr->e_shnum = SH_NUM_SECTIONS;
	ehdr->e_shstrndx = SH_IDX_SHSTRTAB;
}

static void fill_program_header(Elf32_Phdr *phdr, uint32_t base_vaddr, uint32_t filesz, uint32_t memsz) {
	memset(phdr, 0, sizeof(Elf32_Phdr));
	phdr->p_type = PT_LOAD;
	phdr->p_offset = 0;
	phdr->p_vaddr = base_vaddr;
	phdr->p_paddr = base_vaddr;
	phdr->p_filesz = filesz;
	phdr->p_memsz = memsz;
	phdr->p_flags = PF_R | PF_W | PF_X;
	// 4KB
	phdr->p_align = 0x1000;
}

static void fill_section_header(Elf32_Shdr *shdr, uint32_t name_offset, uint32_t type, uint32_t flags, uint32_t addr,
								uint32_t offset, uint32_t size, uint32_t align) {
	memset(shdr, 0, sizeof(Elf32_Shdr));
	shdr->sh_name = name_offset;
	shdr->sh_type = type;
	shdr->sh_flags = flags;
	shdr->sh_addr = addr;
	shdr->sh_offset = offset;
	shdr->sh_size = size;
	shdr->sh_addralign = align;
}

assembler_error writer32(const char *filename, segment *segments, uint32_t base_vaddr) {
	if (!filename || !segments) {
		return ASSEMBLER_NULL_ERROR;
	}

	static const char shstrtab[] = "\0.text\0.data\0.bss\0.shstrtab";
	const size_t shstrtab_size = sizeof(shstrtab);

	const size_t text_len = segments[SEGMENT_TEXT].size;
	const size_t data_len = segments[SEGMENT_DATA].size;
	const size_t bss_len = segments[SEGMENT_BSS].size;

	const size_t text_offset = sizeof(Elf32_Ehdr) + sizeof(Elf32_Phdr);
	const size_t data_offset = text_offset + text_len;
	const size_t shstrtab_offset = data_offset + data_len;
	const size_t shoff = shstrtab_offset + shstrtab_size;

	Elf32_Ehdr elf_header;
	Elf32_Phdr program_header;
	Elf32_Shdr section_headers[SH_NUM_SECTIONS];

	fill_elf_header(&elf_header, base_vaddr + text_offset, shoff);
	fill_program_header(&program_header, base_vaddr, shstrtab_offset, shstrtab_offset + bss_len);

	memset(&section_headers[SH_IDX_NULL], 0, sizeof(Elf32_Shdr));

	fill_section_header(&section_headers[SH_IDX_TEXT], STR_OFF_TEXT, SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR,
						base_vaddr + text_offset, text_offset, text_len, 4);

	fill_section_header(&section_headers[SH_IDX_DATA], STR_OFF_DATA, SHT_PROGBITS, SHF_ALLOC | SHF_WRITE,
						base_vaddr + data_offset, data_offset, data_len, 4);

	fill_section_header(&section_headers[SH_IDX_BSS], STR_OFF_BSS, SHT_NOBITS, SHF_ALLOC | SHF_WRITE,
						base_vaddr + shstrtab_offset, shstrtab_offset, bss_len, 4);

	fill_section_header(&section_headers[SH_IDX_SHSTRTAB], STR_OFF_SHSTRTAB, SHT_STRTAB, 0, 0, shstrtab_offset,
						shstrtab_size, 1);

	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		return ASSEMBLER_ELF_ERROR;
	}

	// @TODO: improve error handling

	fwrite(&elf_header, sizeof(Elf32_Ehdr), 1, fp);
	fwrite(&program_header, sizeof(Elf32_Phdr), 1, fp);

	if (text_len > 0) {
		fwrite(segments[SEGMENT_TEXT].data, text_len, 1, fp);
	}

	if (data_len > 0) {
		fwrite(segments[SEGMENT_TEXT].data, text_len, 1, fp);
	}

	fwrite(shstrtab, shstrtab_size, 1, fp);
	fwrite(section_headers, sizeof(Elf32_Shdr), SH_NUM_SECTIONS, fp);

	fclose(fp);
	return ASSEMBLER_OK;
}
