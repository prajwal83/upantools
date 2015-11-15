#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "ElfHashTable.h"
#include "ElfGlobals.h"

/***************** Static functions Start *******************/
static void AllocateHashTable()
{
	unsigned i ;
	for(i = 0; i < ElfParser_elfHeader.e_shnum; i++)
	{
		if(ElfParser_pELFSectionHeader[i].sh_type == SHT_HASH)
		{
			ElfParser_pHashTable = (unsigned*)malloc(sizeof(char) * ElfParser_pELFSectionHeader[i].sh_size) ;
			return ;
		}
	}
}
/***************** Static functions End *******************/

void ElfHashTable_DeAllocateHashTable()
{
	free(ElfParser_pHashTable) ;
}

int ElfHashTable_Read()
{
	unsigned i ;
	
	AllocateHashTable() ;

	for(i = 0; i < ElfParser_elfHeader.e_shnum; i++)
	{
		if(ElfParser_pELFSectionHeader[i].sh_type == SHT_HASH)
		{
			if(lseek(ElfParser_File_fd, ElfParser_pELFSectionHeader[i].sh_offset, SEEK_SET) < 0)
			{
				perror("LSEEK HASH TABLE") ;
				return -1 ;
			}

			if(read(ElfParser_File_fd, (char*)(ElfParser_pHashTable), ElfParser_pELFSectionHeader[i].sh_size) < 0)
			{
				perror("READ HASH TABLE") ;
				return -1 ;
			}
			ElfHashTable_uiNoOfBuckets = ElfParser_pHashTable[0] ;
			ElfHashTable_uiNoOfChains = ElfParser_pHashTable[1] ;
			break ;
		}
	}

	return 0 ;
}

unsigned ElfHashTable_GetHashValue(const unsigned char* name)
{
	unsigned h = 0, g ;

	while(*name)
	{
		h = (h << 4) + *name++ ;

		if(g = h & 0xf0000000)
			h ^= g >> 24 ;

		h &= ~g ;
	}

	return h ;
}

