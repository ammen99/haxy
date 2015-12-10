#include <iostream>
#include <stack>

#define MEMPOOL_INITIAL_CHUNKS 2
#define MEMPOOL_CHUNK 8
#define MAX_CHUNKS (1024 * 1024)

namespace ptr {

    class memory_pool {
        struct memory_pool_stack {
            int *arr[MAX_CHUNKS];   
            int **helper = arr - 1;

            int tp = 0;

            void push(int *next) { arr[tp++] = next; }

            int *top() { return helper[tp]; }
            void pop() { --tp; }

            bool empty() { return !tp; }

            int size() { return tp; }

            void pre_init() {
                for(int i = 0; i < MEMPOOL_INITIAL_CHUNKS; i++) {
                    arr[tp++] = new int;
                    arr[tp++] = new int;
                    arr[tp++] = new int;
                    arr[tp++] = new int;
                    arr[tp++] = new int;
                    arr[tp++] = new int;
                    arr[tp++] = new int;
                    arr[tp++] = new int;
                }
            }

            void delete_all() {
                for(int i = 0; i < tp; i++) delete arr[i];    
            }
        } s;

        public:
            memory_pool() {
                s.pre_init();
            }

            int *get_chunk() {
                if(s.empty())
                    return new int; 
                else {
                    int *chunk = s.top();
                    s.pop();
                    *chunk = 0;
                    return chunk;
                }
            }

            void delete_chunk(int *chunk) {
                delete chunk;
            }

            void rem_chunk(int *chunk) {
                if(s.size() > MAX_CHUNKS)
                    delete_chunk(chunk);
                else
                    s.push(chunk); 
            }

            ~memory_pool() {
                s.delete_all();
            }
    };

    extern memory_pool mempool;

    template<class T> class shared_ptr {
        private:
            int *refcnt;

            T* ptr;

            inline void self_destroy() {
                mempool.rem_chunk(refcnt);
                delete ptr;
            }

        public:
            shared_ptr() {
                refcnt = mempool.get_chunk(); 
                ++*refcnt;
                ptr = new T;
            }

            shared_ptr(T* p) : ptr(p) {
                refcnt = mempool.get_chunk();
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
                    if(!(--*refcnt))
                        self_destroy();

                    ptr = other.ptr;
                    refcnt = other.refcnt;
                    ++*refcnt;
                }

                return *this;
            }


            shared_ptr<T> operator=(const shared_ptr<T>&& other) {
                if(this != &other) {
                    if(!(--*refcnt))
                        self_destroy();

                    ptr = other.ptr;
                    refcnt = other.refcnt;
                    ++*refcnt;
                }

                return *this;
            }

            ~shared_ptr() {
                if(!(--*refcnt))
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
