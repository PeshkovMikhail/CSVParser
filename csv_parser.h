#include <fstream>
#include <sstream>
#include <tuple>
#include <string>
#include <list>
#include <cassert>

template<typename Type>
Type get_item(std::list<std::string>* itemList){
    std::string itemStr = itemList->front();
    std::stringstream ss(itemStr);
    itemList->pop_front();
    Type item;
    ss >> item;
    return item;
}

template<>
inline std::string get_item(std::list<std::string>* itemList) {
    std::string res = itemList->front();
    itemList->pop_front();
    return res;
}

template<typename... Args>
class CSVParser{
private:
    std::list<std::string>* readLine(char sep = ',', char new_line = '\n', char screen='\\') {
        auto* res = new std::list<std::string>{""};
        assert(res);
        char t = ' ';
        bool screen_status = false;
        bool screen_skip = false;
        if(input.eof()){
            success = false;
            return res;
        }

        input.get(t);

        while(t!= new_line && !input.eof()) {
            if(t == sep && !screen_status){
                res->emplace_front("");
                input.get(t);
                continue;
            } else if(t == screen){
                screen_skip = true;
                goto end;
            }
            else if (t == '"' && !screen_skip) {
                screen_status = !screen_status;
                goto end;
            }

            res->front() += t;
            screen_skip = false;
            end:
            input.get(t);
        }
        if(screen_status)
            throw std::string("screen sequence not closed");
        return res;
    }
    std::ifstream& input;
    bool success = true;
public:
    CSVParser(std::ifstream& in, int lines_skip) : input(in) {
        if(!in.good())
            throw "file not found";
        for(int i = 0; i < lines_skip; i++)
            readLine();
        next();
    }

    void next() {
        auto t = readLine();
        if(success)
            current_ = std::tuple<Args...>(get_item<Args>(t)...);
        delete t;
    }

    bool valid() {
        return success;
    }

    class iterator
    {
        CSVParser* obj_;
    public:
        using value_type = std::tuple<Args...>;
        using reference = const std::tuple<Args...>&;
        using pointer = const std::tuple<Args...>*;
        using iterator_category = std::input_iterator_tag;
        iterator(CSVParser* obj=nullptr): obj_ {obj} {}
        reference operator*() const { return obj_->current_; }
        iterator& operator++() { increment(); return *this; }
        const iterator operator++(int) { increment(); return *this; }
        bool operator==(iterator rhs) const { return obj_ == rhs.obj_; }
        bool operator!=(iterator rhs) const { return !(rhs==*this); }
        void increment()
        {
            obj_->next();
            if (!obj_->valid())
                obj_ = nullptr;
        }
    };
    iterator begin() { return iterator{this}; }
    iterator end() { return iterator{}; }

    std::tuple<Args...> current_ = std::tuple<Args...>();
};


