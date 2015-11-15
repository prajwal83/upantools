#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "ElfStringTable.h"
#include "ElfGlobals.h"

/************* Static Functions Start ***************/
static void AllocateSecHeaderStrTable(int sec_size, int sec_offset)
{
	ElfParser_pSecHeaderStrTable = (char*)malloc(sizeof(char) * sec_size) ;
}

static void AllocateDymStrTable() 
{
	unsigned i ;
	for(i = 0; i < ElfParser_elfHeader.e_shnum; i++)
	{
		if(ElfParser_pELFSectionHeader[i].sh_type == SHT_STRTAB)
		{
			if(strcmp(GET_SECTION_NAME(ElfParser_pELFSectionHeader[i].sh_name), ".dynstr") == 0)
			{
				ElfParser_pDymStrTable = (char*)malloc(sizeof(char) * ElfParser_pELFSectionHeader[i].sh_size) ;
			}
		}
	}	
}

static void AllocateSymStrTable()
{
	unsigned i ;
	for(i = 0; i < ElfParser_elfHeader.e_shnum; i++)
	{
		if(ElfParser_pELFSectionHeader[i].sh_type == SHT_STRTAB)
		{
			if(strcmp(GET_SECTION_NAME(ElfParser_pELFSectionHeader[i].sh_name), ".strtab") == 0)
			{
				ElfParser_pSymStrTable = (char*)malloc(sizeof(char) * ElfParser_pELFSectionHeader[i].sh_size) ;
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
	free(ElfParser_pSecHeaderStrTable) ;
}

void ElfStringTable_DeAllocateDymStrTable()
{
	free(ElfParser_pDymStrTable) ;
}

void ElfStringTable_DeAllocateSymStrTable()
{
	free(ElfParser_pSymStrTable) ;
}

int ElfStringTable_ReadSecHeaderStrTable()
{
	int sec_size = ElfParser_pELFSectionHeader[ElfParser_elfHeader.e_shstrndx].sh_size ;
	int sec_offset = ElfParser_pELFSectionHeader[ElfParser_elfHeader.e_shstrndx].sh_offset ;

	AllocateSecHeaderStrTable(sec_size, sec_offset) ;

	if(lseek(ElfParser_File_fd, sec_offset, SEEK_SET) < 0)
	{
		perror("LSEEK") ;
		return -1 ;
	}
	
	if(read(ElfParser_File_fd, (char*)ElfParser_pSecHeaderStrTable, sec_size) < 0)
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

	for(i = 0; i < ElfParser_elfHeader.e_shnum; i++)
	{
		if(ElfParser_pELFSectionHeader[i].sh_type == SHT_STRTAB)
		{
			if(strcmp(GET_SECTION_NAME(ElfParser_pELFSectionHeader[i].sh_name), ".strtab") == 0)
			{
				printf("\n Symbol String Table:") ;
				printf("\n\tSection Header Index = %d", i) ;
				printf("\n\tSection Name = %s\n", GET_SECTION_NAME(ElfParser_pELFSectionHeader[i].sh_name)) ;

				if(lseek(ElfParser_File_fd, ElfParser_pELFSectionHeader[i].sh_offset, SEEK_SET) < 0)
				{
					perror("LSEEK") ;
					return -1 ;
				}
				
				if(read(ElfParser_File_fd, (char*)ElfParser_pSymStrTable, ElfParser_pELFSectionHeader[i].sh_size) < 0)
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

	for(i = 0; i < ElfParser_elfHeader.e_shnum; i++)
	{
		if(ElfParser_pELFSectionHeader[i].sh_type == SHT_STRTAB)
		{
			if(strcmp(GET_SECTION_NAME(ElfParser_pELFSectionHeader[i].sh_name), ".dynstr") == 0)
			{
				if(lseek(ElfParser_File_fd, ElfParser_pELFSectionHeader[i].sh_offset, SEEK_SET) < 0)
				{
					perror("LSEEK") ;
					return -1 ;
				}
				
				ElfParser_uiDynSmyStrTabelSize = ElfParser_pELFSectionHeader[i].sh_size ;
				if(read(ElfParser_File_fd, (char*)ElfParser_pDymStrTable, ElfParser_uiDynSmyStrTabelSize) < 0)
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
	int sec_size = ElfParser_pELFSectionHeader[ElfParser_elfHeader.e_shstrndx].sh_size ;
	
	printf("\n************ ELF SEC HEADER STR TABLE START **********************\n") ;
	DisplayStrTab(sec_size, ElfParser_pSecHeaderStrTable) ;
	printf("\n************ ELF SEC HEADER STR TABLE END **********************\n") ;
}	

void ElfStringTable_DisplayDymStrTable()
{
	unsigned i ;
	unsigned sec_size ;

	printf("\n************ ELF DYM STR TABLE START **********************\n") ;
	for(i = 0; i < ElfParser_elfHeader.e_shnum; i++)
	{
		if(ElfParser_pELFSectionHeader[i].sh_type == SHT_STRTAB)
		{
			if(strcmp(GET_SECTION_NAME(ElfParser_pELFSectionHeader[i].sh_name), ".dynstr") == 0)
			{
				printf("\n Dynamic Symbol String Table:") ;
				printf("\n\tSection Header Index = %d", i) ;
				printf("\n\tSection Name = %s\n", GET_SECTION_NAME(ElfParser_pELFSectionHeader[i].sh_name)) ;

				sec_size = ElfParser_pELFSectionHeader[i].sh_size ;
				break ;
			}
		}
	}

	DisplayStrTab(sec_size, ElfParser_pDymStrTable) ;
	printf("\n************ ELF DYM STR TABLE END **********************\n") ;
}

void ElfStringTable_DisplaySymStrTable()
{
	unsigned i ;
	unsigned sec_size ;

	printf("\n************ ELF SYM STR TABLE START **********************\n") ;
	for(i = 0; i < ElfParser_elfHeader.e_shnum; i++)
	{
		if(ElfParser_pELFSectionHeader[i].sh_type == SHT_STRTAB)
		{
			if(strcmp(GET_SECTION_NAME(ElfParser_pELFSectionHeader[i].sh_name), ".strtab") == 0)
			{
				printf("\n Symbol String Table:") ;
				printf("\n\tSection Header Index = %d", i) ;
				printf("\n\tSection Name = %s\n", GET_SECTION_NAME(ElfParser_pELFSectionHeader[i].sh_name)) ;

				sec_size = ElfParser_pELFSectionHeader[i].sh_size ;
				break ;
			}
		}
	}

	DisplayStrTab(sec_size, ElfParser_pSymStrTable) ;
	printf("\n************ ELF SYM STR TABLE END **********************\n") ;
}
