#ifndef _VMM_H_
#define _VMM_H_

#include "stdint.h"
#include "shared.h"
#include "physmem.h"

class Node;

namespace VMM {

    struct pd_space{
        uint32_t* pd;

        pd_space(){
            pd = (uint32_t*)(PhysMem::alloc_frame());
        }

        void map(uint32_t va, uint32_t pa) {
            uint32_t pdi = va >> 22;
            uint32_t pti = va >> 12;
            pti = pti & 0x3FF;
            uint32_t* pt = nullptr;

            va = (va >> 12) << 12;

            if (!(pd[pdi] & 0x1)) {
                pd[pdi] = (pd[pdi] >> 12) << 12;
                pd[pdi] += 0x1;
                pt = (uint32_t*)PhysMem::alloc_frame();
                pd[pdi] |= (uint32_t)pt;
            } else {
                pt = (uint32_t*)((pd[pdi] >> 12) << 12);
                pd[pdi] |= (uint32_t)pt;
            }
            pt[pti] = pa;
            pt[pti] |= 0x1;
        }

        // void activate() {
        //     vmm_on((uint32_t)pd);
        // }

        void unmap(uint32_t va) {
            uint32_t pdi = va >> 22;
            uint32_t pti = va >> 12;
            pti = pti & 0x3FF;
            uint32_t* pt = nullptr;
            pt = (uint32_t*)((pd[pdi] >> 12) << 12);
            pt[pti] = 0;
            //invalidate page 
            //deconstruct linked list
            //ivlpg
        }
        

    };

    extern pd_space* global_page_directory;

    // Called (on the initial core) to initialize data structures, etc
    extern void global_init();

    // Called on each core to do per-core initialization
    extern void per_core_init();

    // naive mmap
    extern void* naive_mmap(uint32_t size, Shared<Node> file, uint32_t file_offset);

    // naive munmap
    void naive_munmap(void* p);

}

#endif
