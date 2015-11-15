#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "ElfSymbolTable.h"
#include "ElfConstants.h"
#include "ElfHeader.h"
#include "ElfProgHeader.h"
#include "ElfSectionHeader.h"
#include "ElfParser.h"
#include "ElfDynamicSection.h"
#include "ElfRelocationSection.h"
#include "ElfGlobals.h"

int ElfParser_OpenELFFile(const char* elfFileName)
{
	if((ElfParser_File_fd = open(elfFileName, O_RDONLY)) < 0)
	{
		perror("OPEN") ;
		return -1 ;
	}

	ElfParser_pSecHeaderStrTable = NULL ;
	ElfParser_pDymStrTable = NULL ;
	ElfParser_pSymStrTable = NULL ;
	ElfParser_pELFProgramHeader = NULL ;
	ElfParser_pELFSectionHeader = NULL ;
	ElfParser_pELFSymbolTable = NULL ;
	ElfParser_pELFDynSymbolTable = NULL ;
	ElfParser_pELFDynamicSection = NULL ;
	ElfParser_pELFRelocationSection = NULL ;
	ElfParser_pELFDynRelocationSection = NULL ;
	ElfParser_pELFRelocationSectionAd = NULL ;
	ElfParser_pHashTable = NULL ;
	
	return 0 ;
}

int ElfParser_CloseELFFile()
{
	if(close(ElfParser_File_fd) < 0)
	{
		perror("CLOSE") ;
		return -1 ;
	}
	
	ElfProgHeader_DeAllocate() ;
	ElfSectionHeader_DeAllocate() ;
	ElfSymbolTable_DeAllocate() ;
	ElfDynSymbolTable_DeAllocate() ;
	ElfDynamicSection_DeAllocate() ;
	ElfRelocationSection_DeAllocateDyn() ;
	ElfRelocationSection_DeAllocateWithOutAddends() ;
	ElfRelocationSection_DeAllocateWithAddends() ;
	ElfHashTable_DeAllocateHashTable() ;
	ElfStringTable_DeAllocateSecHeaderStrTable() ;
	ElfStringTable_DeAllocateDymStrTable() ;
	ElfStringTable_DeAllocateSymStrTable() ;

	return 0 ;
}

void ElfParser_DumpGOT()
{
	printf("\n************ ELF GOT START **********************\n") ;
	char strbuf[4096] ;
	unsigned i, j ;
	unsigned strTableNumber = 0 ;

	FILE* fp ;
	if((fp = fopen("got.dump", "w+")) == NULL)
	{
		perror("OPEN got.dump") ;
		return ;
	}

	for(i = 0; i < ElfParser_elfHeader.e_shnum; i++)
	{
		if(strcmp(GET_SECTION_NAME(ElfParser_pELFSectionHeader[i].sh_name), ".got.plt") == 0)
		{
			if(lseek(ElfParser_File_fd, ElfParser_pELFSectionHeader[i].sh_offset, SEEK_SET) < 0)
			{
				perror("LSEEK") ;
				fclose(fp) ;
				return ;
			}
			
			if(read(ElfParser_File_fd, (char*)strbuf, ElfParser_pELFSectionHeader[i].sh_size) < 0)
			{
				perror("READ ELF GOT TABLE") ;
				fclose(fp) ;
				return ;
			}

			for(j = 0; j < ElfParser_pELFSectionHeader[i].sh_size; j++)
				fprintf(fp, "%c", strbuf[j]) ;

		}
	}
	fclose(fp) ;
	system("ndisasm -b32 ./got.dump > got.out") ;
	
	if((fp = fopen("got.out", "r")) == NULL)
	{
		perror("OPEN got.out") ;
		return ;
	}

	do
	{
		printf("%c", fgetc(fp)) ;
	} while(!feof(fp)) ;

	fclose(fp) ;

	unlink("got.out") ;
	unlink("got.dump") ;
	printf("\n************ ELF GOT END **********************\n") ;
}

void ElfParser_DumpPLT()
{
	printf("\n************ ELF PLT START **********************\n") ;
	char strbuf[4096] ;
	unsigned i, j ;
	unsigned strTableNumber = 0 ;

	FILE* fp ;
	if((fp = fopen("plt.dump", "w+")) == NULL)
	{
		perror("OPEN plt.dump") ;
		return ;
	}

	for(i = 0; i < ElfParser_elfHeader.e_shnum; i++)
	{
		if(strcmp(GET_SECTION_NAME(ElfParser_pELFSectionHeader[i].sh_name), ".plt") == 0)
		{
			if(lseek(ElfParser_File_fd, ElfParser_pELFSectionHeader[i].sh_offset, SEEK_SET) < 0)
			{
				perror("LSEEK") ;
				fclose(fp) ;
				return ;
			}
			
			if(read(ElfParser_File_fd, (char*)strbuf, ElfParser_pELFSectionHeader[i].sh_size) < 0)
			{
				perror("READ ELF PLT TABLE") ;
				fclose(fp) ;
				return ;
			}

			for(j = 0; j < ElfParser_pELFSectionHeader[i].sh_size; j++)
				fprintf(fp, "%c", strbuf[j]) ;

		}
	}
	fclose(fp) ;
	system("ndisasm -b32 ./plt.dump > plt.out") ;
	
	if((fp = fopen("plt.out", "r")) == NULL)
	{
		perror("OPEN plt.out") ;
		return ;
	}

	do
	{
		printf("%c", fgetc(fp)) ;
	} while(!feof(fp)) ;

	fclose(fp) ;

	unlink("plt.out") ;
	unlink("plt.dump") ;
	printf("\n************ ELF PLT END **********************\n") ;
}

int ElfParser_HackFile()
{
	if(ElfHeader_Read() < 0)
		return(++exitNumber) ;

	if(ElfProgHeader_Read() < 0)
		return(++exitNumber) ;

	if(ElfSectionHeader_Read() < 0)
		return(++exitNumber) ;

	if(ElfSymbolTable_Read() < 0)
		return(++exitNumber) ;

	if(ElfDynSymbolTable_Read() < 0)
		return(++exitNumber) ;

	if(ElfStringTable_ReadSecHeaderStrTable() < 0)
		return(++exitNumber) ;

	if(ElfStringTable_ReadSymStrTable() < 0)
		return(++exitNumber) ;

	if(ElfStringTable_ReadDymStrTable() < 0)
		return(++exitNumber) ;

	if(ElfDynamicSection_Read() < 0)
		return(++exitNumber) ;

	if(ElfDynRelocationSection_Read() < 0)
		return(++exitNumber) ;

	if(ElfRelocationSection_Read() < 0)
		return(++exitNumber) ;

	if(ElfRelocationSection_ReadAd() < 0)
		return(++exitNumber) ;

	if(ElfHashTable_Read() < 0)
		return(++exitNumber) ;

	return 0 ;
}

void ElfParser_DisplayHackInfo()
{
	ElfHeader_Display() ;
	ElfProgHeader_Display() ;
	ElfSectionHeader_Display() ;
	ElfSymbolTable_Display() ;
	ElfDynSymbolTable_Display() ;
	ElfDynamicSection_Display() ;
	ElfRelocationSection_DisplayDyn() ;
	ElfRelocationSection_DisplayWithOutAddends() ;
	ElfRelocationSection_DisplayWithAddends() ;

	ElfParser_DumpGOT() ;
	ElfParser_DumpPLT() ;

	ElfStringTable_DisplaySecHeaderStrTable() ;
	ElfStringTable_DisplaySymStrTable() ;
	ElfStringTable_DisplayDymStrTable() ;
}

