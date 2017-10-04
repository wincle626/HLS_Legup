#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <bfd.h>

#define CONTROL_PORT 1338

// profiling definitions
//#define POWER
#define PROF_TYPE 1	// 0 == vanprof, 1 == SnoopP

#if PROF_TYPE == 0
	#define NUM_FUNCTIONS 64
	#ifdef POWER
		#define OUTPUT_NAME "%s.v.power.bravo.results"
	#else
		#define OUTPUT_NAME "%s.v.cycle.hier.results"
	#endif
#else
	#define NUM_FUNCTIONS 64
	#define OUTPUT_NAME "%s.s.cycle.hier.results"
#endif

#include "../Debug_Stub/regdef.h"	// include for STACK_ADDR & PROF_ADDR definition
//#include "../Debug_Stub/utils.s"

void displayProfilerResults(int *results, char* flist);
int serverSocket;

// Power in nJ
float DB_A = 38.45;
float DB_B = 57.53;
float DB_C = 62.82;
float DB_D = 68.03;
float DB_E = 75.88;
float DB_F = 101.6;
float DB_STALLS = 44.34;

// ADDRESS HASH
long int V1;
int A1, A2;
int B1, B2;
int tab[NUM_FUNCTIONS+1];

void init(char *fname) {
	FILE *fHash = fopen(fname, "r");
	if (fHash == NULL) { printf("Hash file (%s) not found!\n", fname); exit(1); }

	// Read/parse data from .prof file as 8+N bytes of information (TAB,V1,A1,A2,B1,B2)
	fscanf(fHash, "tab[] = {");
	int i;
	for (i=0; i<NUM_FUNCTIONS; i++) fscanf(fHash, "%d,", &tab[i]);
	tab[NUM_FUNCTIONS] = '\0';	// so we can use it as a string to output

	fscanf(fHash, "}\nV1 = 0x%lx\n", &V1);
	fscanf(fHash, "A1 = %d\n", &A1);
	fscanf(fHash, "A2 = %d\n", &A2);
	fscanf(fHash, "B1 = %d\n", &B1);
	fscanf(fHash, "B2 = 0x%x\n", &B2);
	fclose(fHash);
}

int doHash (unsigned int val) {
	unsigned int a, b, rsl;
	
	val &= 0x3FFFFFF;	// only take bottom 26 bits b/c thats all PC is
	
	val += V1;
	val += (val << 8);
	val ^= (val >> 4);
	b = (val >> B1) & B2;
	
	// fix C++ issue where shifting left 32 bits does nothing
	if (A1 != 32) 	a = (val + (val << A1)) >> A2;
	else		  	a = val >> A2;
	
	rsl = (a^tab[b]);		
	rsl &= NUM_FUNCTIONS-1;
	
	return rsl;
}

int serverSend(void* datav, unsigned int length) {
	char* data = (char*)datav;
	int amount_sent = 0;
	while(amount_sent < length)	{
		int ret = send(serverSocket, data + amount_sent, length - amount_sent, 0);
		
		if(ret == -1) return 1;
		
		amount_sent += ret;	
	}	
	
	return 0;
}

int serverRecv(void* datav, unsigned int length) {
	char* data = (char*)datav;
	int amount_recv = 0;
	while(amount_recv < length)	{
		int ret = recv(serverSocket, data + amount_recv, length - amount_recv, 0);
		
		if(ret == -1 || ret == 0) return 1;
			
		amount_recv += ret;	
	}
	
	return 0;
}

int sendSection(char* sectionData, unsigned int addr, unsigned int length) {
 	if (serverSend("M", 1)) 				return 1;
	if (serverSend(&addr, 4))				return 1;
	if (serverSend(&length, 4))				return 1;
	if (serverSend(sectionData, length))	return 1;
		
	return 0;
}

int verifySection(char* sectionData, unsigned int addr, unsigned int length) {
	if (serverSend("m", 1))					return 1;
	if (serverSend(&addr, 4))				return 1;
	if (serverSend(&length, 4))				return 1;
		
	char* readData = malloc(length);
	if(serverRecv(readData, length)) {
		free(readData);
		return 1;
	}
	
	int i;
	for(i = 0;i < length; i++) {
		if(sectionData[i] != readData[i]) {
			free(readData);
			return 2;
		}	
	}
	
	return 0;
}

// PROFILER DATA SEND
int sendProfileData (char* hash_file) {
	int i;
	printf("Reading profile data from %s\n", hash_file);
	FILE *prof = fopen(hash_file, "r");
	
	// VanProf
	#if PROF_TYPE == 0
		int tab_i;
		char tab[NUM_FUNCTIONS+1];
		long int V1;
		int A1,A2, B1,B2;
		unsigned int length = NUM_FUNCTIONS+8;		
		char prof_data[8+NUM_FUNCTIONS+5];
	
		// Read/parse data from .hash file as 8+N bytes of information (TAB,V1,A1,A2,B1,B2)
		fscanf(prof, "tab[] = {");
		for (i=0; i<NUM_FUNCTIONS; i++) {
			fscanf(prof, "%d,", &tab_i);
			tab[i] = (char)tab_i;
		}

		tab[NUM_FUNCTIONS] = '\0';	// so we can use it as a string to output
		fscanf(prof, "}\nV1 = 0x%lx\n", &V1);
		fscanf(prof, "A1 = %d\n", &A1);
		fscanf(prof, "A2 = %d\n", &A2);
		fscanf(prof, "B1 = %d\n", &B1);
		fscanf(prof, "B2 = 0x%x\n", &B2);
		
		// Test Output
		if (1) {
			printf("tab = { ");
			for (i=0; i<NUM_FUNCTIONS; i++) printf("%d, ", (int)tab[i]);
			printf("}\n");
			printf("V1 = 0x%lx, A1 = %d, A2 = %d, B1 = %d, B2 = %d\n", V1, A1, A2, B1, B2);
		}
		
		// put data into one char array for sending to profiler
		int j;
		for (i=0; i<NUM_FUNCTIONS; i+=4) {
			sprintf(&prof_data[i], "%c%c%c%c", tab[i+3], tab[i+2], tab[i+1], tab[i+0]);
		}
		
		sprintf(&prof_data[NUM_FUNCTIONS], "%c%c%c%c%c%c%c%c",
											(char)(V1&0x00FF), (char)((V1>>8)&0x00FF), (char)((V1>>16)&0x00FF), (char)((V1>>24)&0x00FF),
											(char)(B2&0x00FF),  (char)(B1&0x00FF), (char)(A2&0x00FF), (char)(A1&0x00FF)); //NUM_FUNCTIONS
	// SnoopP
	#else
		unsigned long int range;
		int idx = 0;
		char prof_data[NUM_FUNCTIONS*4*2];	// 4 bytes per address, 2 address per function range
		unsigned int length = NUM_FUNCTIONS*2*4;
		while (fscanf(prof, "%x", &range) > 0) {
			printf("%x\n", range);			
			sprintf(&prof_data[idx*4], "%c%c%c%c", (char)(range&0x00FF), (char)((range>>8)&0x00FF), (char)((range>>16)&0x00FF), (char)((range>>24)&0x00FF));
			idx++;
		}
		while (idx < NUM_FUNCTIONS*2) {
			sprintf(&prof_data[idx*4], "%c%c%c%c", 0,0,0,0);
			idx++;
		}
	#endif
										
	// send/verify all (8+N) or 8*N bytes using sendSection & verifySection
	printf("Sending Profiler Data\n");	
	if (sendSection(prof_data, PROF_ADDR, length))	{
		printf("Error sending profile data, error was: %s\nAborting...\n", strerror(errno));
		return 1;
	}
	
	int verifyResult = verifySection(prof_data, PROF_ADDR, length);
	if (verifyResult == 0) {
		printf("Profile data verify succeeded\n");
	} else if(verifyResult == 1) {
		printf("Socket error while verifying profile data, error was: %s\nAborting...\n", strerror(errno));
		return 1;
	} else if(verifyResult == 2) {
		printf("Error verifying section: profile data, aborting\n");
		return 1;
	}
	
	return 0;
}

// PROFILER DATA RETRIEVAL
int recvProfileData (char* flist) {
	printf("Retrieving profiling data...\n");
	
	#ifndef POWER
		unsigned int length = NUM_FUNCTIONS*32;		// assume 32 bits/function (for counter)
		int *profile_results = (int*)malloc(sizeof(int)*NUM_FUNCTIONS);
	#else
		unsigned int length = NUM_FUNCTIONS*160;	// assume 160 bits/function (for power)
		int *profile_results = (int*)malloc(sizeof(int)*NUM_FUNCTIONS*5);		// need to keep it func_num-addressable
	#endif
	unsigned int addr = PROF_ADDR;
	
	// Send request for data
	if (serverSend("m", 1))		return 1;
	if (serverSend(&addr, 4))	return 1;
	if (serverSend(&length, 4))	return 1;

	// Actually receive data
	if (serverRecv((int*)(&profile_results[0]), length))	return 1;
	printf("Results received, processing.\n");
	
	// print data to terminal, and/or store to file
	displayProfilerResults(&profile_results[0], flist);

	printf("All results returned successfully.\n");
	
	return 0;
}

void displayProfilerResults(int *results, char* flist) {
	// return data -- load filelist and hash to get results
	char* p;
	if(!(p = strstr(flist, ".flist"))) { printf("error with flist!\n"); exit(1); }
	char* bench_name = (char*)malloc(sizeof(char)*(strlen(flist)+10));// = "bench";
	strncpy(bench_name, flist, p-flist);
	bench_name[p-flist]='\0';
	char* flist_lab = (char*)malloc(sizeof(char)*(strlen(flist)+6));// = "bench.flist_lab";
	sprintf(flist_lab, "%s_lab", flist);
	char* results_file = (char*)malloc(sizeof(char)*(strlen(bench_name)+25));// = "bench.results";	
	sprintf(results_file, OUTPUT_NAME, bench_name);
	char* hash_file = (char*)malloc(sizeof(char)*(strlen(bench_name)+6));// = "bench.hash";	
	sprintf(hash_file, "%s.hash", bench_name);

	FILE *fFList = fopen(flist, "r");				// ${BENCHMARK}.flist
	FILE *fFList_lab = fopen(flist_lab, "r");		// ${BENCHMARK}.flist_lab
	FILE *fout = fopen(results_file, "w");
	int f_addr, f_num, f_cnt=0;
	char label[100];
	int i;

	init(hash_file);

	// optionally display raw data
	/*#if PROF_TYPE == 1
		printf("\nRaw Profiling Results:\n");
		fprintf(fout, "Raw Profiling Results:\n");
		for (i=0; i<NUM_FUNCTIONS; i++) {
			printf(       "%5d: 0x%04x = %d\n", i, results[i], results[i]);
			fprintf(fout, "%5d: 0x%04x = %d\n", i, results[i], results[i]);
		}
		//return;
	#endif*/
	
	if (fFList == NULL) { printf("flist file not found!\n"); exit(1); }
	if (fFList_lab == NULL) { printf("flist_lab file (%s) not found!\n", flist_lab); exit(1); }

	printf("\nProfile Results by Function:\n");
	fprintf(fout, "\nProfile Results by Function:\n");
	
	// find longest function label
	int label_size;
	int max_label_len = 0;
	while (fscanf(fFList_lab, "%s%n\n", &label, &label_size) != EOF) {
		if (label_size > max_label_len) max_label_len = label_size;
	}
	fclose(fFList_lab);
	fFList_lab = fopen(flist_lab, "r");	// re-open the file
	
	#ifndef POWER
		while (fscanf(fFList, "%x", &f_addr) != EOF) {
			fscanf(fFList_lab, "%s", label);
			
			#if PROF_TYPE == 0
				f_num = doHash(f_addr);	// van prof
			#else
				f_num = f_cnt++;		// SnoopP
			#endif
			
			printf(       "%*s [%x -> %2d] = %5d\n", max_label_len, label, f_addr, f_num, results[f_num]);
			fprintf(fout, "%*s [%x -> %2d] = %5d\n", max_label_len, label, f_addr, f_num, results[f_num]);
		}
	#else	
		float energy_sum = 0;
		int A,B,C,D,E,F,STALLS;
		int countA=0, countB=0, countC=0, countD=0, countE=0, countF=0, countSTALLS=0;
		int r[5];
		
		while (fscanf(fFList, "%x", &f_addr) != EOF) {
			fscanf(fFList_lab, "%s", label);
			
			f_num = doHash(f_addr);	// van prof
			
			r[0] = results[f_num*5 + 0];
			r[1] = results[f_num*5 + 1];
			r[2] = results[f_num*5 + 2];
			r[3] = results[f_num*5 + 3];
			r[4] = results[f_num*5 + 4];
			
			A = (r[0] >> 10) & 0x3FFFFF;							// first 22 bits of r0
			B = ((r[0]&0x3FF) << 12)   | ((r[1] >> 20) & 0xFFF);	// last 10 bits of r0, first 12 bits of r1
			C = ((r[1]&0xFFFFF) << 2)  | ((r[2] >> 30) & 0x3);		// last 20 bits of r1, first 2 bits of r2
			D = (r[2] >> 8) & 0x3FFFFF;								// 22 bits of r2 starting at bit 3
			E = ((r[2]&0xFF) << 14)    | ((r[3] >> 18) & 0x3FFF);	// last 8 bits of r2, first 14 bits of r3
			F = ((r[3]&0x3FFFF) << 4)  | ((r[4] >> 28) & 0xF);		// last 18 bits of r3, first 4 bits of r4
			STALLS = r[4] & 0xFFFFFFF;								// last 28 bits of r4
			
			countA += A;
			countB += B;
			countC += C;
			countD += D;
			countE += E;
			countF += F;
			countSTALLS += STALLS;
			
			float energy = ((float)DB_A*(float)A + DB_B*(float)B + DB_C*(float)C + DB_D*(float)D + DB_E*(float)E + DB_F*(float)F + DB_STALLS*(float)STALLS)*0.04;
			energy_sum += energy;
			
			printf(       "%*s [%x -> %2d] = %5d|%5d|%5d|%5d|%5d|%5d|%5d => %8.2f nJ\n", max_label_len, label, f_addr, f_num, A, B, C, D, E, F, STALLS, energy);
			fprintf(fout, "%*s [%x -> %2d] = %5d|%5d|%5d|%5d|%5d|%5d|%5d => %8.2f nJ\n", max_label_len, label, f_addr, f_num, A, B, C, D, E, F, STALLS, energy);
		}
		for (i=0; i<(max_label_len+74); i++) printf("-");
		printf("\n");
		printf("%*cTotal Instructions => %8d\n", (max_label_len+41), ' ', (countA+countB+countC+countD+countE+countF));
		printf("%*cTotal Stalls => %8d\n", (max_label_len+47), ' ', countSTALLS);
		printf("%*cTotal Energy => %8.2f nJ\n", (max_label_len+47), ' ', energy_sum);
		for (i=0; i<(max_label_len+74); i++) fprintf(fout, "-");
		fprintf(fout, "\n");
		fprintf(fout, "%*cTotal Instructions => %8d\n", (max_label_len+41), ' ', (countA+countB+countC+countD+countE+countF));
		fprintf(fout, "%*cTotal Stalls => %8d\n", (max_label_len+47), ' ', countSTALLS);
		fprintf(fout, "%*cTotal Energy => %8.2f nJ\n", (max_label_len+47), ' ', energy_sum);
	#endif

	fclose(fFList);
	fclose(fFList_lab);
	fclose(fout);
}

int connectToServer(char* address) {
	serverSocket = socket(PF_INET, SOCK_STREAM, 0);
	
	if(serverSocket == -1) return 1;
		
	struct hostent* he;
	struct sockaddr_in serverAddr;
	
	he = gethostbyname(address);
	if(he == 0) return 2;
		
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(CONTROL_PORT);
	serverAddr.sin_addr = *((struct in_addr*)he->h_addr);
	memset(serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero));
	
	if(connect(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
		return 3;
		
	return 0;
}

int setRegister(char regNum, unsigned int value) {
	if(serverSend("P", 1)) {
		printf("Error communicating with server, error was: %s\nAborting...\n", strerror(errno));
		return 1;
	}
	
	if(serverSend(&regNum, 1)) {
		printf("Error communicating with server, error was: %s\nAborting...\n", strerror(errno));
		return 1;
	}
	
	if(serverSend(&value, 4)) {
		printf("Error communicating with server, error was: %s\nAborting...\n", strerror(errno));
		return 1;
	}
	
	char c;
	if(serverRecv(&c, 1)) {
		printf("Error communicating with server, error was: %s\nAborting...\n", strerror(errno));
		return 1;
	}
	
	return 0;
}

int resetRegisters(unsigned int PC) {
	//char i;
	// for(i = 0;i < 33; ++i) {
	//	 if(setRegister(i, 0))		return 1;
	// }
	if (setRegister(33, PC))		return 1;
	if (setRegister(28, STACK_ADDR))	return 1;
	
	return 0;
}

int main(int argc, char* argv[]) {
	int i;
	// Usage: -P for loading profile data, -p for retrieving results, -r to load elf & run, -R to run (don't load elf)
	if(argc > 6 || argc < 3) 	{
		printf("MIPSLoad, takes an executable and loads it on to a Tiger MIPS System.\n"
		       "The MIPS communication server must be running for this to work\n"
		       "Written by Greg Chadwick Summer 2007\n\n"
		       "Usage: MIPSLoad executable server_addr [-r]\n"
		       "Use r option to start running immediately, otherwise processor starts\n"
		       "halted waiting for the debugger\n");
		return 1;
	}
	
	int connectError = connectToServer(argv[2]);
	if(connectError == 1) 	{
		printf("Could not create socket, error was: %s\nAborting...\n", strerror(errno));
		return 1;

	} else if(connectError == 2) {
		printf("Could not get server address, error was: %s\nAborting...\n", strerror(errno));
		return 1;

	} else if(connectError == 3) {
		printf("Could not connect to server, error was: %s\nAborting...\n", strerror(errno));
		return 1;
	}

	// send hash info to initialize profiler
	if (argc == 6 && !strcmp(argv[4], "-P")) {
		if(serverSend("I", 1)) {
			printf("Error communicating with server, error was: %s\nAborting...\n", strerror(errno));
			return 1;
		}
		sendProfileData(argv[5]);
		return 1;
	}

	// add switch to get profiler data from SDRAM (put before the elf loading since we don't want to actually re-send the data)
	if(argc == 5 && !strcmp(argv[3], "-p"))	{
		if(serverSend("I", 1)) {
			printf("Error communicating with server, error was: %s\nAborting...\n", strerror(errno));
			return 1;
		}
		recvProfileData(argv[4]);
		return 0;
	}

	bfd_init();
	bfd* executable = bfd_openr(argv[1], 0);
	if(!executable) {
		bfd_error_type error = bfd_get_error();
		switch(error) {
			case bfd_error_no_memory:
				printf("Cannot open executable, insufficient memory\n");
				return 1;
			case bfd_error_invalid_target:
				printf("Cannot open executable, it is not an elf32-littlemips target\n");
				return 1;
			case bfd_error_system_call:
			default:
				printf("Error opening executable\n");
				return 1;
		}
	}
	
	bfd_check_format(executable, bfd_object);
	
	if(serverSend("I", 1)) {
		printf("Error communicating with server, error was: %s\nAborting...\n", strerror(errno));
		return 1;
	}
	
	asection* section;
	for(section = executable->sections; section != 0; section = section->next) {
		//If it doesn't need to be loaded or if it's debug info skip over it
		if (!(section->flags & SEC_LOAD || section->flags & SEC_ALLOC) || (section->flags & SEC_DEBUGGING)) {
			printf("Skipping section: %s\n", section->name);
			continue;
		}
			
		if((section->flags & SEC_RELOC)) {
			printf("Warning section %s has relocation information, this will be ignored\n", section->name);
		}
		
		unsigned char* sectionData;
		if(!bfd_malloc_and_get_section(executable, section, &sectionData)) {
			printf("Error getting section: %s, aborting\n", section->name);
			return 1;
		}
		
		unsigned int LMA = section->lma;
		unsigned int Size = section->size;
		printf("Loading section: %s, length: %X, address: %X\n", section->name, Size, LMA);
		
		if(sendSection(sectionData, LMA, Size)) {
			printf("Error sending section: %s, error was: %s\nAborting...\n", section->name, strerror(errno));
			free(sectionData);
			return 1;
		}
		
		int verifyResult = verifySection(sectionData, section->lma, section->size);
		if(verifyResult == 0) {
			printf("Section %s verify succeeded\n", section->name);

		} else if(verifyResult == 1) {
			printf("Socket error while verifying section %s, error was: %s\nAborting...\n", section->name, strerror(errno));
			free(sectionData);
			return 1;
		} else if(verifyResult == 2) {
			printf("Error verifying section: %s, aborting\n", section->name);
			free(sectionData);
			return 1;
		}
		
		free(sectionData);
	}
	
	printf("Load complete\n");
	
	if(resetRegisters(executable->start_address)) return 1;
	
	if(argc == 4 && (!strcmp(argv[3], "-r") || !strcmp(argv[3], "-R"))) {
		printf("Starting execution...\n");
		
		unsigned int addr = executable->start_address;
		
		if(serverSend("r", 1)) {
			printf("Error communicating with server, error was: %s\nAborting...\n", strerror(errno));
			return 1;
		}
		
		if(serverSend(&addr, 4)) {
			printf("Error communicating with server, error was: %s\nAborting...\n", strerror(errno));
			return 1;
		}
		
		char c;
		if(serverRecv(&c, 1)) {
			printf("Error communicating with server, error was: %s\nAborting...\n", strerror(errno));
			return 1;
		}
	}		
	
	return 0;
}
