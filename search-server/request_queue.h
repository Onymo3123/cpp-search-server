        
#pragma once 

#include <vector> 

#include "search_server.h" 

#include <string> 

#include <deque> 

 

class RequestQueue { 

     

public: 

    explicit RequestQueue(const SearchServer& search_server) ; 
    
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) ; 

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status) ; 

    std::vector<Document> AddFindRequest(const std::string& raw_query) ; 

    int GetNoResultRequests() const ; 

private: 

    int count{}; 
    
    struct QueryResult { 

        std::string query{}; 

        bool empty{}; 

        int count{}; 

    };
    
    std::deque<QueryResult> requests_; 

    const static int min_in_day_ = 1440; 

    const SearchServer& server_; 
    
}; 

 

template <typename DocumentPredicate> 
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) { 

    std::vector<Document> result {}; 
    result = server_.FindTopDocuments(raw_query, document_predicate);
    bool flag_empty_result = result.empty(); 
    bool flag_requests_min_in_day = (requests_.size() < min_in_day_);
    if (flag_empty_result && flag_requests_min_in_day) { 
        count++; 
        requests_.push_back({raw_query, 1, count});
    } else if ((flag_empty_result && !flag_requests_min_in_day)) {
        requests_.pop_front(); 
        requests_.push_back({raw_query, 1, count}); 
    } else if (!flag_empty_result && flag_requests_min_in_day) { 
        requests_.push_back({raw_query, 0, count});
    } else if (!flag_empty_result && !flag_requests_min_in_day) {
        count--; 
        requests_.push_back({raw_query, 0, count}); 
        requests_.pop_front(); 
    } 
    return result; 

} 