/******************************************************************************
 * Lab 04 - Exercise 3                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#include "monitor.h"
#include "descriptor_tables.h"
#include "paging.h"

#define PAGE_SIZ 0x1000 /* Each page is 4KB */
#define PAGE_NUM 264    /* 264 pages allocated */
#define PAGE_LIM 160    /* Access limited to 0xA0000 */

void page_print(u32int *ptr)
{
    u32int data;

    /* Read page contents */
    data = *ptr;

    /* Print page contents */
    monitor_write("Ptr ");
    monitor_write_hex((u32int)ptr);
    monitor_write(" (page ");
    monitor_write_dec((u32int)ptr/PAGE_SIZ);
    monitor_write(") contains ");
    monitor_write_dec(data);
    monitor_write("\n");
}

int main(struct multiboot *mboot_ptr)
{
    u32int i, data, *ptr;
    
    /* Initialize all the ISRs and segmentation */
    init_descriptor_tables();
    /* Initialize paging */
    init_paging();
    /* Clear the screen */
    monitor_clear();
    
    /* Write on some pages and read back the value */
    for (i = 0, ptr = 0; i < PAGE_LIM; i++, ptr += PAGE_SIZ >> 2)
    {
        /* Write on a page */
        *ptr = i;

        /* Print page contents */
        page_print(ptr);
    }

    /* Swap the contents of pages 0 and 1 */
    monitor_write("\nSwapping contents of pages 0 and 1:\n");
    page_swap(0, 1);
    page_print((u32int*)0);
    page_print((u32int*)PAGE_SIZ);

    return 0;
}
