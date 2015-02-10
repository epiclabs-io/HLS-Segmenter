/*
 * Copyright (c) 2014 Stoian Ivanov
 * Copyright (c) 2009 Chase Douglas
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**********************************************************************************
 * This code is part of HTTP-Live-Video-Stream-Segmenter-and-Distributor
 * look for newer versions at github.com
 **********************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>


#include "segmenter.h"
//Type of options detected
typedef enum {
	INPUT_FILE, OUTPUT_FILE, OUTPUT_DIR, OUTPUT_BASE_NAME,SEGMENT_LENGTH, LIST_LENGTH, ON_GENERATION_CMD, REMOVE_EXPIRED,
	
	QUIET, VERSION, PRINT_USAGE,
	
	NO_MORE_OPTIONS,INVALID
	
} inputOption;

void printBanner() {
	static int  bannerPrinted= 1;
	if (bannerPrinted==1) {
		bannerPrinted++;
		fprintf(stderr,"Segmenter for HTTP Live Streaming v" PROGRAM_VERSION "\n");
		fprintf(stderr,"part of HTTP-Live-Video-Stream-Segmenter-and-Distributor @ github\n\n");
	}
}
void printUsage_short(int shortonly){
	printBanner();
	if (shortonly) fprintf(stderr, "\n");
	fprintf(stderr, "Usage: segmenter -i infile [-d baseDir] [-f baseFileName] [-o playListFile.m3u8] [-l <sec>] [-m maxlist] [-q] [-h] \n");
	if (shortonly) fprintf(stderr, "  segmenter -h for full help\n");
}

void printUsage() {
	printUsage_short(0);
	fprintf(stderr, "\nOptions (you can use -- or - for short option prefix e.g. -i == --i): \n"
					"-i <infile>\t\tInput file. Required. - is translated to 'pipe:' before handling to libav \n"
					"-o <outfile>\t\tPlaylist file to create. Default is <infile>.m3u8 \n"
					"-d <basedir>\t\tThe base directory for files. Default is  '.'\n"
					"-f <baseFileName>\tSegment files will be named <baseFileName>-#. Default is <outfile>\n"
					"-l <segment length>\tThe length of each segment. Default is 5\n"
					"-m <maximal list length>\tThe length of produced list. Default is - no limit. \"Hard\"  limit  at %u\n"
					"-c <cmd>\t\tExecute <cmd> as system command when index or media files are generated\n"
					"-D\t\t\tDelete segment files after they expire\n"
					"--version\t\tPrint version details and exit.\n"
					"-q,--quiet\t\tTry to be more quiet.\n"
					"-h,--help\t\tPrint this info.\n"
					"\n\n"
					"For the command that generate system commands, use %%P to indicate complete file path, and %%F for just the file name\n"
					"For example, 'echo %%P %%F' for index /Volumes/Data/Foo/bar/none.m3u8 will generate the command:\n"
					"echo /Volumes/Data/Foo/bar/none.m3u8 none.m3u8"
					"\n\n",MAX_SEGMENTS);
}


inputOption getNextOption(int argc, const char * argv[], char * option, int *optioni) {
	static int optionIndex = 1;

	if (optionIndex >= argc)
		return NO_MORE_OPTIONS;
	int hasnext=(optionIndex<argc-1);
	if (strcmp(argv[optionIndex],"-i")==0 || strcmp(argv[optionIndex],"--i")==0) {
		if (!hasnext){
			fprintf(stderr,"ERROR: input filename must be given after -i\n");
			return INVALID;
		}
		
		strncpy(option, argv[++optionIndex], MAX_FILENAME_LENGTH );
		
		//check for valid file name
		if (!isalpha(option[0]) && !isdigit(option[0]) && option[0] != '-' && option[0] != '/' && option[0] != '.') {
			fprintf(stderr,"ERROR: input filename must start with alpha, digit, / or . \n");
			return INVALID;
		}
		if (option[0]=='-' && option[1]==0)  {
			option[0]='p';
			option[1]='i';
			option[2]='p';
			option[3]='e';
			option[4]=':';
			option[5]=0;
		}
		optionIndex++;
		return INPUT_FILE;
	}

	if (strcmp(argv[optionIndex],"-o")==0 || strcmp(argv[optionIndex],"--o")==0) {
		if (!hasnext){
			fprintf(stderr,"ERROR: output filename must be given after -o\n");
			return INVALID;
		}
		strncpy(option, argv[++optionIndex], MAX_FILENAME_LENGTH - 1);

		//check for valid file name
		if (!isalpha(option[0]) && !isdigit(option[0]) && option[0] != '-' && option[0] != '/' && option[0] != '.') {
			fprintf(stderr,"ERROR: output filename must start with alpha, digit, / or . \n");
			return INVALID;
		}

		if (option[0] != '-' && strncmp(option + (strlen(option) - 5), ".m3u8", 5) != 0) {
			fprintf(stderr,"WARN: output file extension should be .m3u8\n\n");
			//return INVALID;
		}

		optionIndex++;
		return OUTPUT_FILE;
	}


	if (strcmp(argv[optionIndex],"-d")==0 || strcmp(argv[optionIndex],"--d")==0) {
		if (!hasnext){
			fprintf(stderr,"ERROR: output directory must be given after -d\n");
			return INVALID;
		}
		
		strncpy(option, argv[++optionIndex], MAX_FILENAME_LENGTH );

		//make sure it ends in a slash
		int l = strnlen(option,MAX_FILENAME_LENGTH - 2);
		if (option[l - 1] != '/') {
			option[l] = '/';
			option[l + 1] = 0;
		}

		//check for valid file name
		if (!isalpha(option[0]) && !isdigit(option[0]) && option[0] != '-' && option[0] != '/' && option[0] != '.') {
			fprintf(stderr,"ERROR: output directory must start with alpha, digit, / or . \n");
			return INVALID;
		}

		optionIndex++;
		return OUTPUT_DIR;
	}


	if (strcmp(argv[optionIndex],"-f")==0 || strcmp(argv[optionIndex],"--f")==0) {
		if (!hasnext){
			fprintf(stderr,"ERROR: output base filename must be given after -f\n");
			return INVALID;
		}
		
		strncpy(option, argv[++optionIndex], MAX_FILENAME_LENGTH );

		//check for valid file name
		if (!isalpha(option[0]) && !isdigit(option[0]) && option[0] != '-' && option[0] != '/' && option[0] != '.') {
			fprintf(stderr,"ERROR: output base filename must start with alpha, digit, / or . \n");
			return INVALID;
		}

		optionIndex++;
		return OUTPUT_BASE_NAME;
	}

	if (strcmp(argv[optionIndex],"-q")==0 || strcmp(argv[optionIndex],"--q" )==0|| strcmp(argv[optionIndex],"--quiet")==0) {
		optionIndex++;
		return QUIET;
	}


	if (strcmp(argv[optionIndex],"-l")==0 || strcmp(argv[optionIndex],"--l")==0) {
		if (!hasnext){
			fprintf(stderr,"ERROR: length must be given after -l\n");
			return INVALID;
		}		
		strncpy(option, argv[++optionIndex], MAX_FILENAME_LENGTH );
		option[MAX_FILENAME_LENGTH - 1] = 0;

		int a = strtol(option, NULL, 10);
		if (a == 0 && errno != 0) {
			fprintf(stderr,"ERROR: length must be an integer.\n");
			return INVALID;
		}

		if (a <= 2) {
			fprintf(stderr, "ERROR: Segment duration time must be > 2.\n");
			return INVALID;
		}
		if (optioni) *optioni=a;
		optionIndex++;
		return SEGMENT_LENGTH;
	}

	if (strcmp(argv[optionIndex],"-m")==0 || strcmp(argv[optionIndex],"--m")==0) {
		if (!hasnext){
			fprintf(stderr,"ERROR: length must be given after -m\n");
			return INVALID;
		}		
		strncpy(option, argv[++optionIndex], MAX_FILENAME_LENGTH );
		option[MAX_FILENAME_LENGTH - 1] = 0;

		int a = strtol(option, NULL, 10);
		if (a == 0 && errno != 0) {
			fprintf(stderr,"ERROR: list length must be an integer.\n");
			return INVALID;
		}

		if (a <= 2) {
			fprintf(stderr, "ERROR: list list must be >= 3.\n");
			return INVALID;
		}
		if (optioni) *optioni=a;
		optionIndex++;
		return LIST_LENGTH;
	}

	if (strcmp(argv[optionIndex],"-c")==0 || strcmp(argv[optionIndex],"--c")==0) {
		if (!hasnext){
			fprintf(stderr,"ERROR: command line should be provided after -c\n");
			return INVALID;
		}		
		strncpy(option, argv[++optionIndex], MAX_FILENAME_LENGTH );

		//check for valid file name
		if (!isalpha(option[0]) && !isdigit(option[0]) && option[0] != '-' && option[0] != '/' && option[0] != '.') {
			fprintf(stderr,"ERROR: command line must start with alpha, digit, / or . \n");
			return INVALID;
		}

		optionIndex++;
		return ON_GENERATION_CMD;
	}

	if (strcmp(argv[optionIndex],"-D")==0 ) {
		optionIndex++;
		return REMOVE_EXPIRED;
	}

	if (strcmp(argv[optionIndex],"--version")==0 ) {
		optionIndex++;
		return VERSION;
	}

	if (strcmp(argv[optionIndex],"-h")==0 || strcmp(argv[optionIndex],"--help")==0) {
		optionIndex++;
		return PRINT_USAGE;
	}

	fprintf(stderr, "ERROR: Unknown option %s\n",argv[optionIndex]);

	return INVALID;
}


#define INPUT_FILE_INDEX 0
#define OUTPUT_FILE_INDEX 1
#define OUTPUT_BASE_NAME_INDEX 2

//assumes that the pointers have allocated memory

int parseCommandLine(
	int argc, const char * argv[],
	
	char * inputFile, char * outputFile, char * baseDir, char * baseName, char * baseExtension, int * segmentLength, int *listlen,
	
	int * quiet, int * version, int * usage, char *cmd, int *removeExpired
	
) {
	printBanner();
	int requiredOptions[3] = {0, 0, 0};

	inputOption result;
	char option[MAX_FILENAME_LENGTH];
	int optioni;
	//default video and audio output
	*quiet = 0;
	*version = 0;
	*usage = 0;
	*segmentLength=5;
	strncpy(baseExtension, ".ts",MAXT_EXT_LENGTH);
	baseDir[0]='.';baseDir[1]=0;
	*removeExpired = 0;
	cmd[0] = 0;
	*listlen=-1;
	
	while (1) {
		result = getNextOption(argc, argv, option,&optioni);
		switch (result) {
			case INPUT_FILE:
				strncpy(inputFile, option, MAX_FILENAME_LENGTH);
				requiredOptions[INPUT_FILE_INDEX] = 1;
				break;
			case OUTPUT_FILE:
				strncpy(outputFile, option, MAX_FILENAME_LENGTH);
				requiredOptions[OUTPUT_FILE_INDEX] = 1;
				break;
			case ON_GENERATION_CMD:
				strncpy(cmd, option, MAX_FILENAME_LENGTH);
				break;
			case OUTPUT_DIR:
				strncpy(baseDir, option, MAX_FILENAME_LENGTH);
				break;
			case OUTPUT_BASE_NAME:
				strncpy(baseName, option, MAX_FILENAME_LENGTH);
				requiredOptions[OUTPUT_BASE_NAME_INDEX] = 1;
				break;
			case SEGMENT_LENGTH:
				*segmentLength = optioni;
			case LIST_LENGTH:
				*listlen = optioni;
			case QUIET:
				*quiet = 1;
				break;
			case VERSION:
				*version = 1;
				break;
			case PRINT_USAGE:
				*usage = 1;
				break;
			case REMOVE_EXPIRED:
				*removeExpired = 1;
				break;
			case NO_MORE_OPTIONS:
			{
				if (argc==1) *usage=1;
				if (*version == 1 || *usage == 1) 
					return 0;
				
				int missing = 0;

				if (requiredOptions[INPUT_FILE_INDEX] == 0) {
					fprintf(stderr, "ERROR: Missing required option --i for input file.\n");
					missing = 1;
					return -1;
				}
				if (requiredOptions[OUTPUT_FILE_INDEX] == 0) {
					char *from=inputFile,*to=outputFile,*lastd=NULL,cc,*rebase=NULL; 
					while (cc=*from) {
						from++;
						*to=cc;
						if (cc=='.') lastd=to;
						to++;
						if (cc=='/') rebase=to;
					}
					*to=0;
					if (!lastd) lastd=to;
					if (lastd-outputFile<MAX_FILENAME_LENGTH-5){
						if (*lastd!='.') *lastd='.';
						lastd[1]='m';
						lastd[2]='3';
						lastd[3]='u';
						lastd[4]='8';
						lastd[5]=0;
					}
					if (rebase && *rebase!=0){
						to=outputFile;
						while (*rebase!=0){
							*to=*rebase;
							rebase++;to++;
						}
						*to=0;
					}
				}
				

				if (requiredOptions[OUTPUT_BASE_NAME_INDEX] == 0) {
					char *from=outputFile,*to=baseName,*lastd=NULL,cc; 
					while (cc=*from) {
						from++;
						*to=cc;
						if (cc=='.') lastd=to;
						to++;
					}
					*to=0;
					if (!lastd) {
						lastd=to;
						lastd[0]='-';
						lastd[1]='p';
						lastd[2]='0';
					} else lastd[0]=0;
					
				}

				if (missing == 1) {
					printUsage_short(1);
					return -1;
				}

				return 0;
			}
			case INVALID:
			default:
				return -1;
		}
	}

	return 0;
}
