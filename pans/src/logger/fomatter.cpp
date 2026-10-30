#include <logger/fomatter.h>
#include <unordered_map>

namespace pans::detail{
    constexpr std::string_view DEFAULT_DATE_FORMAT = "%Y-%m-%d %H:%M:%S";
    Formatter::Formatter(std::string_view pattern):m_pattern(pattern){
        if(parse()){
            throw "";
        }
    }
    void Formatter::format(const LogRecordView& record,FormattedRecordBuffer& output) const
    {
         for(const auto& item: m_items){
            item->format(record,output);
         }
    }
    template<typename T>
    void AppendInteger(FormattedRecordBuffer& output, T value){
        std::array<char,24>buffer{};
        const auto result = std::to_chars(buffer.data(),buffer.data()+buffer.size(),value);
        output.append(buffer.data(),static_cast<std::size_t>(result.ptr-buffer.data()));
    }//数字转字符串

    class LiteralFormatItem final :public Formatter::FormatItem{
        private:
        std::string m_value;
        public:
        explicit LiteralFormatItem(std::string value):
        m_value(std::move(value)){

        }
        void format(const LogRecordView&,FormattedRecordBuffer& output) const{
        output.append(m_value);
        }
    };

    class MessageFormatItem final :public Formatter::FormatItem{
        public:
        void format(const LogRecordView& record,FormattedRecordBuffer& output) const{
        output.append(record.m_message);
        }
    };

    class LevelFormatItem final :public Formatter::FormatItem{
        public:
        void format(const LogRecordView& record,FormattedRecordBuffer& output) const{
        output.append(Loglevel::ToString(record.m_level));
        }
    };

    class DurationFormatItem final :public Formatter::FormatItem{
        public:
        void format(const LogRecordView& record,FormattedRecordBuffer& output) const{
        AppendInteger(output,std::chrono::duration_cast<std::chrono::milliseconds>(record.m_duration).count());
        }
    };

    class LoggerNameFormatItem final :public Formatter::FormatItem{
        public:
        void format(const LogRecordView& record,FormattedRecordBuffer& output) const{
        output.append(record.m_loggername);
        }
    };

    class ThreadIdFormatItem final :public Formatter::FormatItem{
        public:
        void format(const LogRecordView& record,FormattedRecordBuffer& output) const{
        AppendInteger(output,record.m_threadid);
        }
    };

    class NewlinewFormatItem final :public Formatter::FormatItem{
        public:
        void format(const LogRecordView&,FormattedRecordBuffer& output) const{
        output.append('\n');
        }
    };
    
    class DataTimeFormatItem final :public Formatter::FormatItem{
        public:
        explicit DataTimeFormatItem(std::string_view format)
        :m_format(format.empty()? DEFAULT_DATE_FORMAT : format){

        }
        void format(const LogRecordView& record,FormattedRecordBuffer& output) const
        {
        static thread_local time_t last_second =0;
        static thread_local char cached_data_time[20] ={'\0'};
        const auto duration = record.m_timestamp.time_since_epoch();
        const time_t current_second = static_cast<time_t>(std::chrono::duration_cast<std::chrono::seconds>(duration).count());
        if(current_second!=last_second)
        {
        std::tm buffer{};
        #if defined(__WIN32)
        localtime_s(&buffer,&current_second);
        #else
        localtime_r(&current_second,&buffer);
        #endif
        std::strftime(cached_data_time,sizeof(cached_data_time),m_format.c_str(),&buffer);
        last_second = current_second;
        }
        output.append(cached_data_time);
        }
        private:
        std::string m_format;
    };

    class MicrosecondFormatItem final :public Formatter::FormatItem{
        public:
        void format(const LogRecordView& record,FormattedRecordBuffer& output) const {
        const auto total_microsecons = std::chrono::duration_cast<std::chrono::microseconds>(record.m_timestamp.time_since_epoch()).count();
        s64 microseconds = total_microsecons % 1000000;
        if(microseconds<0){
            microseconds+=1000000;
        }
        //对齐
        std::array<char,MICROSECONDS_WIDTH> buffer{};
        auto remaining = static_cast<u32>(microseconds);
        for(std::size_t index = buffer.size();index>0;index--){
            buffer[index-1] = static_cast<char>('0'+remaining%10);
            remaining/=10;
        }
        output.append(buffer.data(),buffer.size());
        }
        private:
        static constexpr std::size_t MICROSECONDS_WIDTH = 6;
    };

    class FileNameFormatItem final :public Formatter::FormatItem{
        public:
        void format(const LogRecordView& record,FormattedRecordBuffer& output) const{
        output.append(record.m_file_name);
        }
    };
    
    class LineFormatItem final :public Formatter::FormatItem{
        public:
        void format(const LogRecordView& record,FormattedRecordBuffer& output) const{
        AppendInteger(output,record.m_line);
        }
    };

    class TabFormatItem final :public Formatter::FormatItem{
        public:
        void format(const LogRecordView&,FormattedRecordBuffer& output) const {
        output.append('\t');
        }
    };

    class FiberFormatItem final :public Formatter::FormatItem{
        public:
        void format(const LogRecordView& record,FormattedRecordBuffer& output) const{
        AppendInteger(output,record.m_fiberid);
        }
    };
    class ThreadNameFormatItem final :public Formatter::FormatItem{
        public:
        void format(const LogRecordView& record,FormattedRecordBuffer& output) const{
        output.append(record.m_thread_name);
        }
    };

    using FromatItemFactory = std::unique_ptr<Formatter::FormatItem>(*) (std::string_view);

    template<typename Item>
    std::unique_ptr<Formatter::FormatItem> CreateSimpleFormatItem(std::string_view){
        return std::make_unique<Item>();
    }   

    template<typename Item>
    std::unique_ptr<Formatter::FormatItem> CreateConfigureFormatItem(std::string_view format){
        return std::make_unique<Item>(format);
    }

    const std::unordered_map<char,FromatItemFactory>& GetFormatItemFactories()
    {
        static const std::unordered_map<char,FromatItemFactory> FORMAT_ITEM_FACTORY
        {
            {'m',&CreateSimpleFormatItem<MessageFormatItem>},
            {'p',&CreateSimpleFormatItem<LevelFormatItem>},
            {'r',&CreateSimpleFormatItem<DurationFormatItem>},
            {'c',&CreateSimpleFormatItem<LoggerNameFormatItem>},
            {'t',&CreateSimpleFormatItem<ThreadIdFormatItem>},
            {'n',&CreateSimpleFormatItem<NewlinewFormatItem>},
            {'d',&CreateConfigureFormatItem<DataTimeFormatItem>},
            {'u',&CreateSimpleFormatItem<MicrosecondFormatItem>},
            {'f',&CreateSimpleFormatItem<FileNameFormatItem>},
            {'l',&CreateSimpleFormatItem<LineFormatItem>},
            {'T',&CreateSimpleFormatItem<TabFormatItem>},
            {'F',&CreateSimpleFormatItem<FiberFormatItem>},
            {'N',&CreateSimpleFormatItem<ThreadNameFormatItem>}
        };
        return FORMAT_ITEM_FACTORY;
    }

    std::unique_ptr<Formatter::FormatItem> CreateFormatItem(char directive,std::string_view format){
        const auto& factories = GetFormatItemFactories();
        const auto it = factories.find(directive);
        return it->second(format);
    }

    void Formatter::addLiteral(std::string& literal){
        if(literal.empty()){
            return;
        }
        m_items.push_back(std::make_unique<LiteralFormatItem>(std::move(literal)));
        literal.clear();
    }

    int Formatter::parse(){
        std::string literal;
        for(std::size_t index=0;index<m_pattern.size();index++){
            if(m_pattern[index]!='%'){
                literal.push_back(m_pattern[index]);
                continue;
            }
            const char directive = m_pattern[++index];
            if(directive=='%'){
                literal.push_back('%');
                continue;
            }
            addLiteral(literal);
            std::string_view item_format;
            if(index+1<m_pattern.size()&&m_pattern[index+1]=='{'){
                const std::size_t closing_brace = m_pattern.find('}',index+2);
                item_format = std::string_view(m_pattern).substr(index+2,closing_brace-index-2);
                index = closing_brace;
            }
            m_items.push_back(CreateFormatItem(directive,item_format));
        }
        addLiteral(literal);
        return 0;
    }
}

