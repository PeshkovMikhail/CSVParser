#include <fstream>
#include <sstream>
#include <tuple>
#include <string>
#include <list>
#include <cassert>
#include <exception>
#include <utility>
#include "config.h"

class parser_exception : std::exception {
    std::string description;

public:
    explicit parser_exception(std::string text) {
        description = std::move(text);
    }

    virtual const char* what() const throw()
    {
        return description.c_str();
    }
};


template<typename... Args>
class CSVParser{
private:
        int line = 0;
        int column = 0;
    template<typename Type>
    Type get_item(std::list<std::string>* itemList){
        std::string itemStr = itemList->front();
        std::stringstream ss(itemStr);
        itemList->pop_front();
        Type item = Type();
        Type p;
        int c = sizeof...(Args) - column;
        while(ss.rdbuf()->in_avail() != 0) {
            if (!(ss >> p))
                throw parser_exception(std::string("argument type does not match at line ") + std::to_string(line)
                                       + std::string(" column ") + std::to_string(c));
            item += p;
            if(typeid(Type) == typeid(std::string) && ss.rdbuf()->in_avail() != 0) {
                item += ' ';
            }
        }

        column++;
        return item;
    }

    std::list<std::string>* readLine() {
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

        while(t!= line_separator && !input.eof()) {
            if(t == separator && !screen_status){
                res->emplace_front("");
                input.get(t);
                continue;
            } else if(t == disable_screen){
                screen_skip = true;
                goto end;
            }
            else if (t == screen && !screen_skip) {
                screen_status = !screen_status;
                goto end;
            }

            res->front() += t;
            screen_skip = false;
            end:
            input.get(t);
        }
        if(screen_status)
            throw parser_exception(std::string("screen sequence not closed at line ") + std::to_string(line));
        if(res->size() != sizeof...(Args))
            throw parser_exception(std::string("argument count does not match at line ") + std::to_string(line));
        line++;
        return res;
    }
    std::ifstream& input;
    bool success = true;
public:
    CSVParser(std::ifstream& in, int lines_skip) : input(in) {
        if(!in.good())
            throw parser_exception("file not found");
        for(int i = 0; i < lines_skip; i++)
            readLine();
        next();
    }

    void next() {
        auto t = readLine();
        column = 0;
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


