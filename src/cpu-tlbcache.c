/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee 
 * a personal to use and modify the Licensed Source Code for 
 * the sole purpose of studying during attending the course CO2018.
 */
//#ifdef MM_TLB
/*
 * Memory physical based TLB Cache
 * TLB cache module tlb/tlbcache.c
 *
 * TLB cache is physically memory phy
 * supports random access 
 * and runs at high speed
 */


#include "mm.h"
#include <stdlib.h>
#include <stdio.h>

#define init_tlbcache(mp,sz,...) init_memphy(mp, sz, (1, ##__VA_ARGS__))

/*
 *  tlb_cache_read read TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */

/* Groups' define memphy_struct TLB:
 *    + maxsz will be the size of the TLB
 *    + storage will be 
 *    + rmdflg will always be 1
 *    + used_fp_list->owner->pgd[pgnum] = frmnum
 *    + 
*/

int tlb_cache_read(struct memphy_struct * mp, int pid, int pgnum, int *value)
{
   /* TODO: the identify info is mapped to 
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    */

   /* Our group's code */
   /* We implement direct mapping technique for mapping */
   if(!mp) return -1;   

   int TLB_SIZE = mp->used_fp_list->owner->pgd_size;
   if(mp->used_fp_list->owner->pgd[pgnum % TLB_SIZE] == pgnum
   && mp->used_fp_list->owner->pidd[pgnum % TLB_SIZE] == pid) {
      /* If TLB hit */
      *value = mp->used_fp_list->owner->frmnumd[pgnum % TLB_SIZE];
      return 0;
   }

   *value = -1;
   /* Our group's code */

   return 0;
}

/*
 *  tlb_cache_write write TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */
int tlb_cache_write(struct memphy_struct *mp, int pid, int pgnum, int value)
{
   /* TODO: the identify info is mapped to 
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    */

   /* Our group's code */
   /*
    * tlb_cache_write will write pid and frame number into
    * the pgnum % TLB_SIZE position
   */
   if(!mp) return -1;
   
   int TLB_SIZE = mp->used_fp_list->owner->pgd_size;
   mp->used_fp_list->owner->frmnumd[pgnum % TLB_SIZE] = value;
   mp->used_fp_list->owner->pidd[pgnum % TLB_SIZE] = pid;   
   /* Our group's code */
   return 0;
}

/*
 *  TLBMEMPHY_read natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @value: obtained value
 */
int TLBMEMPHY_read(struct memphy_struct * mp, int addr, BYTE *value)
{
   if (mp == NULL)
     return -1;

   /* TLB cached is random access by native */
   *value = mp->storage[addr];

   return 0;
}


/*
 *  TLBMEMPHY_write natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data
 */
int TLBMEMPHY_write(struct memphy_struct * mp, int addr, BYTE data)
{
   if (mp == NULL)
     return -1;

   /* TLB cached is random access by native */
   mp->storage[addr] = data;

   return 0;
}

   /*
   *  TLBMEMPHY_format natively supports MEMPHY device interfaces
   *  @mp: memphy struct
   */


int TLBMEMPHY_dump(struct memphy_struct * mp)
{
   /*TODO dump memphy contnt mp->storage 
    *     for tracing the memory content
    */

   /* Our group's code */
   if(!mp || !mp->storage) {
      return -1;
   }

   int i;
   for(i = 0; i < mp->maxsz; i ++) {
      printf("%c\n", mp->storage[i]);
   }
   printf("\n");
   /* Our group's code */

   return 0;
}


/*
 *  Init TLBMEMPHY struct
 */
int init_tlbmemphy(struct memphy_struct *mp, int max_size)
{
   mp->storage = (BYTE *)malloc(max_size*sizeof(BYTE));
   mp->maxsz = max_size;

   /* Our group's code */
   mp->used_fp_list->owner->pgd_size = max_size;
   mp->used_fp_list->owner->pgd = malloc(max_size*sizeof(uint32_t));
   mp->used_fp_list->owner->pidd = malloc(max_size*sizeof(uint32_t));
   mp->used_fp_list->owner->frmnumd = malloc(max_size*sizeof(uint32_t));   

   int iter = 0;
   for(; iter < max_size; iter ++) {
      mp->used_fp_list->owner->pgd[iter] = -1;
      mp->used_fp_list->owner->pidd[iter] = -1;
      mp->used_fp_list->owner->frmnumd[iter] = -1;
   }

   /* This setting come with fixed constant PAGESZ */
   int numfp = max_size;
   struct framephy_struct *newfst, *fst;   

   if (numfp <= 0) {
      return -1;
   }

   /* Init head of free framephy list */ 
   fst = malloc(sizeof(struct framephy_struct));
   fst->fpn = 0;
   mp->used_fp_list = fst;

   /* We have list with first element, fill in the rest num-1 element member*/
   for (iter = 1; iter < numfp ; iter++)
   {
      newfst =  malloc(sizeof(struct framephy_struct));
      newfst->fpn = iter;
      newfst->fp_next = NULL;
      fst->fp_next = newfst;
      fst = newfst;
   }
   /* Our group's code */

   mp->rdmflg = 1;

   return 0;
}

//#endif
