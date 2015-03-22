#ifndef _ELF_HASH_TAB_HEADER_H_
#define _ELF_HASH_TAB_HEADER_H_

unsigned ElfHashTable_uiNoOfBuckets ;
unsigned ElfHashTable_uiNoOfChains ;

void ElfHashTable_DeAllocateHashTable() ;
int ElfHashTable_Read() ;
unsigned ElfHashTable_GetHashValue(const unsigned char* name) ;

#endif
