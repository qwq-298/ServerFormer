#ifndef YYY
#define YYY

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <streambuf>
#include <vector>
#include <string_view>

namespace pans::detail{
   
    //日志短 栈
    template<std::size_t INLINE_CAPACITY>
    class Inline_buffer{
        private:
        std::array<char,INLINE_CAPACITY> m_inline{};
        std::vector<char> m_overflow;
        std::size_t m_size=0;
        public:
        void append(const char* data, std::size_t size){
            if(size==0){
                return;
            }
            if(m_overflow.empty()&& size+m_size<INLINE_CAPACITY){
                std::memcpy(m_inline.data()+m_size,data,size);
                m_size+=size;
                return;
            }
            if(m_overflow.empty()){
                const std::size_t need_size = size+m_size;
                m_overflow.reserve(std::max(2*INLINE_CAPACITY,need_size));
                m_overflow.insert(m_overflow.end(),m_inline.data(),m_inline.data()+m_size);
            }
            m_overflow.insert(m_overflow.end(),data,data+size);
            m_size = m_overflow.size();
        }
        void append(std::string_view value){
            append(value.data(),value.size());
        }
        void append(char value){
            append(&value,1);
        }
        [[nodiscard]] const char* data () const noexcept{
            return m_overflow.empty() ? m_inline.data() : m_overflow.data();
        }
        [[nodiscard]] std::size_t size () const noexcept{
            return m_size;
        }
        [[nodiscard]] std::string_view  view() const noexcept{
            return {data(),size()};
        }
    };
    //日志长 堆

    template<std::size_t INLINE_CAPACITY>
    class SmallStringBuffer final : public std::streambuf
    {
        public:
        explicit SmallStringBuffer(Inline_buffer<INLINE_CAPACITY>& buffer) noexcept
        :m_buffer(buffer){}
        protected:
        std::streamsize xsputn(const char* data,std::streamsize size) override{
            if(size<=0){
                return 0;
            }
            m_buffer.append(data,static_cast<std::size_t>(size));
            return size;
        }
        int_type overflow(int_type charactor) override{
            if(traits_type::eq_int_type(charactor,traits_type::eof())){
                return traits_type::not_eof(charactor);
            }
            m_buffer.append(traits_type::to_char_type(charactor));
            return charactor;
        }
        private:
        Inline_buffer<INLINE_CAPACITY>& m_buffer;
    };
}

#endif
