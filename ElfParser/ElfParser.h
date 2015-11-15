
#ifndef _ELF_PARSER_H_
#define _ELF_PARSER_H_

int ElfParser_OpenELFFile(const char* elfFileName) ;
int ElfParser_CloseELFFile() ;

int ElfParser_HackFile() ;
void ElfParser_DisplayHackInfo() ;

#endif
