#ifndef _ELF_GLOBAL_H_
#define _ELF_GLOBAL_H_

#include <malloc.h>
#include <string.h>
#include "ElfSymbolTable.h"
#include "ElfConstants.h"
#include "ElfHeader.h"
#include "ElfProgHeader.h"
#include "ElfSectionHeader.h"
#include "ElfParser.h"
#include "ElfDynamicSection.h"
#include "ElfRelocationSection.h"

int ElfParser_File_fd ;
Elf_Header ElfParser_elfHeader ;
char* ElfParser_pSecHeaderStrTable ;
char* ElfParser_pDymStrTable ;
char* ElfParser_pSymStrTable ;
Elf32_Phdr* ElfParser_pELFProgramHeader ;
Elf32_Shdr* ElfParser_pELFSectionHeader ;
Elf32_Dyn* ElfParser_pELFDynamicSection ;
Elf32_Rel* ElfParser_pELFRelocationSection ;
Elf32_Rel* ElfParser_pELFDynRelocationSection ;
Elf32_Rela* ElfParser_pELFRelocationSectionAd ;

unsigned ElfParser_uiSymTabCount ;
ElfParser_SymbolTableList* ElfParser_pELFSymbolTable ;
unsigned ElfParser_uiDynSymTabCount ;
ElfParser_SymbolTableList* ElfParser_pELFDynSymbolTable ;
unsigned* ElfParser_pHashTable ;

unsigned ElfParser_uiNoOfDynSectionEntries ;
unsigned ElfParser_uiDynSmyStrTabelSize ;

int exitNumber ;

#define TRACE_LINE printf("\n%s : %d", __FILE__, __LINE__)

#endif
