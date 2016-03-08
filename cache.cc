/*******************************************************
                          cache.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include "cache.h"
using namespace std;

Cache::Cache(int s,int a,int b )
{
   ulong i, j;
   reads = readMisses = writes = 0; 
   writeMisses = writeBacks = currentCycle = bus = flush = invalid = intervention = flushopt = mem = 0;

   size       = (ulong)(s);
   lineSize   = (ulong)(b);
   assoc      = (ulong)(a);   
   sets       = (ulong)((s/b)/a);
   numLines   = (ulong)(s/b);
   log2Sets   = (ulong)(log2(sets));   
   log2Blk    = (ulong)(log2(b));   
  
   //*******************//
   //initialize your counters here//
   //*******************//
 
   tagMask =0;
   for(i=0;i<log2Sets;i++)
   {
		tagMask <<= 1;
        tagMask |= 1;
   }
   
   /**create a two dimentional cache, sized as cache[sets][assoc]**/ 
   cache = new cacheLine*[sets];
   for(i=0; i<sets; i++)
   {
      cache[i] = new cacheLine[assoc];
      for(j=0; j<assoc; j++) 
      {
	   cache[i][j].invalidate();
      }
   }      
   
}

/**you might add other parameters to Access()
since this function is an entry point 
to the memory hierarchy (i.e. caches)**/
void Cache::Access(ulong addr,uchar op)
{
	currentCycle++;/*per cache global counter to maintain LRU order 
			among cache ways, updated on every cache access*/
        	
	if(op == 'w') writes++;
	else          reads++;
	
	cacheLine * line = findLine(addr);
	if(line == NULL)/*miss*/
	{
		if(op == 'w') writeMisses++;
		else readMisses++;

		cacheLine *newline = fillLine(addr);
   		if(op == 'w') newline->setFlags(DIRTY);    
		
	}
	else
	{
		/**since it's a hit, update LRU and update dirty flag**/
		updateLRU(line);
		if(op == 'w') line->setFlags(DIRTY);
	}
}

ulong Cache::CHKDRAGON(ulong addr,ulong x){
	 ulong i, j, tag, pos;
   
   	pos = assoc;
   	tag = calcTag(addr);
  	 i   = calcIndex(addr);
  
   	for(j=0; j<assoc; j++){
		if(cache[i][j].isValid()){
	    	    if(cache[i][j].getTag() == tag)
			{
			      x++;
			}
		}
	}
	return x;
}

ulong Cache::CHKSM(ulong addr,ulong x){
	 ulong i, j, tag, pos;
   
   	pos = assoc;
   	tag = calcTag(addr);
  	 i   = calcIndex(addr);
  
   	for(j=0; j<assoc; j++){
		if(cache[i][j].isValid()){
	    	    if(cache[i][j].getTag() == tag)
			{
				if(cache[i][j].getFlags() == 5)
					x++;
			}
		}
	}
	return x;
}

void Cache::DRAGON(ulong copy,ulong addr,uchar op){
	currentCycle++;
	
	if(op == 'w') writes++;
	else          reads++;
	
	cacheLine * line = findLine(addr);
	if(line == NULL)/*miss*/
	{
		if(op == 'w'){
			 writeMisses++;
		}
		else{
		 	readMisses++;
		}
		cacheLine *newline = dragon_fillLine(addr);
		   	
		if(op == 'w'){
			 if(copy == 0){
				newline->setFlags(2);
				setbusrd(1);
				mem++;
			}
			else{
				newline->setFlags(5);
				setbusrd(1);
				//flushopt++;
				mem++;
				setbusupd(1);
			}
		}
		if(op == 'r'){
			if(copy == 0){
				newline->setFlags(EXCLUSIVE);
				setbusrd(1);
				mem++;
			}
			else{
				newline->setFlags(4);
				setbusrd(1);
				//flushopt++;
				mem++;
			}
		}    
	}
	else
	{
		/**since it's a hit, update LRU and update dirty flag**/
		updateLRU(line);
		if(op == 'w'){
			if(line->getFlags() == 3){
				line->setFlags(2);
			}
			if(line->getFlags() == 4){
				if(copy == 0){
					line->setFlags(2);
					setbusupd(1);
				}
				else{
					line->setFlags(5);
					setbusupd(1);
				}
			}	
			if(line->getFlags() == 5){
				if(copy == 0){
					line->setFlags(2);
					setbusupd(1);
				}
				else{
					setbusupd(1);
				}
			}
		}
	}
	
}

void Cache::snoop_dragon(ulong busrd,ulong busupd,ulong addr){
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
 
   for(j=0; j<assoc; j++){
	if(cache[i][j].isValid()){
	        if(cache[i][j].getTag() == tag)
		{
		     if(busrd == 1){
			if(cache[i][j].getFlags() == 3){
				cache[i][j].setFlags(4);
				intervention++;
			}
			if(cache[i][j].getFlags() == 5){
				flush++;
			}
			if(cache[i][j].getFlags() == 2){
				cache[i][j].setFlags(5);
				intervention++;
				flush++;
			}
		     }
		     if(busupd == 1){
			if(cache[i][j].getFlags()  == 5){
				cache[i][j].setFlags(4);
			}
		     }
		     break; 
		}
	}
   }
}


void Cache::setbusrd(ulong x){
	busrd = x;
}

void Cache::setbusupd(ulong x){
	busupd = x;
}

ulong Cache::getbusrd(){
	return busrd;
}

ulong Cache::getbusupd(){
	return busupd;
}


ulong Cache::CHKMESI(ulong addr,ulong x){
	 ulong i, j, tag, pos;
   
   	pos = assoc;
   	tag = calcTag(addr);
  	 i   = calcIndex(addr);
  
   	for(j=0; j<assoc; j++){
		if(cache[i][j].isValid()){
	    	    if(cache[i][j].getTag() == tag)
			{
			      x++;
			}
		}
	}
	return x;
}

void Cache::MESI(ulong copy,ulong addr,uchar op){
	currentCycle++;
	
	if(op == 'w') writes++;
	else          reads++;
	
	cacheLine * line = findLine(addr);
	if(line == NULL)/*miss*/
	{
		if(op == 'w'){
			 writeMisses++;
			 setbus(2); 
		}
		else{
		 	readMisses++;
			setbus(1);
		}
		cacheLine *newline = mesi_fillLine(addr);
		   	
		if(op == 'w'){
			 newline->setFlags(DIRTY);
			 //mem++;
		}
		if(op == 'r'){
			if(copy == 0){
				newline->setFlags(EXCLUSIVE);
				//mem++;
			}
			else{
				newline->setFlags(VALID);
			}
		}    
	}
	else
	{
		/**since it's a hit, update LRU and update dirty flag**/
		updateLRU(line);
		if(op == 'w'){
			if(line->getFlags() == 1){
				setbus(3);
				line->setFlags(DIRTY);	
			}
			if(line->getFlags() == 3){
				line->setFlags(DIRTY);
			}			
			
		}
	}
	
}

int Cache::snoop_mesi(int dummy,ulong bus,ulong addr){
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
 
   for(j=0; j<assoc; j++){
	if(cache[i][j].isValid()){
	        if(cache[i][j].getTag() == tag)
		{
		     if((cache[i][j].getFlags() == 1) && (bus == 2)){
			//cache[i][j].setFlags(INVALID);
			dummy++;
			invalid++;
			//mem++;
		     }
		     if((cache[i][j].getFlags() == 1) && (bus == 3)){
			//cache[i][j].setFlags(INVALID);
			//mem++;			
			invalid++;
		     }      
		     if((cache[i][j].getFlags() == 1) && (bus == 1)){
			dummy++;
			//mem++;
		     }      
		     if((cache[i][j].getFlags() == 2) && (bus == 1)){
			//cache[i][j].setFlags(VALID);
			flush++;
			//mem++;
			writeBacks++;
			dummy++;
			intervention++;
		     }    
		     if((cache[i][j].getFlags() == 2) && (bus == 2)){ //CORRECT HERE
			//cache[i][j].setFlags(INVALID);
			invalid++;
			flush++;
			dummy++;
			writeBacks++;
			//mem++;
		     }    
		     if((cache[i][j].getFlags() == 3) && (bus == 1)){ //Copt1
			//cache[i][j].setFlags(VALID);
			dummy++;
			//mem++;
			intervention++;
		     }    
		     if((cache[i][j].getFlags() == 3) && (bus == 2)){
			//cache[i][j].setFlags(INVALID);
			dummy++;
			invalid++;
			//mem++;
		     }    
		     break; 
		}
	}
   }
   return dummy;
}

int Cache::snoop_m2(int dummy,ulong bus,ulong addr){
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
 
   for(j=0; j<assoc; j++){
	if(cache[i][j].isValid()){
	        if(cache[i][j].getTag() == tag)
		{
		     if((cache[i][j].getFlags() == 1) && (bus == 2)){
			cache[i][j].setFlags(INVALID);
			//dummy++;
			//invalid++;
			//mem++;
		     }
		     if((cache[i][j].getFlags() == 1) && (bus == 3)){
			cache[i][j].setFlags(INVALID);
			//dummy++;			
			//invalid++;
		     }      
		     if((cache[i][j].getFlags() == 1) && (bus == 1)){
			//dummy++;
			//mem++;
		     }      
		     if((cache[i][j].getFlags() == 2) && (bus == 1)){
			cache[i][j].setFlags(VALID);
			flush++;
			//mem++;
			//writeBacks++;
			//dummy++;
			//intervention++;
		     }    
		     if((cache[i][j].getFlags() == 2) && (bus == 2)){ //CORRECT HERE
			cache[i][j].setFlags(INVALID);
			//invalid++;
			//flush++;
			//dummy++;
			//writeBacks++;
			//mem++;
		     }    
		     if((cache[i][j].getFlags() == 3) && (bus == 1)){ //Copt1
			cache[i][j].setFlags(VALID);
			//dummy++;
			//mem++;
			//intervention++;
		     }    
		     if((cache[i][j].getFlags() == 3) && (bus == 2)){
			cache[i][j].setFlags(INVALID);
			//dummy++;
			//invalid++;
			//mem++;
		     }    
		     break; 
		}
	}
   }
   return dummy;
}

void Cache::cache_opt(){
		flushopt++;
}

void Cache::mem_opt(){
		mem++;
}

void Cache::MSI(ulong addr,uchar op){
	currentCycle++;
	
	if(op == 'w') writes++;
	else          reads++;
	
	cacheLine * line = findLine(addr);
	if(line == NULL)/*miss*/
	{
		if(op == 'w'){
			 writeMisses++;
			 setbus(2); 
			 mem++;
		}
		else{
		 	readMisses++;
			setbus(1);
			mem++;
		}
		cacheLine *newline = mesi_fillLine(addr);
		   	
		if(op == 'w') newline->setFlags(DIRTY);    
		
	}
	else
	{
		/**since it's a hit, update LRU and update dirty flag**/
		updateLRU(line);
		if(op == 'w'){
			if(line->getFlags() == 1){
				setbus(2);
				mem++;
			}		
			line->setFlags(DIRTY);	
		}
	}
	
}

void Cache::snoop_msi(ulong bus,ulong addr){
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);

   for(j=0; j<assoc; j++){
	if(cache[i][j].isValid()){
	        if(cache[i][j].getTag() == tag)
		{
		     if((bus == 1) && (cache[i][j].getFlags() == 2)){
			cache[i][j].setFlags(1);
			intervention++;
			mem++;
			flush++;
			writeBacks++;
		     }
		     if(bus == 2){
			if(cache[i][j].getFlags() != 0){
				if(cache[i][j].getFlags() == 2)
				{
					flush++;
					mem++;
					writeBacks++;
				}			
				cache[i][j].setFlags(0);
				invalid++;
			}
		     }		     
		     break; 
		}
	}
   }
}

void Cache::setbus(ulong val){
	bus = val;
}

ulong Cache::getbus(){
	return bus;
}

/*look up line*/
cacheLine * Cache::findLine(ulong addr)
{
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
  
   for(j=0; j<assoc; j++)
	if(cache[i][j].isValid())
	        if(cache[i][j].getTag() == tag)
		{
		     pos = j; break; 
		}
   if(pos == assoc)
	return NULL;
   else
	return &(cache[i][pos]); 
}

/*upgrade LRU line to be MRU line*/
void Cache::updateLRU(cacheLine *line)
{
  line->setSeq(currentCycle);  
}

/*return an invalid line as LRU, if any, otherwise return LRU line*/
cacheLine * Cache::getLRU(ulong addr)
{
   ulong i, j, victim, min;

   victim = assoc;
   min    = currentCycle;
   i      = calcIndex(addr);
   
   for(j=0;j<assoc;j++)
   {
      if(cache[i][j].isValid() == 0) return &(cache[i][j]);     
   }   
   for(j=0;j<assoc;j++)
   {
	 if(cache[i][j].getSeq() <= min) { victim = j; min = cache[i][j].getSeq();}
   } 
   assert(victim != assoc);
   
   return &(cache[i][victim]);
}

/*find a victim, move it to MRU position*/
cacheLine *Cache::findLineToReplace(ulong addr)
{
   cacheLine * victim = getLRU(addr);
   updateLRU(victim);
  
   return (victim);
}

/*allocate a new line*/
cacheLine *Cache::fillLine(ulong addr)
{ 
   ulong tag;
  
   cacheLine *victim = findLineToReplace(addr);
   assert(victim != 0);
   if(victim->getFlags() == DIRTY) writeBack(addr);
    	
   tag = calcTag(addr);   
   victim->setTag(tag);
   victim->setFlags(VALID);    
   /**note that this cache line has been already 
      upgraded to MRU in the previous function (findLineToReplace)**/

   return victim;
}

cacheLine *Cache::dragon_fillLine(ulong addr)
{ 
   ulong tag;
  
   cacheLine *victim = findLineToReplace(addr);
   assert(victim != 0);
   if(victim->getFlags() == DIRTY || victim->getFlags() == 5) dragon_writeBack(addr);
    	
   tag = calcTag(addr);   
   victim->setTag(tag);
   victim->setFlags(VALID);    
   /**note that this cache line has been already 
      upgraded to MRU in the previous function (findLineToReplace)**/

   return victim;
}

cacheLine *Cache::mesi_fillLine(ulong addr)
{ 
   ulong tag;
  
   cacheLine *victim = findLineToReplace(addr);
   assert(victim != 0);
   if(victim->getFlags() == DIRTY) mesi_writeBack(addr);
    	
   tag = calcTag(addr);   
   victim->setTag(tag);
   victim->setFlags(VALID);    
   /**note that this cache line has been already 
      upgraded to MRU in the previous function (findLineToReplace)**/

   return victim;
}

void Cache::msi_printStats()
{ 
	printf("===== Simulation results   =====\n");
	cout << readMisses << " " << writeMisses << " " << writeBacks << " " << intervention << " "  << flush << " " << mem << " " << flushopt << " " << reads << " " << writes <<  endl;
	/****print out the rest of statistics here.****/
	/****follow the ouput file format**************/
}
