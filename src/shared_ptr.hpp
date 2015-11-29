#include <iostream>

namespace ptr {
    class ref_counter {
        private:
            int counter;

        public:
            void inc() {++counter;};
            void dec() {--counter;};
            int  get() {return counter;};
    };

    template<class T> class shared_ptr {
        private:
            ref_counter *refcnt;
            T* ptr;

            void self_destroy() {
                delete refcnt;
                delete ptr;
            }
        public:
            shared_ptr() {
                refcnt = new ref_counter();
                refcnt->inc();
                ptr = new T;
            }

            shared_ptr(T* p) : ptr(p) {
                refcnt = new ref_counter();
                refcnt->inc();
            } 

            shared_ptr(const shared_ptr<T> &other) {
                refcnt = other.refcnt;
                ptr = other.ptr;

                refcnt->inc();
            }

            shared_ptr(const shared_ptr<T>&& other) {
                refcnt = other.refcnt;
                ptr = other.ptr;

                refcnt->inc();
            }

            shared_ptr<T> operator=(const shared_ptr<T>& other) {
                if(this != &other) {
                    refcnt->dec(); 
                    if(refcnt->get() < 1)
                        self_destroy();

                    ptr = other.ptr;
                    refcnt = other.refcnt;
                    refcnt->inc();
                }

                return *this;
            }


            shared_ptr<T> operator=(const shared_ptr<T>&& other) {
                if(this != &other) {
                    refcnt->dec(); 
                    if(refcnt->get() < 1)
                        self_destroy();

                    ptr = other.ptr;
                    refcnt = other.refcnt;
                    refcnt->inc();
                }

                return *this;
            }

            ~shared_ptr() {
                refcnt->dec();
                if(refcnt->get() < 1)
                    self_destroy();
            }

            T& operator*() const {
                return *ptr;
            }

            T* operator->() const {
                return ptr;
            }
    };
}
