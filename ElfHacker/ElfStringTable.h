#ifndef _ELF_STR_TAB_H_
#define _ELF_STR_TAB_H_

void ElfStringTable_DeAllocateSecHeaderStrTable() ;
void ElfStringTable_DeAllocateDymStrTable() ;
void ElfStringTable_DeAllocateSymStrTable() ;

int ElfStringTable_ReadSecHeaderStrTable() ;
int ElfStringTable_ReadDymStrTable() ;
int ElfStringTable_ReadSymStrTable() ;

void ElfStringTable_DisplaySecHeaderStrTable() ;
void ElfStringTable_DisplayDymStrTable() ;
void ElfStringTable_DisplaySymStrTable() ;

#endif
