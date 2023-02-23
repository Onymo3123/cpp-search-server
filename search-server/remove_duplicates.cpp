#include "remove_duplicates.h"
#include <map>
#include <string>
#include <vector>
#include <set>
//вроде поправил все, честно говоря некоторые вещи я правил до этого, видимо потом копировал с тренажера код, а он похоже меняется переодически(например константу я уже задавал, помню[MinimumDifferenceValue = 1e-6]).
void RemoveDuplicates(SearchServer& search_server){
    std::map<std::string, double> word_freqs;
    std::map<std::set<std::string>, int> docs;
    std::vector<int> docs_to_delete;
    std::set<std::string> document;
    for (auto document_id : search_server) {
        word_freqs = search_server.GetWordFrequencies(document_id);
        for (auto word_frequency_pair : word_freqs) {
        document.insert(word_frequency_pair.first);
        }
        if (docs.count(document)>0) {
            docs_to_delete.push_back(document_id);
        } else {
           docs[document] =  document_id;
        }
        document={}; 
    } 
    for(auto id : docs_to_delete){
    std::cout << "Found duplicate document id " << id << std::endl; 
    search_server.RemoveDocument(id);
    }
 }
