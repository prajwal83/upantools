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

static void AllocateSectionHeader()
{
	ElfHacker_pELFSectionHeader = (Elf32_Shdr*)malloc(sizeof(Elf32_Shdr) * ElfHacker_elfHeader.e_shnum) ;
}

void ElfSectionHeader_DeAllocate()
{
	free(ElfHacker_pELFSectionHeader) ;
}

static const char* GetSectionFlags(int sflag)
{
	char blank[10] = "-----" ;
	strcpy(sec_flags, "") ;

	if(sflag > (SHF_EXECINSTR | SHF_ALLOC | SHF_WRITE) || sflag < 1)
	{
		strcat(sec_flags, "Processor Specific") ;
		return sec_flags ;
	}

	if((sflag & SHF_WRITE) == SHF_WRITE)
		strcat(sec_flags, "Write-") ;
	else
		strcat(sec_flags, blank) ;
		
	if((sflag & SHF_ALLOC) == SHF_ALLOC)
		strcat(sec_flags, "Alloc-") ;
	else
		strcat(sec_flags, blank) ;

	if((sflag & SHF_EXECINSTR) == SHF_EXECINSTR)
		strcat(sec_flags, "Execute") ;
	else
		strcat(sec_flags, blank) ;
		
	return sec_flags ;
}


/************* Static Functions End ***************/

int ElfSectionHeader_Read()
{
	/* e_shentsize if not used as this hacker is only for 32 bit elf files.
	   and ElfSectionHeader size is 40 bytes */
	   
	AllocateSectionHeader() ;
	
	if(lseek(ElfHacker_File_fd, ElfHacker_elfHeader.e_shoff, SEEK_SET) < 0)
	{
		perror("LSEEK") ;
		return -1 ;
	}
	
	unsigned i ;
	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(read(ElfHacker_File_fd, (char*)&ElfHacker_pELFSectionHeader[i], sizeof(Elf32_Shdr)) < 0)
		{
			perror("READ ELF SEC_HEADER") ;
			return -1 ;
		}
	}
	
	return 0 ;
}

void ElfSectionHeader_Display()
{
	printf("\n************ ELF SECTION HEADERS START **********************\n") ;

	if(ElfHacker_pELFSectionHeader == NULL)
	{
		printf("\n ELF Section Header is not populated\n") ;
		return ;
	}

	unsigned i ;
	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		printf("\n\nELF Section Header %d", i) ;
		printf("\n\tsh_name = %s", GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name)) ;
		printf("\n\tsh_type = %s", GET_SEC_HEADER_TYPE(ElfHacker_pELFSectionHeader[i].sh_type)) ;
		printf("\n\tsh_flags = %s", GetSectionFlags(ElfHacker_pELFSectionHeader[i].sh_flags)) ;
		printf("\n\tsh_addr = 0x%X (%d)", ElfHacker_pELFSectionHeader[i].sh_addr, ElfHacker_pELFSectionHeader[i].sh_addr) ;
		printf("\n\tsh_offset = %d", ElfHacker_pELFSectionHeader[i].sh_offset) ;
		printf("\n\tsh_size = %d", ElfHacker_pELFSectionHeader[i].sh_size) ;
		printf("\n\tsh_link = %d", ElfHacker_pELFSectionHeader[i].sh_link) ;
		printf("\n\tsh_info = %d", ElfHacker_pELFSectionHeader[i].sh_info) ;
		printf("\n\tsh_addralign = %d", ElfHacker_pELFSectionHeader[i].sh_addralign) ;
		printf("\n\tsh_entsize = %d", ElfHacker_pELFSectionHeader[i].sh_entsize) ;
	}
	
	printf("\n************ ELF SECTION HEADERS END ************************\n") ;
}

