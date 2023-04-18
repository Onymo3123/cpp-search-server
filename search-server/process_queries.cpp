#include "process_queries.h"
#include <algorithm>
#include <execution>
#include <iostream>

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries){
    std::vector<std::vector<Document>> documents_lists(queries.size());
    std::transform(std::execution::par, queries.begin(), queries.end(), documents_lists.begin(),
              [&search_server](std::string query) { return search_server.FindTopDocuments(query); });
    return documents_lists;
}

std::vector<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries){
    std::vector<std::vector<Document>> answer;
    answer = ProcessQueries(search_server, queries);
    std::vector<Document> result;
    for(std::vector<Document> documents :answer){
        for(Document document : documents){
            result.push_back(document);
        }
    }
    return result;   
}