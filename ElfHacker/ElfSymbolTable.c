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

static void AllocateSymbolTable()
{
	unsigned i ;
	unsigned uiSymTabIndex = 0 ;

	ElfHacker_uiSymTabCount = 0 ;

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_SYMTAB)
			ElfHacker_uiSymTabCount++ ;
	}

	ElfHacker_pELFSymbolTable = (ElfHacker_SymbolTableList*)malloc(ElfHacker_uiSymTabCount * sizeof(ElfHacker_SymbolTableList)) ;

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_SYMTAB)
		{
			ElfHacker_pELFSymbolTable[uiSymTabIndex].uiTableSize = 
						(ElfHacker_pELFSectionHeader[i].sh_size / ElfHacker_pELFSectionHeader[i].sh_entsize) ;

			ElfHacker_pELFSymbolTable[uiSymTabIndex].SymTabEntries = (Elf32_Sym*) malloc (
						sizeof(Elf32_Sym) * ElfHacker_pELFSymbolTable[uiSymTabIndex].uiTableSize ) ;

			uiSymTabIndex++ ;
		}
	}
}

void ElfSymbolTable_DeAllocate()
{
	unsigned i, j ;

	for(i = 0; i < ElfHacker_uiSymTabCount; i++)
		free(ElfHacker_pELFSymbolTable[i].SymTabEntries) ;
	
	free(ElfHacker_pELFSymbolTable) ;
}

/************* Static Functions End ***************/

int ElfSymbolTable_Read()
{
	unsigned i, j, uiSymTabIndex ;

	AllocateSymbolTable() ;

	uiSymTabIndex = 0 ;
	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_SYMTAB)
		{
			if(lseek(ElfHacker_File_fd, ElfHacker_pELFSectionHeader[i].sh_offset, SEEK_SET) < 0)
			{
				perror("LSEEK SYM TABLE") ;
				return -1 ;
			}

			for(j = 0; j < ElfHacker_pELFSymbolTable[uiSymTabIndex].uiTableSize; j++)
			{
				if(read(ElfHacker_File_fd, (char*)&(ElfHacker_pELFSymbolTable[uiSymTabIndex].SymTabEntries[j]),
						sizeof(Elf32_Sym)) < 0)
				{
					perror("READ SYM TABLE") ;
					return -1 ;
				}
			}

			uiSymTabIndex++ ;
		}
	}

	return 0 ;
}

void ElfSymbolTable_Display()
{
	printf("\n************ ELF SYMBOL TABLES START **********************\n") ;

	unsigned i, j ;

	for(i = 0; i < ElfHacker_uiSymTabCount; i++)
	{
		printf("\n\tSymbol Table %u", i) ;

		for(j = 0; j < ElfHacker_pELFSymbolTable[i].uiTableSize; j++)
		{
			printf("\n\t\tSymbol Entry %u", j) ;
		
			printf("\n\t\t\tst_name = %u (%s)", ElfHacker_pELFSymbolTable[i].SymTabEntries[j].st_name,
					ElfHacker_pSymStrTable + ElfHacker_pELFSymbolTable[i].SymTabEntries[j].st_name) ;
			printf("\n\t\t\tst_value = 0x%X (%d)", ElfHacker_pELFSymbolTable[i].SymTabEntries[j].st_value, ElfHacker_pELFSymbolTable[i].SymTabEntries[j].st_value) ;
			printf("\n\t\t\tst_size = %u", ElfHacker_pELFSymbolTable[i].SymTabEntries[j].st_size) ;
			printf("\n\t\t\tst_info = 0x%X (%d)", ElfHacker_pELFSymbolTable[i].SymTabEntries[j].st_info, ElfHacker_pELFSymbolTable[i].SymTabEntries[j].st_info) ;
			printf("\n\t\t\tst_other = %u", ElfHacker_pELFSymbolTable[i].SymTabEntries[j].st_other) ;
			printf("\n\t\t\tst_shndx = %u", ElfHacker_pELFSymbolTable[i].SymTabEntries[j].st_shndx) ;
			
		}
	}
	
	printf("\n************ ELF SYMBOL TABLES END ************************\n") ;
}

