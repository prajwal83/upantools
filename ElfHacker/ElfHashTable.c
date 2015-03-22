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
	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_HASH)
		{
			ElfHacker_pHashTable = (unsigned*)malloc(sizeof(char) * ElfHacker_pELFSectionHeader[i].sh_size) ;
			return ;
		}
	}
}
/***************** Static functions End *******************/

void ElfHashTable_DeAllocateHashTable()
{
	free(ElfHacker_pHashTable) ;
}

int ElfHashTable_Read()
{
	unsigned i ;
	
	AllocateHashTable() ;

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_HASH)
		{
			if(lseek(ElfHacker_File_fd, ElfHacker_pELFSectionHeader[i].sh_offset, SEEK_SET) < 0)
			{
				perror("LSEEK HASH TABLE") ;
				return -1 ;
			}

			if(read(ElfHacker_File_fd, (char*)(ElfHacker_pHashTable), ElfHacker_pELFSectionHeader[i].sh_size) < 0)
			{
				perror("READ HASH TABLE") ;
				return -1 ;
			}
			ElfHashTable_uiNoOfBuckets = ElfHacker_pHashTable[0] ;
			ElfHashTable_uiNoOfChains = ElfHacker_pHashTable[1] ;
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

