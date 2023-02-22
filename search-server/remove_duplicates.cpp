#include "remove_duplicates.h"
#include <map>
#include <string>
#include <vector>
#include <set>

void RemoveDuplicates(SearchServer& search_server){
    std::map<std::string, double> word_freqs;
    std::vector<std::pair<int, std::set<std::string>>> docs;
    std::set<std::string> document;
    for(auto document_id:search_server){
        word_freqs = search_server.GetWordFrequencies(document_id);
        for(auto f :word_freqs){
        document.insert(f.first);
        }
        docs.push_back({document_id,document});
        document={};
    } 
    for(int i = 0; i < docs.size()-1; i++){
        for(int j = i+1; j < docs.size(); j++){
            if(docs[i].second == docs[j].second){
                std::cout << "Found duplicate document id " << docs[j].first << std::endl;
                search_server.RemoveDocument(docs[j].first);
                docs.erase(docs.begin() + j);
                j--;
            } 
        }  
    }  
}
