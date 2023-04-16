#pragma once 
#include <ostream>
#include <vector>

template <typename Iterator> 
class IteratorRange{ 
    public: 

        IteratorRange(Iterator begin, Iterator end, size_t size):page_begin(begin), page_end(end) {} 

        auto begin() const{ 
            return page_begin;
        } 
        
        auto end() const{ 
            return page_end;
        } 
        
    private: 
    
      Iterator page_begin; 

      Iterator page_end; 

}; 

  

  

  

template <typename Iterator> 

class Paginator{ 

    public: 

        Paginator(Iterator begin, Iterator end, size_t size){
            auto buf = begin; 
            for(; buf < end; buf = buf + size){ 
                IteratorRange page(buf, buf + size, size);
                pages.push_back(page); 
            } 
        } 
        
        auto begin() const { 
            return pages.begin(); 
        } 
        
        auto end() const { 
            return pages.end(); 
        } 
    
    private: 

        std::vector<IteratorRange<Iterator>>pages; 

}; 

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size); 
} 

template<typename Iterator>
std::ostream& operator<< (std::ostream& ostr, const IteratorRange<Iterator>& page) {
    for (auto it = page.begin(); it != page.end(); ++it) {
        ostr << *it; 
    } 
    return ostr; 
} 