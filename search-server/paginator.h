#pragma once
#include <ostream>
#include <vector>


template <typename Iterator>
class IteratorRange{
    public:
    IteratorRange(Iterator begin, Iterator end, size_t size):page(begin, end), size(size){}
    
    auto begin() const{
    return page.first;
    }
    
    auto end() const{
    return page.second;
    }
    
    private:
    std::pair<Iterator,Iterator> page;
    size_t size;
};
 
 
 
template <typename Iterator>
class Paginator{
    public:
    Paginator(Iterator begin, Iterator end, size_t size):begin_(begin), end_(end), size(size){
    
    auto buf = begin_;
    
    for(;buf < end_; buf = buf+size){
    IteratorRange page(buf,buf+size,size);
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
    Iterator begin_;
    Iterator end_;
    size_t size;
    std::vector<IteratorRange<Iterator>>pages;
    
    
    
};
template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

template<typename Iterator>
std::ostream& operator <<(std::ostream& o, const IteratorRange<Iterator>& page) {
    for (auto it = page.begin(); it != page.end(); ++it) {
        o << *it;
    }
   
    return o;
}