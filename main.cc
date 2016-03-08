/*******************************************************
                          main.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fstream>
using namespace std;
#include "cache.cc"
#include "cache.h"

int main(int argc, char *argv[])
{
	
	ifstream fin;
	FILE * pFile;

	if(argv[1] == NULL){
		 printf("input format: ");
		 printf("./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> \n");
		 exit(0);
        }

	/*****uncomment the next five lines*****/
	int cache_size = atoi(argv[1]);
	int cache_assoc= atoi(argv[2]);
	int blk_size   = atoi(argv[3]);
	int num_processors = atoi(argv[4]);/*1, 2, 4, 8*/
	int protocol   = atoi(argv[5]);	 /*0:MSI, 1:MESI, 2:Dragon*/
	char *fname =  (char *)malloc(20);
 	fname = argv[6];

    //******************************************************//
    //**printf("===== 506 Personal information =====\n");***//
    //*******print out your personal information here*******//
    //*******according to the format in validation runs*****//
    //******************************************************//
	
	//****************************************************//
	//**printf("===== Simulator configuration =====\n");**//
	//*******print out simulator configuration here*******//
	//****************************************************//

 
	//*********************************************//
    //*****create an array of caches here**********//
	//*********************************************//	

	Cache **check1 = new Cache*[4];
	int i;
	for(i=0;i<4;i++){
		check1[i] = new Cache(cache_size,cache_assoc,blk_size);
	}

	ulong proc,addr;
	int j,x1,j1=0,k1=0;
	uchar op;
	char x[8];	
	pFile = fopen (fname,"r");

	if(pFile == 0)
	{   
		printf("Trace file problem\n");
		exit(0);
	}
	else{
		while(fscanf(pFile,"%lu %c %s",&proc,&op,x) != EOF){
			//break;
			addr = strtoull(x,NULL,16);
			//DRAGON
			x1 = 0;
			j1 = 0;
			for(i=0;i<4;i++){
				if(i != proc){
					x1 = check1[i]->CHKDRAGON(addr,x1);
				}
			}
			//cout << x1 << endl;
			check1[proc]->DRAGON(x1,addr,op);
			for(i=0;i<4;i++){
				if(i != proc){
					check1[i]->snoop_dragon(check1[proc]->getbusrd(),check1[proc]->getbusupd(),addr);
				}
			}
			//if(j1 != 0) check1[proc]->cache_opt();
			check1[proc]->setbusrd(0);
			check1[proc]->setbusupd(0);
			//MESI
			/*x1 = 0;
			j1 = 0;
			k1 = 0;
			for(i=0;i<4;i++){
				if(i != proc){
					x1 = check1[i]->CHKMESI(addr,x1);
				}
			}
			//cout << x1 << endl;
			check1[proc]->MESI(x1,addr,op);
			for(i=0;i<4;i++){
				if(i != proc){
					j1 = check1[i]->snoop_mesi(j1,check1[proc]->getbus(),addr);
					k1 = check1[i]->snoop_m2(k1,check1[proc]->getbus(),addr);
				}
			}
			if(j1 != 0) check1[proc]->cache_opt();
			if(k1 != 0 ) check1[proc]->mem_opt();
			check1[proc]->setbus(0);*/

			//printf("%x\n",addr);
			//cout << proc << " " << op << " " << addr << endl;
			//MSI PROTOCOL
			/*check1[proc]->MSI(addr,op);
			for(i=0;i<4;i++){
				if(i != proc){
					check1[i]->snoop_msi(check1[proc]->getbus(),addr);
				}
			}
			check1[proc]->setbus(0);*/
		}

	}
	///******************************************************************//
	//**read trace file,line by line,each(processor#,operation,address)**//
	//*****propagate each request down through memory hierarchy**********//
	//*****by calling cachesArray[processor#]->Access(...)***************//
	///******************************************************************//
	fclose(pFile);

	//********************************//
	//print out all caches' statistics //
	//********************************//

	for(i=0;i<4;i++){
			check1[i]->msi_printStats();
			//cout << (1!=3) << endl;	
	}
	
}
