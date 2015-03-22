#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "ElfSymbolTable.h"
#include "ElfConstants.h"
#include "ElfHeader.h"
#include "ElfProgHeader.h"
#include "ElfSectionHeader.h"
#include "ElfHacker.h"
#include "ElfDynamicSection.h"
#include "ElfRelocationSection.h"
#include "ElfGlobals.h"

int ElfHacker_OpenELFFile(const char* elfFileName)
{
	if((ElfHacker_File_fd = open(elfFileName, O_RDONLY)) < 0)
	{
		perror("OPEN") ;
		return -1 ;
	}

	ElfHacker_pSecHeaderStrTable = NULL ;
	ElfHacker_pDymStrTable = NULL ;
	ElfHacker_pSymStrTable = NULL ;
	ElfHacker_pELFProgramHeader = NULL ;
	ElfHacker_pELFSectionHeader = NULL ;
	ElfHacker_pELFSymbolTable = NULL ;
	ElfHacker_pELFDynSymbolTable = NULL ;
	ElfHacker_pELFDynamicSection = NULL ;
	ElfHacker_pELFRelocationSection = NULL ;
	ElfHacker_pELFDynRelocationSection = NULL ;
	ElfHacker_pELFRelocationSectionAd = NULL ;
	ElfHacker_pHashTable = NULL ;
	
	return 0 ;
}

int ElfHacker_CloseELFFile()
{
	if(close(ElfHacker_File_fd) < 0)
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

void ElfHacker_DumpGOT()
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

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(strcmp(GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name), ".got.plt") == 0)
		{
			if(lseek(ElfHacker_File_fd, ElfHacker_pELFSectionHeader[i].sh_offset, SEEK_SET) < 0)
			{
				perror("LSEEK") ;
				fclose(fp) ;
				return ;
			}
			
			if(read(ElfHacker_File_fd, (char*)strbuf, ElfHacker_pELFSectionHeader[i].sh_size) < 0)
			{
				perror("READ ELF GOT TABLE") ;
				fclose(fp) ;
				return ;
			}

			for(j = 0; j < ElfHacker_pELFSectionHeader[i].sh_size; j++)
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

void ElfHacker_DumpPLT()
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

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(strcmp(GET_SECTION_NAME(ElfHacker_pELFSectionHeader[i].sh_name), ".plt") == 0)
		{
			if(lseek(ElfHacker_File_fd, ElfHacker_pELFSectionHeader[i].sh_offset, SEEK_SET) < 0)
			{
				perror("LSEEK") ;
				fclose(fp) ;
				return ;
			}
			
			if(read(ElfHacker_File_fd, (char*)strbuf, ElfHacker_pELFSectionHeader[i].sh_size) < 0)
			{
				perror("READ ELF PLT TABLE") ;
				fclose(fp) ;
				return ;
			}

			for(j = 0; j < ElfHacker_pELFSectionHeader[i].sh_size; j++)
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

int ElfHacker_HackFile()
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

void ElfHacker_DisplayHackInfo()
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

	ElfHacker_DumpGOT() ;
	ElfHacker_DumpPLT() ;

	ElfStringTable_DisplaySecHeaderStrTable() ;
	ElfStringTable_DisplaySymStrTable() ;
	ElfStringTable_DisplayDymStrTable() ;
}

