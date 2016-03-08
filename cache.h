/*******************************************************
                          cache.h
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#ifndef CACHE_H
#define CACHE_H

#include <cmath>
#include <iostream>

typedef unsigned long ulong;
typedef unsigned char uchar;
typedef unsigned int uint;

/****add new states, based on the protocol****/
enum{
	INVALID = 0,
	VALID=1,
	DIRTY=2,
	EXCLUSIVE = 3,
	SHAREDCLEAN = 4,
	SHAREDMODIFIED = 5
};

class cacheLine 
{
protected:
   ulong tag;
   ulong Flags;   // 0:invalid, 1:valid, 2:dirty 
   ulong seq; 
 
public:
   cacheLine()            { tag = 0; Flags = 0; }
   ulong getTag()         { return tag; }
   ulong getFlags()			{ return Flags;}
   ulong getSeq()         { return seq; }
   void setSeq(ulong Seq)			{ seq = Seq;}
   void setFlags(ulong flags)			{  Flags = flags;}
   void setTag(ulong a)   { tag = a; }
   void invalidate()      { tag = 0; Flags = INVALID; }//useful function
   bool isValid()         { return ((Flags) != INVALID); }
};

class Cache
{
protected:
   ulong size, lineSize, assoc, sets, log2Sets, log2Blk, tagMask, numLines;
   ulong reads,readMisses,writes,writeMisses,writeBacks;
   ulong bus,mem,busrd,busupd;
   ulong invalid,intervention,flush,flushopt;
   //******///
   //add coherence counters here///
   //******///

   cacheLine **cache;
   ulong calcTag(ulong addr)     { return (addr >> (log2Blk) );}
   ulong calcIndex(ulong addr)  { return ((addr >> log2Blk) & tagMask);}
   ulong calcAddr4Tag(ulong tag)   { return (tag << (log2Blk));}
   
public:
    ulong currentCycle;  
     
    Cache(int,int,int);
   ~Cache() { delete cache;}
   
   cacheLine *findLineToReplace(ulong addr);
   cacheLine *fillLine(ulong addr);
   cacheLine *dragon_fillLine(ulong addr);
   cacheLine *mesi_fillLine(ulong addr);
   cacheLine * findLine(ulong addr);
   cacheLine * getLRU(ulong);
   
   ulong getRM(){return readMisses;} ulong getWM(){return writeMisses;} 
   ulong getReads(){return reads;}ulong getWrites(){return writes;}
   ulong getWB(){return writeBacks;}
   
   void mesi_writeBack(ulong)   {writeBacks++;}
   void writeBack(ulong)   {writeBacks++;}
   void dragon_writeBack(ulong)   {mem++; writeBacks++;}
   void Access(ulong,uchar);
   void msi_printStats();
   void updateLRU(cacheLine *);
   void MSI(ulong,uchar);
   //void MSI_Snoop(ulong,ulong,ulong);
   void setbus(ulong);
   ulong getbus();
   void snoop_msi(ulong,ulong);
   void cache_opt();
   void mem_opt();
   ulong CHKMESI(ulong,ulong);
   void MESI(ulong,ulong,uchar);
   int snoop_mesi(int,ulong,ulong);
   int snoop_m2(int,ulong,ulong);
   ulong CHKDRAGON(ulong addr,ulong x);
   void DRAGON(ulong,ulong,uchar);
   void setbusrd(ulong);
   void setbusupd(ulong);
   ulong getbusrd();
   ulong getbusupd();
   void snoop_dragon(ulong,ulong,ulong);
   ulong CHKSM(ulong,ulong);
   //******///
   //add other functions to handle bus transactions///
   //******///

};

#endif
