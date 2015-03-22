#ifndef _ELF_GLOBAL_H_
#define _ELF_GLOBAL_H_

#include <malloc.h>
#include <string.h>
#include "ElfSymbolTable.h"
#include "ElfConstants.h"
#include "ElfHeader.h"
#include "ElfProgHeader.h"
#include "ElfSectionHeader.h"
#include "ElfHacker.h"
#include "ElfDynamicSection.h"
#include "ElfRelocationSection.h"

int ElfHacker_File_fd ;
Elf_Header ElfHacker_elfHeader ;
char* ElfHacker_pSecHeaderStrTable ;
char* ElfHacker_pDymStrTable ;
char* ElfHacker_pSymStrTable ;
Elf32_Phdr* ElfHacker_pELFProgramHeader ;
Elf32_Shdr* ElfHacker_pELFSectionHeader ;
Elf32_Dyn* ElfHacker_pELFDynamicSection ;
Elf32_Rel* ElfHacker_pELFRelocationSection ;
Elf32_Rel* ElfHacker_pELFDynRelocationSection ;
Elf32_Rela* ElfHacker_pELFRelocationSectionAd ;

unsigned ElfHacker_uiSymTabCount ;
ElfHacker_SymbolTableList* ElfHacker_pELFSymbolTable ;
unsigned ElfHacker_uiDynSymTabCount ;
ElfHacker_SymbolTableList* ElfHacker_pELFDynSymbolTable ;
unsigned* ElfHacker_pHashTable ;

unsigned ElfHacker_uiNoOfDynSectionEntries ;
unsigned ElfHacker_uiDynSmyStrTabelSize ;

int exitNumber ;

#define TRACE_LINE printf("\n%s : %d", __FILE__, __LINE__)

#endif
