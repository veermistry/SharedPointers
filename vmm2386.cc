#include "ide.h"
#include "ext2.h"
#include "shared.h"
#include "libk.h"
#include "vmm.h"
#include "config.h"
#include "future.h"

void kernelMain(void) {

    using namespace VMM;

    // Create a new Ide object representing the first IDE device
    auto ide = Shared<Ide>::make(1);
    
    auto fs = Shared<Ext2>::make(ide);
    
    auto root = fs->root;

    // Find the file "idk.txt" in the file system and map it into memory
    auto idk = fs->find(root,"idk.txt");
    uint32_t* x = (uint32_t*) naive_mmap(idk->size_in_bytes(),idk,0);

    Debug::printf("*** x == %x\n", x);

    Debug::printf("%s\n",x);

    // Create a new thread to perform an asynchronous computation
    auto thread = future<bool>([x] {

        // Map a single page of memory into the thread's address space
        uint32_t* val = (uint32_t*) naive_mmap(1,Shared<Node>{},0);
        Debug::printf("*** val == %x\n", val);

        return true;
    });

    // Unmap the file that was previously mapped into memory
    naive_munmap((void*)x);

    thread->get();
}
