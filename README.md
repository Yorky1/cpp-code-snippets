# cpp-code-snippets

Cpp code snippets from different courses in university.

## Data structures

- [copy-on-write vector](data_structures/cow_vector.h)
- [deque](data_structures/deque.h)
- [intrusive list](data_structures/intrusive_list.h)
- [Matrix](data_structures/matrix.h) (implementation of matrix class with optimized multipication)
- [Hashmap](https://github.com/Yorky1/HashMap)
  
## Smart pointers

- [Unique-ptr](smart_pointers/unique.h)
- [Shared-ptr](smart_pointers/shared.h)
- [Weak-ptr](smart_pointers/weak.h)
  
## Functions and operators

- [Defer](functions/defer.h) (implementation of defer operator from go language)
- [BindFront](functions/bind_front.h) (implementation of `std::bind_front`)
- [Implementation of simple functions working with constexpr calculations](functions/stdflib.h)

## Threads

- [Semaphore](threads/sema.h)
- [RW-lock](threads/rw_lock.h)
- [RW-spinlock](threads/rw_spinlock)
- [Buffered channel](threads/buffered_channel.h)
- [Unbuffered channel](threads/unbuffered_channel.h)
- [Multiple Producer Single Consumer lock free stack](threads/mpsc_stack.h)
  
## Coroutines

- [Coroutine](coroutines/coroutine.h) (implementation of coroutine class in cpp using `boost::context::continuation`)
- [Generator](coroutines/generator.h)
