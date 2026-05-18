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

#include <elf.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void fill_ident32(unsigned char *ident) {
	memset(ident, 0, EI_NIDENT);

	ident[EI_MAG0] = ELFMAG0;
	ident[EI_MAG1] = ELFMAG1;
	ident[EI_MAG2] = ELFMAG2;
	ident[EI_MAG3] = ELFMAG3;

	/* 32 bits elf */
	ident[EI_CLASS] = ELFCLASS32;
	/* Little-endian */
	ident[EI_DATA] = ELFDATA2LSB;
	ident[EI_VERSION] = EV_CURRENT;

	ident[EI_OSABI] = ELFOSABI_NONE;
	ident[EI_ABIVERSION] = 0;
}

static void fill_elf_header(Elf32_Ehdr *elf_header, size_t code_offset, size_t shoff, size_t shnum, size_t shstrndx,
							uint32_t base_vaddr) {
	memset(elf_header, 0, sizeof(*elf_header));

	fill_ident32(elf_header->e_ident);

	elf_header->e_type = ET_EXEC;
	elf_header->e_machine = EM_RISCV;
	elf_header->e_version = EV_CURRENT;

	/* Virtual address of code */
	elf_header->e_entry = base_vaddr + code_offset;
	// elf_header->e_flags = EF_RISCV_RVC;
	elf_header->e_flags = 0;
	elf_header->e_ehsize = sizeof(Elf32_Ehdr);

	/* Program header starts after ELF header */
	elf_header->e_phoff = sizeof(Elf32_Ehdr);
	elf_header->e_phentsize = sizeof(Elf32_Phdr);
	elf_header->e_phnum = 1;

	/* Section header table (minimal null section) */
	/* without we cannot test it using spike-isa-sim */
	elf_header->e_shoff = shoff;
	elf_header->e_shentsize = sizeof(Elf32_Shdr);
	elf_header->e_shnum = shnum;
	elf_header->e_shstrndx = shstrndx;
}

static void fill_program_header(Elf32_Phdr *program_header, size_t file_size, uint32_t base_vaddr) {
	memset(program_header, 0, sizeof(*program_header));

	program_header->p_type = PT_LOAD;

	/* Load entire file */
	program_header->p_offset = 0;

	/* Virtual memory base */
	program_header->p_vaddr = base_vaddr;
	program_header->p_paddr = base_vaddr;

	program_header->p_filesz = file_size;
	program_header->p_memsz = file_size;

	/* Read + Write + Execute */
	program_header->p_flags = PF_R | PF_W | PF_X;

	program_header->p_align = 0x1000;
}

static void fill_section_header(Elf32_Shdr *section_header) {
	memset(section_header, 0, sizeof(*section_header));
}

static void fill_shstrtab_section_header(Elf32_Shdr *section_header, size_t offset, size_t size) {
	memset(section_header, 0, sizeof(*section_header));

	section_header->sh_name = 0;
	section_header->sh_type = SHT_STRTAB;
	section_header->sh_offset = offset;
	section_header->sh_size = size;
	section_header->sh_addralign = 1;
}

assembler_error writer32(const char *filename, uint8_t *code, size_t code_len, uint32_t base_vaddr) {
	static const char shstrtab[] = "\0";

	const size_t code_offset = sizeof(Elf32_Ehdr) + sizeof(Elf32_Phdr);
	const size_t code_end = code_offset + code_len;
	const size_t shstrtab_offset = code_end;
	const size_t shstrtab_size = sizeof(shstrtab);
	const size_t shoff = shstrtab_offset + shstrtab_size;
	const size_t shnum = 2;
	const size_t shstrndx = 1;

	// const size_t file_size = shoff + (sizeof(Elf32_Shdr) * shnum);
	// log_msg(LOG_DEBUG, "File size: %ld", file_size);

	Elf32_Ehdr elf_header;
	Elf32_Phdr program_header;
	Elf32_Shdr section_header;
	Elf32_Shdr shstrtab_section_header;

	fill_elf_header(&elf_header, code_offset, shoff, shnum, shstrndx, base_vaddr);
	fill_program_header(&program_header, code_end, base_vaddr);

	fill_section_header(&section_header);
	fill_shstrtab_section_header(&shstrtab_section_header, shstrtab_offset, shstrtab_size);

	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		return ASSEMBLER_ELF_ERROR;
	}

	/* Write elf file */
	fwrite(&elf_header, sizeof(elf_header), 1, fp);
	fwrite(&program_header, sizeof(program_header), 1, fp);
	fwrite(code, code_len, 1, fp);
	fwrite(shstrtab, sizeof(shstrtab), 1, fp);
	fwrite(&section_header, sizeof(section_header), 1, fp);
	fwrite(&shstrtab_section_header, sizeof(shstrtab_section_header), 1, fp);

	fclose(fp);

	return ASSEMBLER_OK;
}
