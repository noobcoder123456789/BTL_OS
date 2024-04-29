/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee 
 * a personal to use and modify the Licensed Source Code for 
 * the sole purpose of studying during attending the course CO2018.
 */
//#ifdef CPU_TLB
/*
 * CPU TLB
 * TLB module cpu/cpu-tlb.c
 */
 
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>

int tlb_change_all_page_tables_of(struct pcb_t *proc,  struct memphy_struct * mp)
{
  /* TODO update all page table directory info 
   *      in flush or wipe TLB (if needed)
   */

  /* Our group's code */
  if(!mp || !proc) {
    return -1;
  }

  int i;
  for(i = 0; i < (1 << FIRST_LV_LEN); i ++) {
    if(!proc->page_table->table[i].next_lv) {
      continue;
    }

    int j;
    for(j = 0; j < (1 << SECOND_LV_LEN); j++) {
      /* assign the virtual index at first layer to all second layer member*/
      proc->page_table->table[i].next_lv->table[j].v_index = proc->page_table->table[i].v_index;
    }
  }

  tlb_flush_tlb_of(proc, mp);
  /* Our group's code */
  
  return 0;
}

int tlb_flush_tlb_of(struct pcb_t *proc, struct memphy_struct * mp)
{
  /* TODO flush tlb cached*/

  /* Our group's code */
  if(!proc || !proc->tlb) {
    return -1;
  }

  mp = proc->tlb;

  int i;
  for(i = 0; i < mp->used_fp_list->owner->pgd_size; i ++) {
    mp->used_fp_list->owner->pgd[i] = -1;
    mp->used_fp_list->owner->pidd[i] = -1;
    mp->used_fp_list->owner->frmnumd[i] = -1;
  }
  /* Our group's code */

  return 0;
}

/*tlballoc - CPU TLB-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int addr, val;

  /* By default using vmaid = 0 */
  val = __alloc(proc, 0, reg_index, size, &addr);

  /* TODO update TLB CACHED frame num of the new allocated page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  /* Our group's code */
  if(val + 1 == 0) {
    return val;
  }
  
  /* If alloc succeed then we put it into TLB */
  int TLB_SIZE = proc->tlb->used_fp_list->owner->pgd_size;
  int pgn = PAGING_PGN(addr);
  int fpn = PAGING_FPN(proc->tlb->used_fp_list->owner->pgd[pgn % TLB_SIZE]);
  if(tlb_cache_write(proc->tlb, proc->pid, pgn, fpn) < 0) {
    return -1;
  }
  /* Our group's code */

  return val;
}

/*pgfree - CPU TLB-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlbfree_data(struct pcb_t *proc, uint32_t reg_index)
{
  __free(proc, 0, reg_index);

  /* TODO update TLB CACHED frame num of freed page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  /* Our group's code */
  int frmnum = -1;
  int TLB_SIZE = proc->tlb->used_fp_list->owner->pgd_size;
  struct vm_rg_struct *currg = get_symrg_byid(proc->tlb->used_fp_list->owner, reg_index);
  int addr = currg->rg_start;
  int pgn = PAGING_PGN(addr);
  tlb_cache_read(proc->tlb, proc->pid, pgn, &frmnum);

  if(frmnum != -1) {
    /* If TLB hit then free that TLB entry */
    proc->tlb->used_fp_list->owner->frmnumd[pgn % TLB_SIZE] = -1;
    proc->tlb->used_fp_list->owner->pidd[pgn % TLB_SIZE] = -1;
  }
  /* Our group's code */

  return 0;
}


/*tlbread - CPU TLB-based read a region memory
 *@proc: Process executing the instruction
 *@source: index of source register
 *@offset: source address = [source] + [offset]
 *@destination: destination storage
 */
int tlbread(struct pcb_t * proc, uint32_t source,
            uint32_t offset, 	uint32_t *destination) 
{
  /* 
   * we just add * for destination because
   * we think that we need the return value after 
   * reading the TLB cache
   */
  BYTE data;
  int frmnum = -1;
	
  /* TODO retrieve TLB CACHED frame num of accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  /* frmnum is return value of tlb_cache_read/write value*/

  /* Our group's code */
  /* hình như source là region id */
  int TLB_SIZE = proc->tlb->used_fp_list->owner->pgd_size;
  struct vm_rg_struct *currg = get_symrg_byid(proc->tlb->used_fp_list->owner, source);
  int addr = currg->rg_start + offset;
  int pgn = PAGING_PGN(addr);
  tlb_cache_read(proc->tlb, proc->pid, pgn, &frmnum);
  /* Our group's code */
	
#ifdef IODUMP
  if (frmnum >= 0)
    printf("TLB hit at read region=%d offset=%d\n", 
	         source, offset);
  else 
    printf("TLB miss at read region=%d offset=%d\n", 
	         source, offset);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  int val = __read(proc, 0, source, offset, &data);

  *destination = (uint32_t) data;

  /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  /* Our group's code */
  if(frmnum < 0) {
    /* 
     * If the frame num of recent accessing page(s) is not present in TLB 
     * then put it in TLB by direct mapping technique
     */    

    if(pg_getpage(proc->mm, pgn, &frmnum, proc) != 0) {
      /* Get the page to MEMRAM, swap from MEMSWAP if needed */
      return -1; /* invalid page access */
    }  

    proc->tlb->used_fp_list->owner->pidd[source % TLB_SIZE] = proc->pid;
    proc->tlb->used_fp_list->owner->pgd[source % TLB_SIZE] = pgn;
    proc->tlb->used_fp_list->owner->frmnumd[source % TLB_SIZE] = frmnum;
  }
  /* Our group's code */

  return val;
}

/*tlbwrite - CPU TLB-based write a region memory
 *@proc: Process executing the instruction
 *@data: data to be wrttien into memory
 *@destination: index of destination register
 *@offset: destination address = [destination] + [offset]
 */
int tlbwrite(struct pcb_t * proc, BYTE data,
             uint32_t destination, uint32_t offset)
{
  int val, frmnum = -1;

  /* TODO retrieve TLB CACHED frame num of accessing page(s))*/
  /* by using tlb_cache_read()/tlb_cache_write()
  frmnum is return value of tlb_cache_read/write value*/

  /* Our group's code */
  int TLB_SIZE = proc->tlb->used_fp_list->owner->pgd_size;
  struct vm_rg_struct *currg = get_symrg_byid(proc->tlb->used_fp_list->owner, destination);
  int addr = currg->rg_start + offset;
  int pgn = PAGING_PGN(addr);
  tlb_cache_read(proc->tlb, proc->pid, pgn, &frmnum);
  /* Our group's code */

#ifdef IODUMP
  if (frmnum >= 0)
    printf("TLB hit at write region=%d offset=%d value=%d\n",
	          destination, offset, data);
	else
    printf("TLB miss at write region=%d offset=%d value=%d\n",
            destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  val = __write(proc, 0, destination, offset, data);

  /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  /* Our group's code */
  if(frmnum < 0) {
    /* 
     * If the frame num of recent accessing page(s) is not present in TLB 
     * then put it in TLB by direct mapping technique
     */    

    if(pg_getpage(proc->mm, pgn, &frmnum, proc) != 0) {
      /* Get the page to MEMRAM, swap from MEMSWAP if needed */
      return -1; /* invalid page access */
    }  

    proc->tlb->used_fp_list->owner->pidd[destination % TLB_SIZE] = proc->pid;
    proc->tlb->used_fp_list->owner->pgd[destination % TLB_SIZE] = pgn;
    proc->tlb->used_fp_list->owner->frmnumd[destination % TLB_SIZE] = frmnum;
  }
  /* Our group's code */

  return val;
}

//#endif
