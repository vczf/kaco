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
    void trim(){
        auto used = end - begin;
        memmove(data ,data + begin, used);
        begin = 0;
        end = used;
    }
    std::size_t count(){
        return end - begin;
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
        }

        stream_buffer(const stream_buffer& rhs): stream_buffer(){
            put(rhs);
        }

        stream_buffer(stream_buffer&& rhs):stream_buffer(){
            rhs.swap(*this);
        }

        stream_buffer& operator = (const stream_buffer& rhs){
            stream_buffer(rhs).swap(*this);
            return *this;
        }

        stream_buffer& operator = (stream_buffer&& rhs){
            rhs.swap(*this);
            return *this;
        }

        void put(const void* src, std::size_t count){
            gen(count);
            auto chunk = m_last;
            memcpy(chunk->data + chunk->end, src, count);
            chunk->end += count;
            m_size +=count;
        }

        void put(const stream_buffer& data){
            auto count = data.size();
            gen(count);
            for(auto src = data.m_first; src; src = src->next){
                memcpy(m_last->data + m_last->end, src->data + src->begin, src->count());
                m_last->end += src->count();
            }
            m_size +=count;
        }
        
        bool get(void* dst, std::size_t count){
            if(m_size < count) return false;
            auto chunk = m_first;
            std::size_t num = 0;
            while(num < count){
                auto tmp_count = std::min(count - num, chunk->count());
                memcpy(static_cast<unsigned char*>(dst) + num, chunk->data + chunk->begin, tmp_count);
                num += tmp_count;
                chunk->begin += tmp_count;
                if(chunk->next && chunk->end == chunk->begin){
                    auto tmp = chunk->next;
                    line::destory(chunk);
                    chunk = tmp;
                    m_first = tmp;
                }
                else{
                    chunk = chunk->next;
                }
            }
            m_size -= count;
            clear();
            return true;
        }
        
        ~stream_buffer(){
            m_size = 0;
            auto ptr = m_first;
            while(ptr != nullptr){
                auto tmp = ptr->next;
                line::destory(ptr);
                ptr = tmp;
            }
        }

        void swap(stream_buffer& rhs){
            using std::swap;
            swap(m_first, rhs.m_first);
            swap(m_last, rhs.m_last);
            swap(m_size, rhs.m_size);
        }
        std::size_t size() const{
            return m_size;
        }
    private:
        void gen(std::size_t count){
            auto chunk = m_last;
            if(chunk && chunk->capacity - chunk->end < count ){
                auto avail = chunk->count();
                if(chunk->capacity - avail >= count){
                    chunk->trim();
                }
                else{
                    chunk = nullptr;
                }
            }
            if(nullptr == chunk){
                chunk = line::create(count);
                (nullptr == m_last ? m_first : m_last->next) = chunk;
                m_last = chunk;
            }
        }
        line* m_first;
        line* m_last;
        std::size_t m_size;
};

}
#endif
