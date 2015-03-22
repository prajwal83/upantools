#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "ElfConstants.h"
#include "ElfHeader.h"
#include "ElfProgHeader.h"
#include "ElfSectionHeader.h"
#include "ElfHacker.h"
#include "ElfSymbolTable.h"
#include "ElfGlobals.h"

/************* Static Functions Start ***************/

static void ElfHeader_Display_e_ident_Field(const Elf_Header* elfHeader)
{
	printf("\n e_ident:") ;
	printf("\n\t\tEI_MAG0 = 0x%X (%d)", elfHeader->e_ident[EI_MAG0], elfHeader->e_ident[EI_MAG0]) ;
	printf("\n\t\tEI_MAG1 = %c", elfHeader->e_ident[EI_MAG1]) ; 
	printf("\n\t\tEI_MAG2 = %c", elfHeader->e_ident[EI_MAG2]) ;
	printf("\n\t\tEI_MAG3 = %c", elfHeader->e_ident[EI_MAG3]) ;
	printf("\n\t\tEI_CLASS = %s", Elf_Classes[elfHeader->e_ident[EI_CLASS]]) ; 
	printf("\n\t\tEI_DATA = %s", Elf_DataEncoding[elfHeader->e_ident[EI_DATA]]) ;
	printf("\n\t\tEI_VERSION = %s", Elf_Versions[elfHeader->e_ident[EI_VERSION]]) ;
	printf("\n\t\tEI_PAD = %d\n", elfHeader->e_ident[EI_PAD]) ;
}

/************* Static Functions End ***************/

int ElfHeader_Read()
{
	char buf[512] ;

	if(read(ElfHacker_File_fd, &buf, 512) < 0)
	{
		perror("READ ELF HEADER") ;
		return -1 ;
	}

	memcpy(&ElfHacker_elfHeader, &buf, sizeof(Elf_Header)) ;

	return 0 ;
}

void ElfHeader_Display()
{
	printf("\n************ ELF HEADER START **********************\n") ;
	
	ElfHeader_Display_e_ident_Field(&ElfHacker_elfHeader) ;
	printf("\n\te_tpye = %s", Elf_eType[ElfHacker_elfHeader.e_type]) ;
	printf("\n\te_machine = %s", Elf_eMachine[ElfHacker_elfHeader.e_machine]) ;
	printf("\n\te_version = %d", ElfHacker_elfHeader.e_version) ;
	printf("\n\te_entry = 0x%X (%d)", ElfHacker_elfHeader.e_entry, ElfHacker_elfHeader.e_entry) ;
	printf("\n\te_phoff = %d", ElfHacker_elfHeader.e_phoff) ;
	printf("\n\te_shoff = %d", ElfHacker_elfHeader.e_shoff) ;
	printf("\n\te_flags = %d", ElfHacker_elfHeader.e_flags) ;
	printf("\n\te_ehsize = %d", ElfHacker_elfHeader.e_ehsize) ;
	printf("\n\te_phentsize = %d", ElfHacker_elfHeader.e_phentsize) ;
	printf("\n\te_phnum = %d", ElfHacker_elfHeader.e_phnum) ;
	printf("\n\te_shentsize = %d", ElfHacker_elfHeader.e_shentsize) ;
	printf("\n\te_shnum = %d", ElfHacker_elfHeader.e_shnum) ;
	printf("\n\te_shstrndx = %d", ElfHacker_elfHeader.e_shstrndx) ;

	printf("\n************ ELF HEADER END ************************\n") ;
}

