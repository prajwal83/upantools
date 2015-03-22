
#ifndef _ELF_HACKER_H_
#define _ELF_HACKER_H_

int ElfHacker_OpenELFFile(const char* elfFileName) ;
int ElfHacker_CloseELFFile() ;

int ElfHacker_HackFile() ;
void ElfHacker_DisplayHackInfo() ;

#endif
