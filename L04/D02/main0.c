/******************************************************************************
 * Lab 04 - Exercise 2                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#include "monitor.h"
#include "descriptor_tables.h"
#include "paging.h"

#define PAGE_SIZ 0x1000 /* Each page is 4KB */

int main(struct multiboot *mboot_ptr)
{
    u32int i, data, *ptr;
    
    /* Initialize all the ISRs and segmentation */
    init_descriptor_tables();
    /* Initialize paging */
    init_paging();
    /* Clear the screen */
    monitor_clear();
    
    /* Access pages until a page fault occurs */
    for (i = 0, ptr = 0; ; i++, ptr += PAGE_SIZ >> 2)
    {
        data = *ptr;
        monitor_write("Normal access at address ");
        monitor_write_hex((u32int)ptr);
        monitor_write(" at page ");
        monitor_write_dec(i);
        monitor_write("\n");
    }

    return 0;
}
