#ifndef _shared_h_
#define _shared_h_

#include "debug.h"

template <typename T>
class Shared {
    T* ptr;

    // Decrease the reference count, delete the object if count becomes zero.
    void drop() {
        if (ptr != nullptr) {
            auto new_count = ptr->ref_count.add_fetch(-1);
            if (new_count == 0) {
                delete ptr;
                ptr = nullptr;
            }
        }
    }

    // Increase the reference count.
    void add() {
        if (ptr != nullptr) {
            ptr->ref_count.add_fetch(1);
        }
    }

public:

    // Construct from a raw pointer and increase the reference count.
    explicit Shared(T* it) : ptr(it) {
        add();
    }

    // Default constructor, creates a null Shared pointer.
    //
    // Shared<Thing> a{};
    //
    Shared(): ptr(nullptr) {

    }

    // Copy constructor, increase the reference count.
    //
    // Shared<Thing> b { a };
    // Shared<Thing> c = b;
    // f(b);
    // return c;
    //
    Shared(const Shared& rhs): ptr(rhs.ptr) {
        add();
    }

    // Move constructor, transfer ownership from rhs to this.
    //
    // Shared<Thing> d = g();
    //
    Shared(Shared&& rhs): ptr(rhs.ptr) {
        rhs.ptr = nullptr;
    }

    // Destructor, decrease the reference count and delete if necessary.
    ~Shared() {
        drop();
    }

    // Overloaded arrow operator to access members of the underlying object.
    // d->m();
    T* operator -> () const {
        return ptr;
    }

    // Overloaded assignment operator for raw pointers.
    // d = nullptr;
    // d = new Thing{};
    Shared<T>& operator=(T* rhs) {
        if (this->ptr != rhs) {
            drop();
            this->ptr = rhs;
            add();
        }
        return *this;
    }

    // Overloaded assignment operator for Shared pointers.
    // d = a;
    // d = Thing{};
    Shared<T>& operator=(const Shared<T>& rhs) {
        auto other_ptr = rhs.ptr;
        if (ptr != other_ptr) {
            drop();
            ptr = other_ptr;
            add();
        }
        return *this;
    }

    // Move assignment operator, transfer ownership from rhs to this.
    // d = g();
    Shared<T>& operator=(Shared<T>&& rhs) {
        drop();
        ptr = rhs.ptr;
        rhs.ptr = nullptr;
        
        return *this;
    }

    // Comparison for shared pointers
    bool operator==(const Shared<T>& rhs) const {
	    return ptr == rhs.ptr;
    }

    bool operator!=(const Shared<T>& rhs) const {
	    return ptr != rhs.ptr;
    }

    // Comparison for raw pointers
    bool operator==(T* rhs) {
        return ptr == rhs;
    }

    bool operator!=(T* rhs) {
        return ptr != rhs;
    }

    // Factory function to create a Shared pointer with a new object.
    // e = Shared<Thing>::make(1,2,3);
    template <typename... Args>
    static Shared<T> make(Args... args) {
        return Shared<T>{new T(args...)};
    }

};

#endif
