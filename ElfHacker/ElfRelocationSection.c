#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "ElfRelocationSection.h"
#include "ElfConstants.h"
#include "ElfGlobals.h"
#include "ElfHashTable.h"

/************* Static Functions Start ***************/

static void AllocateDynRelocationSection()
{
	unsigned i ;

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_REL)
		{
			if(strstr(GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name), ".dyn") == NULL)
				continue ;

			ElfHacker_pELFDynRelocationSection = (Elf32_Rel*)malloc(
			(ElfHacker_pELFSectionHeader[i].sh_size / ElfHacker_pELFSectionHeader[i].sh_entsize) * sizeof(Elf32_Rel)) ;
			break ;
		}
	}
}

static void AllocateRelocationSectionWithOutAddends()
{
	unsigned i ;

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_REL)
		{
			if(strstr(GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name), ".plt") == NULL)
				if(strstr(GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name), ".rel.text") == NULL)
					continue ;

			ElfHacker_pELFRelocationSection = (Elf32_Rel*)malloc(
			(ElfHacker_pELFSectionHeader[i].sh_size / ElfHacker_pELFSectionHeader[i].sh_entsize) * sizeof(Elf32_Rel)) ;
			break ;
		}
	}
}

static void AllocateRelocationSectionWithAddends()
{
	unsigned i ;

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_RELA)
		{
			ElfHacker_pELFRelocationSectionAd = (Elf32_Rela*)malloc(
			(ElfHacker_pELFSectionHeader[i].sh_size / ElfHacker_pELFSectionHeader[i].sh_entsize) * sizeof(Elf32_Rela)) ;
			break ;
		}
	}
}

/************* Static Functions End ***************/

void ElfRelocationSection_DeAllocateDyn()
{
	if(ElfHacker_pELFDynRelocationSection)
		free(ElfHacker_pELFDynRelocationSection) ;
}

void ElfRelocationSection_DeAllocateWithOutAddends()
{
	free(ElfHacker_pELFRelocationSection) ;
}

void ElfRelocationSection_DeAllocateWithAddends()
{
	free(ElfHacker_pELFRelocationSectionAd) ;
}

int ElfDynRelocationSection_Read()
{
	unsigned i ;

	AllocateDynRelocationSection() ;

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_REL)
		{
			if(strstr(GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name), ".dyn") == NULL)
				continue ;

			if(lseek(ElfHacker_File_fd, ElfHacker_pELFSectionHeader[i].sh_offset, SEEK_SET) < 0)
			{
				perror("LSEEK REL TABLE") ;
				return -1 ;
			}

			if(read(ElfHacker_File_fd, (char*)(ElfHacker_pELFDynRelocationSection),
				(ElfHacker_pELFSectionHeader[i].sh_size / ElfHacker_pELFSectionHeader[i].sh_entsize) 
					* sizeof(Elf32_Rel)) < 0)
			{
				perror("READ REL TABLE") ;
				return -1 ;
			}
			break ;
		}
	}

	return 0 ;
}

int ElfRelocationSection_Read()
{
	unsigned i ;

	AllocateRelocationSectionWithOutAddends() ;

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_REL)
		{
			if(strstr(GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name), ".plt") == NULL)
				if(strstr(GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name), ".rel.text") == NULL)
					continue ;

			if(lseek(ElfHacker_File_fd, ElfHacker_pELFSectionHeader[i].sh_offset, SEEK_SET) < 0)
			{
				perror("LSEEK REL TABLE") ;
				return -1 ;
			}

			if(read(ElfHacker_File_fd, (char*)(ElfHacker_pELFRelocationSection),
				(ElfHacker_pELFSectionHeader[i].sh_size / ElfHacker_pELFSectionHeader[i].sh_entsize) 
					* sizeof(Elf32_Rel)) < 0)
			{
				perror("READ REL TABLE") ;
				return -1 ;
			}
			break ;
		}
	}

	return 0 ;
}

int ElfRelocationSection_ReadAd()
{
	unsigned i ;

	AllocateRelocationSectionWithAddends() ;

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_RELA)
		{
			if(lseek(ElfHacker_File_fd, ElfHacker_pELFSectionHeader[i].sh_offset, SEEK_SET) < 0)
			{
				perror("LSEEK RELA TABLE") ;
				return -1 ;
			}

			if(read(ElfHacker_File_fd, (char*)(ElfHacker_pELFRelocationSectionAd),
				(ElfHacker_pELFSectionHeader[i].sh_size / ElfHacker_pELFSectionHeader[i].sh_entsize) 
					* sizeof(Elf32_Rela)) < 0)
			{
				perror("READ RELA TABLE") ;
				return -1 ;
			}
			break ;
		}
	}

	return 0 ;
}

void ElfRelocationSection_DisplayDyn()
{
	printf("\n************ ELF Dynamic RELOCATION SECTION WITH OUT ADDEND START **********************\n") ;

	unsigned i ;

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_REL)
		{
			if(strstr(GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name), ".dyn") == NULL)
				continue ;

			printf("\n Dynamic Relocation Section - %s", GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name)) ;

			unsigned uiNoOfEntries = ElfHacker_pELFSectionHeader[i].sh_size / ElfHacker_pELFSectionHeader[i].sh_entsize ;
			unsigned j ;
			for(j = 0; j < uiNoOfEntries; j++)
			{
				printf("\n Dynamic Relocation Entry %d:", j) ;
				printf("\t\t Rel Offset = 0x%X (%u)", ElfHacker_pELFDynRelocationSection[j].r_offset,
										ElfHacker_pELFDynRelocationSection[j].r_offset) ;
				printf("\t\t Sym Index = %u", ELF32_R_SYM(ElfHacker_pELFDynRelocationSection[j].r_info)) ;
				printf("\t\t Sym Index = %u", (ElfHacker_pELFDynRelocationSection[j].r_info)) ;
				printf("\t\t Rel Type = %s", RelocationType[ELF32_R_TYPE(ElfHacker_pELFDynRelocationSection[j].r_info)]) ;
			}
			break ;
		}
	}

	printf("\n************ ELF RELOCATION SECTION WITH OUT ADDEND END **********************\n") ;
}

void ElfRelocationSection_DisplayWithOutAddends()
{
	printf("\n************ ELF RELOCATION SECTION WITH OUT ADDEND START **********************\n") ;

	unsigned i ;

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_REL)
		{
			if(strstr(GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name), ".plt") == NULL)
				if(strstr(GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name), ".rel.text") == NULL)
					continue ;

			printf("\n Relocation Section - %s", GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name)) ;

			unsigned uiNoOfEntries = ElfHacker_pELFSectionHeader[i].sh_size / ElfHacker_pELFSectionHeader[i].sh_entsize ;
			unsigned j ;
			for(j = 0; j < uiNoOfEntries; j++)
			{
				printf("\n Relocation Entry %d:", j) ;
				printf("\t\t Rel Offset = 0x%X (%u)", ElfHacker_pELFRelocationSection[j].r_offset,
										ElfHacker_pELFRelocationSection[j].r_offset) ;
				printf("\t\t Sym Index = %u", ELF32_R_SYM(ElfHacker_pELFRelocationSection[j].r_info)) ;
				printf("\t\t Sym Index = %u", (ElfHacker_pELFRelocationSection[j].r_info)) ;
				printf("\t\t Rel Type = %s", RelocationType[ELF32_R_TYPE(ElfHacker_pELFRelocationSection[j].r_info)]) ;
			}
			break ;
		}
	}

	printf("\n************ ELF RELOCATION SECTION WITH OUT ADDEND END **********************\n") ;
}

void ElfRelocationSection_DisplayWithAddends()
{
	printf("\n************ ELF RELOCATION SECTION WITH ADDEND START **********************\n") ;

	unsigned i ;
	unsigned uiRelIndex = 0 ;

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_RELA)
		{
			printf("\n Relocation Entry %d:", uiRelIndex) ;
			printf("\t\t Rel Offset = 0x%X (%u)", ElfHacker_pELFRelocationSectionAd[uiRelIndex].r_offset,
										ElfHacker_pELFRelocationSectionAd[uiRelIndex].r_offset) ;
			printf("\t\t Sym Index = %u", ELF32_R_SYM(ElfHacker_pELFRelocationSectionAd[uiRelIndex].r_info)) ;
			printf("\t\t Rel Type = %s", RelocationType[ELF32_R_TYPE(ElfHacker_pELFRelocationSectionAd[uiRelIndex].r_info)]) ;
			printf("\t\t Rel Addend = %u", ElfHacker_pELFRelocationSectionAd[uiRelIndex].r_addend) ;

			uiRelIndex++ ;
		}
	}

	printf("\n************ ELF RELOCATION SECTION WITH ADDEND END **********************\n") ;
}

