#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "ElfStringTable.h"
#include "ElfGlobals.h"

/************* Static Functions Start ***************/
static void AllocateSecHeaderStrTable(int sec_size, int sec_offset)
{
	ElfHacker_pSecHeaderStrTable = (char*)malloc(sizeof(char) * sec_size) ;
}

static void AllocateDymStrTable() 
{
	unsigned i ;
	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_STRTAB)
		{
			if(strcmp(GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name), ".dynstr") == 0)
			{
				ElfHacker_pDymStrTable = (char*)malloc(sizeof(char) * ElfHacker_pELFSectionHeader[i].sh_size) ;
			}
		}
	}	
}

static void AllocateSymStrTable()
{
	unsigned i ;
	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_STRTAB)
		{
			if(strcmp(GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name), ".strtab") == 0)
			{
				ElfHacker_pSymStrTable = (char*)malloc(sizeof(char) * ElfHacker_pELFSectionHeader[i].sh_size) ;
			}
		}
	}	
}

void DisplayStrTab(unsigned sec_size, const char* szStrTabBuf)
{
	unsigned j ;

	for(j = 0; j < sec_size; j++)
	{
		if(szStrTabBuf[j] == '\0')
			printf("\n\t* - %u -  \t", j + 1) ;
		else
			printf("%c", szStrTabBuf[j]) ;
	}
}
/************* Static Functions End ***************/

void ElfStringTable_DeAllocateSecHeaderStrTable()
{
	free(ElfHacker_pSecHeaderStrTable) ;
}

void ElfStringTable_DeAllocateDymStrTable()
{
	free(ElfHacker_pDymStrTable) ;
}

void ElfStringTable_DeAllocateSymStrTable()
{
	free(ElfHacker_pSymStrTable) ;
}

int ElfStringTable_ReadSecHeaderStrTable()
{
	int sec_size = ElfHacker_pELFSectionHeader[ElfHacker_elfHeader.e_shstrndx].sh_size ;
	int sec_offset = ElfHacker_pELFSectionHeader[ElfHacker_elfHeader.e_shstrndx].sh_offset ;

	AllocateSecHeaderStrTable(sec_size, sec_offset) ;

	if(lseek(ElfHacker_File_fd, sec_offset, SEEK_SET) < 0)
	{
		perror("LSEEK") ;
		return -1 ;
	}
	
	if(read(ElfHacker_File_fd, (char*)ElfHacker_pSecHeaderStrTable, sec_size) < 0)
	{
		perror("READ ELF SEC HEADER STR TABLE") ;
		return -1 ;
	}

	return 0 ;
}

int ElfStringTable_ReadSymStrTable()
{
	char strbuf[4096] ;
	unsigned i, j ;
	unsigned strTableNumber = 0 ;

	AllocateSymStrTable() ;

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_STRTAB)
		{
			if(strcmp(GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name), ".strtab") == 0)
			{
				printf("\n Symbol String Table:") ;
				printf("\n\tSection Header Index = %d", i) ;
				printf("\n\tSection Name = %s\n", GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name)) ;

				if(lseek(ElfHacker_File_fd, ElfHacker_pELFSectionHeader[i].sh_offset, SEEK_SET) < 0)
				{
					perror("LSEEK") ;
					return -1 ;
				}
				
				if(read(ElfHacker_File_fd, (char*)ElfHacker_pSymStrTable, ElfHacker_pELFSectionHeader[i].sh_size) < 0)
				{
					perror("READ ELF STR TABLE") ;
					return -1 ;
				}

				return 0 ;
			}
		}
	}

	return 0 ;
}

int ElfStringTable_ReadDymStrTable()
{
	char strbuf[4096] ;
	unsigned i, j ;
	unsigned strTableNumber = 0 ;

	AllocateDymStrTable() ;

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_STRTAB)
		{
			if(strcmp(GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name), ".dynstr") == 0)
			{
				if(lseek(ElfHacker_File_fd, ElfHacker_pELFSectionHeader[i].sh_offset, SEEK_SET) < 0)
				{
					perror("LSEEK") ;
					return -1 ;
				}
				
				ElfHacker_uiDynSmyStrTabelSize = ElfHacker_pELFSectionHeader[i].sh_size ;
				if(read(ElfHacker_File_fd, (char*)ElfHacker_pDymStrTable, ElfHacker_uiDynSmyStrTabelSize) < 0)
				{
					perror("READ ELF DYM STR TABLE") ;
					return -1 ;
				}

				return 0 ;
			}
		}
	}

	return 0 ;
}

void ElfStringTable_DisplaySecHeaderStrTable()
{
	int sec_size = ElfHacker_pELFSectionHeader[ElfHacker_elfHeader.e_shstrndx].sh_size ;
	
	printf("\n************ ELF SEC HEADER STR TABLE START **********************\n") ;
	DisplayStrTab(sec_size, ElfHacker_pSecHeaderStrTable) ;
	printf("\n************ ELF SEC HEADER STR TABLE END **********************\n") ;
}	

void ElfStringTable_DisplayDymStrTable()
{
	unsigned i ;
	unsigned sec_size ;

	printf("\n************ ELF DYM STR TABLE START **********************\n") ;
	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_STRTAB)
		{
			if(strcmp(GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name), ".dynstr") == 0)
			{
				printf("\n Dynamic Symbol String Table:") ;
				printf("\n\tSection Header Index = %d", i) ;
				printf("\n\tSection Name = %s\n", GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name)) ;

				sec_size = ElfHacker_pELFSectionHeader[i].sh_size ;
				break ;
			}
		}
	}

	DisplayStrTab(sec_size, ElfHacker_pDymStrTable) ;
	printf("\n************ ELF DYM STR TABLE END **********************\n") ;
}

void ElfStringTable_DisplaySymStrTable()
{
	unsigned i ;
	unsigned sec_size ;

	printf("\n************ ELF SYM STR TABLE START **********************\n") ;
	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_STRTAB)
		{
			if(strcmp(GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name), ".strtab") == 0)
			{
				printf("\n Symbol String Table:") ;
				printf("\n\tSection Header Index = %d", i) ;
				printf("\n\tSection Name = %s\n", GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name)) ;

				sec_size = ElfHacker_pELFSectionHeader[i].sh_size ;
				break ;
			}
		}
	}

	DisplayStrTab(sec_size, ElfHacker_pSymStrTable) ;
	printf("\n************ ELF SYM STR TABLE END **********************\n") ;
}
