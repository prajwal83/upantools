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

static void AllocateSymbolTable()
{
	unsigned i ;
	unsigned uiSymTabIndex = 0 ;

	ElfParser_uiSymTabCount = 0 ;

	for(i = 0; i < ElfParser_elfHeader.e_shnum; i++)
	{
		if(ElfParser_pELFSectionHeader[i].sh_type == SHT_SYMTAB)
			ElfParser_uiSymTabCount++ ;
	}

	ElfParser_pELFSymbolTable = (ElfParser_SymbolTableList*)malloc(ElfParser_uiSymTabCount * sizeof(ElfParser_SymbolTableList)) ;

	for(i = 0; i < ElfParser_elfHeader.e_shnum; i++)
	{
		if(ElfParser_pELFSectionHeader[i].sh_type == SHT_SYMTAB)
		{
			ElfParser_pELFSymbolTable[uiSymTabIndex].uiTableSize = 
						(ElfParser_pELFSectionHeader[i].sh_size / ElfParser_pELFSectionHeader[i].sh_entsize) ;

			ElfParser_pELFSymbolTable[uiSymTabIndex].SymTabEntries = (Elf32_Sym*) malloc (
						sizeof(Elf32_Sym) * ElfParser_pELFSymbolTable[uiSymTabIndex].uiTableSize ) ;

			uiSymTabIndex++ ;
		}
	}
}

void ElfSymbolTable_DeAllocate()
{
	unsigned i, j ;

	for(i = 0; i < ElfParser_uiSymTabCount; i++)
		free(ElfParser_pELFSymbolTable[i].SymTabEntries) ;
	
	free(ElfParser_pELFSymbolTable) ;
}

/************* Static Functions End ***************/

int ElfSymbolTable_Read()
{
	unsigned i, j, uiSymTabIndex ;

	AllocateSymbolTable() ;

	uiSymTabIndex = 0 ;
	for(i = 0; i < ElfParser_elfHeader.e_shnum; i++)
	{
		if(ElfParser_pELFSectionHeader[i].sh_type == SHT_SYMTAB)
		{
			if(lseek(ElfParser_File_fd, ElfParser_pELFSectionHeader[i].sh_offset, SEEK_SET) < 0)
			{
				perror("LSEEK SYM TABLE") ;
				return -1 ;
			}

			for(j = 0; j < ElfParser_pELFSymbolTable[uiSymTabIndex].uiTableSize; j++)
			{
				if(read(ElfParser_File_fd, (char*)&(ElfParser_pELFSymbolTable[uiSymTabIndex].SymTabEntries[j]),
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

	for(i = 0; i < ElfParser_uiSymTabCount; i++)
	{
		printf("\n\tSymbol Table %u", i) ;

		for(j = 0; j < ElfParser_pELFSymbolTable[i].uiTableSize; j++)
		{
			printf("\n\t\tSymbol Entry %u", j) ;
		
			printf("\n\t\t\tst_name = %u (%s)", ElfParser_pELFSymbolTable[i].SymTabEntries[j].st_name,
					ElfParser_pSymStrTable + ElfParser_pELFSymbolTable[i].SymTabEntries[j].st_name) ;
			printf("\n\t\t\tst_value = 0x%X (%d)", ElfParser_pELFSymbolTable[i].SymTabEntries[j].st_value, ElfParser_pELFSymbolTable[i].SymTabEntries[j].st_value) ;
			printf("\n\t\t\tst_size = %u", ElfParser_pELFSymbolTable[i].SymTabEntries[j].st_size) ;
			printf("\n\t\t\tst_info = 0x%X (%d)", ElfParser_pELFSymbolTable[i].SymTabEntries[j].st_info, ElfParser_pELFSymbolTable[i].SymTabEntries[j].st_info) ;
			printf("\n\t\t\tst_other = %u", ElfParser_pELFSymbolTable[i].SymTabEntries[j].st_other) ;
			printf("\n\t\t\tst_shndx = %u", ElfParser_pELFSymbolTable[i].SymTabEntries[j].st_shndx) ;
			
		}
	}
	
	printf("\n************ ELF SYMBOL TABLES END ************************\n") ;
}

