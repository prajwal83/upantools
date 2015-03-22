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

static void AllocateDynamicSymbolTable()
{
	unsigned i ;
	unsigned uiSymTabIndex = 0 ;

	ElfHacker_uiDynSymTabCount = 0 ;

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_DYNSYM)
			ElfHacker_uiDynSymTabCount++ ;
	}

	ElfHacker_pELFDynSymbolTable = (ElfHacker_SymbolTableList*)malloc(ElfHacker_uiDynSymTabCount * sizeof(ElfHacker_SymbolTableList)) ;

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_DYNSYM)
		{
			ElfHacker_pELFDynSymbolTable[uiSymTabIndex].uiTableSize = 
						(ElfHacker_pELFSectionHeader[i].sh_size / ElfHacker_pELFSectionHeader[i].sh_entsize) ;

			ElfHacker_pELFDynSymbolTable[uiSymTabIndex].SymTabEntries = (Elf32_Sym*) malloc (
						sizeof(Elf32_Sym) * ElfHacker_pELFDynSymbolTable[uiSymTabIndex].uiTableSize ) ;

			uiSymTabIndex++ ;
		}
	}
}

/************* Static Functions End ***************/

void ElfDynSymbolTable_DeAllocate()
{
	unsigned i, j ;

	for(i = 0; i < ElfHacker_uiDynSymTabCount; i++)
		free(ElfHacker_pELFDynSymbolTable[i].SymTabEntries) ;
	
	free(ElfHacker_pELFDynSymbolTable) ;
}

int ElfDynSymbolTable_Read()
{
	unsigned i, j, uiSymTabIndex ;

	AllocateDynamicSymbolTable() ;

	uiSymTabIndex = 0 ;
	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_DYNSYM)
		{
			if(lseek(ElfHacker_File_fd, ElfHacker_pELFSectionHeader[i].sh_offset, SEEK_SET) < 0)
			{
				perror("LSEEK DYN SYM TABLE") ;
				return -1 ;
			}

			for(j = 0; j < ElfHacker_pELFDynSymbolTable[uiSymTabIndex].uiTableSize; j++)
			{
				if(read(ElfHacker_File_fd, (char*)&(ElfHacker_pELFDynSymbolTable[uiSymTabIndex].SymTabEntries[j]),
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

	for(i = 0; i < ElfHacker_uiDynSymTabCount; i++)
	{
		printf("\n\tDynamic Symbol Table %u", i) ;

		for(j = 0; j < ElfHacker_pELFDynSymbolTable[i].uiTableSize; j++)
		{
			printf("\n\t\tDyn Symbol Entry %u", j) ;
		
			printf("\n\t\t\tst_name = %u (%s)", ElfHacker_pELFDynSymbolTable[i].SymTabEntries[j].st_name,
					ElfHacker_pDymStrTable + ElfHacker_pELFDynSymbolTable[i].SymTabEntries[j].st_name) ;
			printf("\n\t\t\tst_value = 0x%X (%d)", ElfHacker_pELFDynSymbolTable[i].SymTabEntries[j].st_value, ElfHacker_pELFDynSymbolTable[i].SymTabEntries[j].st_value) ;
			printf("\n\t\t\tst_size = %u", ElfHacker_pELFDynSymbolTable[i].SymTabEntries[j].st_size) ;
			printf("\n\t\t\tst_info = 0x%X (%d)", ElfHacker_pELFDynSymbolTable[i].SymTabEntries[j].st_info, ElfHacker_pELFDynSymbolTable[i].SymTabEntries[j].st_info) ;
			printf("\n\t\t\tst_other = %u", ElfHacker_pELFDynSymbolTable[i].SymTabEntries[j].st_other) ;
			printf("\n\t\t\tst_shndx = %u", ElfHacker_pELFDynSymbolTable[i].SymTabEntries[j].st_shndx) ;
			
		}
	}
	
	printf("\n************ ELF DUNAMIC SYMBOL TABLES END ************************\n") ;
}

