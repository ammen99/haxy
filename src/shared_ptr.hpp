#include <iostream>

namespace ptr {
    template<class T> class shared_ptr {
        private:
            int *refcnt;
            T* ptr;

            void self_destroy() {
                delete refcnt;
                delete ptr;
            }
        public:
            shared_ptr() {
                refcnt = new int; 
                ++*refcnt;
                ptr = new T;
            }

            shared_ptr(T* p) : ptr(p) {
                refcnt = new int;
                ++*refcnt;
            } 

            shared_ptr(const shared_ptr<T> &other) {
                refcnt = other.refcnt;
                ptr = other.ptr;

                ++*refcnt;
            }

            shared_ptr(const shared_ptr<T>&& other) {
                refcnt = other.refcnt;
                ptr = other.ptr;

                ++*refcnt;
            }

            shared_ptr<T> operator=(const shared_ptr<T>& other) {
                if(this != &other) {
                    if(--*refcnt < 1)
                        self_destroy();

                    ptr = other.ptr;
                    refcnt = other.refcnt;
                    ++*refcnt;
                }

                return *this;
            }


            shared_ptr<T> operator=(const shared_ptr<T>&& other) {
                if(this != &other) {
                    if(--*refcnt < 1)
                        self_destroy();

                    ptr = other.ptr;
                    refcnt = other.refcnt;
                    ++*refcnt;
                }

                return *this;
            }

            ~shared_ptr() {
                if(--*refcnt < 1)
                    self_destroy();
            }

            inline T& operator*() const {
                return *ptr;
            }

            inline T* operator->() const {
                return ptr;
            }
    };
}
