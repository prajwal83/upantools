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

static void AllocateDynamicSymbolTable()
{
	unsigned i ;
	unsigned uiSymTabIndex = 0 ;

	ElfParser_uiDynSymTabCount = 0 ;

	for(i = 0; i < ElfParser_elfHeader.e_shnum; i++)
	{
		if(ElfParser_pELFSectionHeader[i].sh_type == SHT_DYNSYM)
			ElfParser_uiDynSymTabCount++ ;
	}

	ElfParser_pELFDynSymbolTable = (ElfParser_SymbolTableList*)malloc(ElfParser_uiDynSymTabCount * sizeof(ElfParser_SymbolTableList)) ;

	for(i = 0; i < ElfParser_elfHeader.e_shnum; i++)
	{
		if(ElfParser_pELFSectionHeader[i].sh_type == SHT_DYNSYM)
		{
			ElfParser_pELFDynSymbolTable[uiSymTabIndex].uiTableSize = 
						(ElfParser_pELFSectionHeader[i].sh_size / ElfParser_pELFSectionHeader[i].sh_entsize) ;

			ElfParser_pELFDynSymbolTable[uiSymTabIndex].SymTabEntries = (Elf32_Sym*) malloc (
						sizeof(Elf32_Sym) * ElfParser_pELFDynSymbolTable[uiSymTabIndex].uiTableSize ) ;

			uiSymTabIndex++ ;
		}
	}
}

/************* Static Functions End ***************/

void ElfDynSymbolTable_DeAllocate()
{
	unsigned i, j ;

	for(i = 0; i < ElfParser_uiDynSymTabCount; i++)
		free(ElfParser_pELFDynSymbolTable[i].SymTabEntries) ;
	
	free(ElfParser_pELFDynSymbolTable) ;
}

int ElfDynSymbolTable_Read()
{
	unsigned i, j, uiSymTabIndex ;

	AllocateDynamicSymbolTable() ;

	uiSymTabIndex = 0 ;
	for(i = 0; i < ElfParser_elfHeader.e_shnum; i++)
	{
		if(ElfParser_pELFSectionHeader[i].sh_type == SHT_DYNSYM)
		{
			if(lseek(ElfParser_File_fd, ElfParser_pELFSectionHeader[i].sh_offset, SEEK_SET) < 0)
			{
				perror("LSEEK DYN SYM TABLE") ;
				return -1 ;
			}

			for(j = 0; j < ElfParser_pELFDynSymbolTable[uiSymTabIndex].uiTableSize; j++)
			{
				if(read(ElfParser_File_fd, (char*)&(ElfParser_pELFDynSymbolTable[uiSymTabIndex].SymTabEntries[j]),
						sizeof(Elf32_Sym)) < 0)
				{
					perror("READ DYN SYM TABLE") ;
					return -1 ;
				}
			}

			uiSymTabIndex++ ;
		}
	}

	return 0 ;
}

void ElfDynSymbolTable_Display()
{
	printf("\n************ ELF DYNAMIC SYMBOL TABLES START **********************\n") ;

	unsigned i, j ;

	for(i = 0; i < ElfParser_uiDynSymTabCount; i++)
	{
		printf("\n\tDynamic Symbol Table %u", i) ;

		for(j = 0; j < ElfParser_pELFDynSymbolTable[i].uiTableSize; j++)
		{
			printf("\n\t\tDyn Symbol Entry %u", j) ;
		
			printf("\n\t\t\tst_name = %u (%s)", ElfParser_pELFDynSymbolTable[i].SymTabEntries[j].st_name,
					ElfParser_pDymStrTable + ElfParser_pELFDynSymbolTable[i].SymTabEntries[j].st_name) ;
			printf("\n\t\t\tst_value = 0x%X (%d)", ElfParser_pELFDynSymbolTable[i].SymTabEntries[j].st_value, ElfParser_pELFDynSymbolTable[i].SymTabEntries[j].st_value) ;
			printf("\n\t\t\tst_size = %u", ElfParser_pELFDynSymbolTable[i].SymTabEntries[j].st_size) ;
			printf("\n\t\t\tst_info = 0x%X (%d)", ElfParser_pELFDynSymbolTable[i].SymTabEntries[j].st_info, ElfParser_pELFDynSymbolTable[i].SymTabEntries[j].st_info) ;
			printf("\n\t\t\tst_other = %u", ElfParser_pELFDynSymbolTable[i].SymTabEntries[j].st_other) ;
			printf("\n\t\t\tst_shndx = %u", ElfParser_pELFDynSymbolTable[i].SymTabEntries[j].st_shndx) ;
			
		}
	}
	
	printf("\n************ ELF DUNAMIC SYMBOL TABLES END ************************\n") ;
}

