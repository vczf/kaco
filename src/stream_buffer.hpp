#ifndef KACO_BUFF
#define KACO_BUFF

#include "def.hpp"
namespace kaco {

struct line {
    static line* create(std::size_t min_size, line* next = nullptr){
        std::size_t capacity = (min_size + sizeof(line) + KACO_CACHE_SIZE - 1) / KACO_CACHE_SIZE * KACO_CACHE_SIZE;
        struct line* tmp = static_cast<line*>(::operator new(capacity));
        tmp->next = next;
        tmp->begin = 0;
        tmp->end = 0;
        tmp->capacity = capacity - sizeof(line);
        return tmp;

    }   
    static void destory(line* tmp){
        ::operator delete(tmp);
    }   
    line *next;
    std::size_t begin, end;
    std::size_t capacity;
    unsigned char data[];
};

class stream_buffer {
    public:
        stream_buffer(): m_first(nullptr), m_last(nullptr), m_size(0){}
        explicit stream_buffer(const void* src, std::size_t size): stream_buffer(){
            put(src, size);
        };
        explicit stream_buffer(stream_buffer&& rhs):stream_buffer(){}
        stream_buffer(const stream_buffer& rhs): stream_buffer(){
            put(rhs);
        };
        void put(const void* src, std::size_t size){}
    private:
        line* m_first;
        line* m_last;
        std::size_t m_size;
};

}
#endif
