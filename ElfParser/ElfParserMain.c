#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "ElfConstants.h"
#include "ElfHeader.h"
#include "ElfProgHeader.h"
#include "ElfSectionHeader.h"
#include "ElfParser.h"
#include "ElfGlobals.h"

int main(int argc, char** argv)
{
	if(argc != 2)
	{
		fprintf(stderr, "\n Usage elfparser <ElfExe>\n") ;
		exit(1) ;
	}

	char* elfFileName = argv[1] ;
	exitNumber = 0 ;

//	printf("\n Working on File: %s\n", elfFileName) ;

	if(ElfParser_OpenELFFile(argv[1]) < 0)
		exit(++exitNumber) ;

	if((exitNumber = ElfParser_HackFile() < 0))
		exit(exitNumber) ;

	ElfParser_DisplayHackInfo() ;

	if(ElfParser_CloseELFFile() < 0)
		exit(++exitNumber) ;
		
	return exitNumber ;
}

