#include <iostream>
#include <tuple>
#include "csv_parser.h"

template<typename Ch, typename Tr, class Tuple, std::size_t N>
struct TuplePrinter {
    static void print(std::basic_ostream<Ch, Tr>& os,const Tuple& t)
    {
        TuplePrinter<Ch, Tr, Tuple, N-1>::print(os, t);
        os << ", " << std::get<N-1>(t);
    }
};

template<typename Ch, typename Tr, class Tuple>
struct TuplePrinter<Ch, Tr, Tuple, 1> {
    static void print(std::basic_ostream<Ch, Tr>& os,const Tuple& t)
    {
        os << std::get<0>(t);
    }
};

template<typename Ch, typename Tr, typename... Args, std::enable_if_t<sizeof...(Args) == 0, int> = 0>
std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& os,const std::tuple<Args...>& t)
{
    os << "()";
    return os;
}

template<typename Ch, typename Tr, typename... Args, std::enable_if_t<sizeof...(Args) != 0, int> = 0>
std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& os,const std::tuple<Args...>& t)
{
    os << '(';
    TuplePrinter<Ch, Tr, decltype(t), sizeof...(Args)>::print(os, t);
    os << ')';
    return os;
}


int main() {
    std::ifstream file("test.csv");
    CSVParser<int, std::string> parser(file, 0 /*skip first lines count*/);
    try {
        for (const std::tuple<int, std::string> &t: parser) {
            std::cout << t << std::endl;
        }
    }
    catch(std::exception e){
        std::cout << e.what();
    }
}