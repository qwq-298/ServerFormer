#ifndef QQQ
#define QQQ

#include<memory>
#include<string>
#include<vector>
#include<logger/log_record.h>
#include<logger/buffer.h>
#include<logger/buffer_config.h>

namespace pans::detail{
    using FormattedRecordBuffer = Inline_buffer<FORMMATED_RECORD_INLINE_SIZE>;
    class Formatter final
    {
        public:
        class FormatItem{
            public:
            virtual ~FormatItem()=default;
            virtual void format(const LogRecordView& record,FormattedRecordBuffer& output) const=0;
        };

        explicit Formatter(std::string_view pattern);//拒绝隐式转换

        void format(const LogRecordView& record,FormattedRecordBuffer& output) const ;

        [[nodiscard]]const std::string& getPattern() const noexcept {
            return m_pattern;
        }
        private:
        int parse();
        void addLiteral(std::string& literal);
        private:
        std::string m_pattern ="";
        std::vector<std::unique_ptr<FormatItem>> m_items;
    };
}

#endif