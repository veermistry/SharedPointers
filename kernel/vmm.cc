#include "vmm.h"
#include "machine.h"
#include "idt.h"
#include "libk.h"
#include "blocking_lock.h"
#include "config.h"
#include "threads.h"
#include "debug.h"
#include "ext2.h"
#include "physmem.h"

//when offset == 0, read(va-start, ___, ___)
//else, read(va-start + offset, ____, ___)


namespace VMM {
    pd_space* global_page_directory = nullptr;
    using namespace gheith;
    auto mem_start = 0x80000000;

void global_init() {
    global_page_directory = new pd_space();
    for (uint32_t VA = 0; VA < kConfig.memSize; VA += 4096) {
        global_page_directory->map(VA, VA);
    }
    global_page_directory->map(kConfig.ioAPIC, kConfig.ioAPIC);
    global_page_directory->map(kConfig.localAPIC, kConfig.localAPIC);
}

void per_core_init() {
    using namespace gheith;
    vmm_on((uint32_t)gheith::current()->pd);
}

void naive_munmap(void* p_) {
     if( (uint32_t)p_ < 0x80000000 ) {
        return;
     }
    //loop thru linkedlist
    linked_list* list = gheith::current()->lis;
    Virtual_Node* found = list->find((uint32_t)p_);
    if (found->start == kConfig.ioAPIC || found->start == kConfig.localAPIC) {
        return;
    }
    invlpg((uint32_t)p_);

    if (list->head->start == 0xfec00000) {
        list->head = list->head->next;
    }

    gheith::current()->local->unmap((uint32_t)p_);
    list->remove(found);
}

void* naive_mmap(uint32_t sz_, Shared<Node> node, uint32_t offset_) {
    //just doing add
    linked_list* list = gheith::current()->lis;
    return list->add(sz_, node, offset_);

}

extern "C" void vmm_pageFault(uintptr_t va_, uintptr_t *saveState) {
    using namespace gheith;
    linked_list* list = gheith::current()->lis;

    if (va_ >= 0x0 && va_ <= 0x1000) {
        Debug::panic("*** can't handle page fault at %x\n",va_);
    }
    Virtual_Node* curr = list->find(va_);
    if (curr != nullptr && va_ >= 0x80000000) {
        uint32_t* frame = (uint32_t*)PhysMem::alloc_frame();
        uint32_t page_num = (va_ - curr->start) / 4096;
        if (curr->sharedNode != nullptr) {
            curr->sharedNode->read_all(page_num * 4096 + curr->offset, curr->size, (char *)frame);
        }
        gheith::current()->local->map(va_, (uint32_t)frame);
        return;
    }
    
    Debug::panic("*** can't handle page fault at %x\n",va_);
}

}
