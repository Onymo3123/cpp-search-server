#pragma once
#include <vector>
#include "search_server.h"
#include <string>
#include <deque>
class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server) ;
    
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        // напишите реализацию
        std::vector<Document> result {};
        result = server_.FindTopDocuments(raw_query, document_predicate);
        if(result.empty() && (requests_.size() < min_in_day_)){
        count++;
        requests_.push_back({raw_query,1,count});
            
        }else if((result.empty() && (requests_.size() >= min_in_day_))){
        requests_.pop_front();
        requests_.push_back({raw_query,1,count});
        }else if(!result.empty()&&(requests_.size() < min_in_day_)){
           requests_.push_back({raw_query,0,count});
        }else if(!result.empty()&&(requests_.size() >= min_in_day_)){
          count--;
            requests_.push_back({raw_query,0,count});
            requests_.pop_front();
        }
        return result;
        
        
        
    }
    
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