#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "ElfConstants.h"
#include "ElfHeader.h"
#include "ElfProgHeader.h"
#include "ElfSectionHeader.h"
#include "ElfParser.h"
#include "ElfSymbolTable.h"
#include "ElfGlobals.h"

/************* Static Functions Start ***************/

static void AllocateProgramHeader()
{
	ElfParser_pELFProgramHeader = (Elf32_Phdr*)malloc(sizeof(Elf32_Phdr) * ElfParser_elfHeader.e_phnum) ;
}

void ElfProgHeader_DeAllocate()
{
	free(ElfParser_pELFProgramHeader) ;
}

static const char* GetSegAccess(int pflag)
{
	char blank[10] = "-----" ;
	strcpy(seg_access, "") ;
	
	if((pflag & PF_R) == PF_R)
		strcat(seg_access, "Read-") ;
	else
		strcat(seg_access, blank) ;
		
	if((pflag& PF_W) == PF_W)
		strcat(seg_access, "Write-") ;
	else
		strcat(seg_access, blank) ;

	if((pflag & PF_X) == PF_X)
		strcat(seg_access, "Execute") ;
	else
		strcat(seg_access, blank) ;
		
	return seg_access ;
}

/************* Static Functions End ***************/

int ElfProgHeader_Read()
{
	/* e_phentsize if not used as this parser is only for 32 bit elf files.
	   and ElfProgHeader size is 32 bytes */
	   
	AllocateProgramHeader() ;
	
	if(lseek(ElfParser_File_fd, ElfParser_elfHeader.e_phoff, SEEK_SET) < 0)
	{
		perror("LSEEK") ;
		return -1 ;
	}
	
	unsigned i ;
	for(i = 0; i < ElfParser_elfHeader.e_phnum; i++)
	{
		if(read(ElfParser_File_fd, (char*)&ElfParser_pELFProgramHeader[i], sizeof(Elf32_Phdr)) < 0)
		{
			perror("READ ELF PROG_HEADER") ;
			return -1 ;
		}
	}
	
	return 0 ;
}

void ElfProgHeader_Display()
{
	printf("\n************ ELF PROGRAM HEADERS START **********************\n") ;

	if(ElfParser_pELFProgramHeader == NULL)
	{
		printf("\n ELF Program Header is not populated\n") ;
		return ;
	}

	unsigned i ;
	for(i = 0; i < ElfParser_elfHeader.e_phnum; i++)
	{
		printf("\n\nELF Program Header %d", i) ;
		printf("\n\tp_type = %s", GET_PROG_HEADER_TYPE(ElfParser_pELFProgramHeader[i].p_type)) ;
		printf("\n\tp_offset = %d", ElfParser_pELFProgramHeader[i].p_offset) ;
		printf("\n\tp_vaddr = 0x%X (%d)", ElfParser_pELFProgramHeader[i].p_vaddr, ElfParser_pELFProgramHeader[i].p_vaddr) ;
		printf("\n\tp_paddr = 0x%X (%d)", ElfParser_pELFProgramHeader[i].p_paddr, ElfParser_pELFProgramHeader[i].p_paddr) ;
		printf("\n\tp_filesz = %d", ElfParser_pELFProgramHeader[i].p_filesz) ;
		printf("\n\tp_memsz = %d", ElfParser_pELFProgramHeader[i].p_memsz) ;
		printf("\n\tp_flags = %s", GetSegAccess(ElfParser_pELFProgramHeader[i].p_flags)) ;
		printf("\n\tp_align = %d", ElfParser_pELFProgramHeader[i].p_align) ;
	}
	
	printf("\n************ ELF PROGRAM HEADERS END ************************\n") ;
}

