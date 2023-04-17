#pragma once 
#include <string> 
#include <type_traits>
#include <vector> 
#include <execution>
#include <map>
#include <deque> 
#include <set> 
#include "document.h" 
#include "concurrent_map.h" 
#include <algorithm>
#include "string_processing.h"

const int MAX_RESULT_DOCUMENT_COUNT = 5; 

constexpr double MinimumDifferenceValue = 1e-6;//вывел значение в глобальную константу  

class SearchServer { 

public: 

    template <typename StringContainer> 

    explicit SearchServer(const StringContainer& stop_words); 

    explicit SearchServer(const std::string_view stop_words_text);

    explicit SearchServer(const std::string& stop_words_text);
    
    void AddDocument(int document_id, const std::string_view document, DocumentStatus status, const std::vector<int>& ratings) ; 
    
    template <typename Policy> 
    std::vector<Document> FindTopDocuments(const Policy& policy, const std::string_view raw_query) const ;
    
    template <typename Policy> 
    std::vector<Document> FindTopDocuments(const Policy& policy, const std::string_view raw_query, DocumentStatus status) const ; 
    
    template <typename Policy, typename DocumentPredicate> 
    std::vector<Document> FindTopDocuments(const Policy& policy, const std::string_view raw_query, DocumentPredicate document_predicate) const ; 

    template <typename DocumentPredicate> 
    std::vector<Document> FindTopDocuments(const std::string_view raw_query, DocumentPredicate document_predicate) const ; 

    std::vector<Document>  FindTopDocuments(const std::string_view raw_query, DocumentStatus status) const { 
        return FindTopDocuments( raw_query, [status](int document_id, DocumentStatus document_status, int rating) { 
                return document_status == status;
            }); 
    } 
    
    std::vector<Document>  FindTopDocuments(const std::string_view raw_query) const { 
        return SearchServer::FindTopDocuments(raw_query, DocumentStatus::ACTUAL); 
    } 
    
    int GetDocumentCount() const ; 
    
    int GetDocumentId(int index) const ; 
    
    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const; 
    
    std::set<int>::iterator begin(); 
    
    std::set<int>::const_iterator begin() const;
    
    std::set<int>::iterator end(); 
    
    std::set<int>::const_iterator end() const; 
    
    void RemoveDocument( int document_id);
    
    template <typename Policy>
    void RemoveDocument(const Policy& policy, int document_id);

    //void RemoveDocument(const std::execution::parallel_policy&, int document_id);
    
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::sequenced_policy&, const std::string_view raw_query, int document_id) const;
    
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::parallel_policy&, const std::string_view raw_query, int document_id) const;
    
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::string_view raw_query, 

                                                        int document_id) const ; 

  

private: 

    struct DocumentData { 

        int rating; 

        DocumentStatus status; 

        std::vector<std::string> base;
        
    }; 

    const std::set<std::string, std::less<>> stop_words_; 

   // std::deque<std::string> base;

    std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;
    
    std::map<int, std::map<std::string_view, double>> word_freqs_; 
    
    std::map<int, DocumentData> documents_; 

    std::set<int> document_ids_; 
    
    bool IsStopWord(const std::string_view word) const ; 
    
    static bool IsValidWord(const std::string_view word) ;
    
    std::vector<std::string_view> SplitIntoWordsNoStop(const std::string_view text) const ; 
    
    static int ComputeAverageRating(const std::vector<int>& ratings) ; 
    
    struct QueryWord { 

        std::string_view data; 

        bool is_minus; 

        bool is_stop; 

    }; 
    
    QueryWord ParseQueryWord(const std::string_view text) const ; 
    
    struct Query { 

        std::vector<std::string_view> plus_words; 

        std::vector<std::string_view> minus_words; 

    }; 

    Query ParseQuery(const std::string_view text, bool parall) const;
    
    double ComputeWordInverseDocumentFreq(const std::string_view word) const ; 
    
    template <typename DocumentPredicate> 
    std::vector<Document> FindAllDocuments(const SearchServer::Query& query, 
                                      DocumentPredicate document_predicate) const ;
    
    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const std::execution::parallel_policy&, const SearchServer::Query& query,
                                      DocumentPredicate document_predicate) const;
    
    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const std::execution::sequenced_policy&, const SearchServer::Query& query,
                                      DocumentPredicate document_predicate) const;

}; 

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words) 
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words))
{ 
    if (!std::all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
        throw std::invalid_argument("Some of stop words are invalid");
    } 
} 

template <typename DocumentPredicate> 
std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query, 
                                      DocumentPredicate document_predicate) const { 
    return  SearchServer::FindTopDocuments(std::execution::seq, raw_query, document_predicate);
} 

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const SearchServer::Query& query,
                                      DocumentPredicate document_predicate) const {
    std::map<int, double> document_to_relevance;
    for (const std::string_view word : query.plus_words) { 
        if (word_to_document_freqs_.count(word) == 0) { 
            continue; 
        } 
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) { 
            const auto& document_data = documents_.at(document_id); 
            if (document_predicate(document_id, document_data.status, document_data.rating)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            } 
        } 
    } 
    for (const std::string_view word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue; 
        } 
        for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id); 
        } 
    } 
    std::vector<Document> matched_documents; 
    for (const auto [document_id, relevance] : document_to_relevance) {
        matched_documents.push_back( 
            {document_id, relevance, documents_.at(document_id).rating});
    } 
    return matched_documents; 
} 

template <typename Policy> 
std::vector<Document> SearchServer::FindTopDocuments(const Policy& policy, const std::string_view raw_query) const {
       
            return SearchServer::FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
    }
    
template <typename Policy> 
std::vector<Document> SearchServer::FindTopDocuments(const Policy& policy, const std::string_view raw_query, DocumentStatus status) const {
        
        
            return FindTopDocuments(policy, raw_query, [status](int document_id, DocumentStatus document_status, int rating){
                return document_status == status; 
            });
    } 
    
    template <typename Policy, typename DocumentPredicate> 
    std::vector<Document> SearchServer::FindTopDocuments(const Policy& policy, const std::string_view raw_query, DocumentPredicate document_predicate) const {
        
            const auto query = ParseQuery(raw_query, 0); 
            auto matched_documents = FindAllDocuments(policy, query, document_predicate);
            std::sort(policy, matched_documents.begin(), matched_documents.end(),[](const Document& lhs, const Document& rhs) {
            if (std::abs(lhs.relevance - rhs.relevance) < MinimumDifferenceValue) {
                return lhs.rating > rhs.rating;
            } else { 
                return lhs.relevance > rhs.relevance;
            } 
            }); 
            if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
                matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
            } 
            return matched_documents; 
        
    }
        
    template <typename DocumentPredicate>
    std::vector<Document> SearchServer::FindAllDocuments(const std::execution::parallel_policy&, const SearchServer::Query& query,
                                      DocumentPredicate document_predicate) const {
    ConcurrentMap<int, double> document_to_relevance(document_ids_.size()); 
        for_each(
            std::execution::par, 
            query.plus_words.begin(), query.plus_words.end(),
            [&, document_predicate](auto word){
                if (word_to_document_freqs_.count(word) != 0) { 
                    const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
                    for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                        const auto& document_data = documents_.at(document_id);
                        if (document_predicate(document_id, document_data.status, document_data.rating)) {
                            document_to_relevance[document_id].get() += term_freq * inverse_document_freq; 
                        }
                    }
                } 
            }
        );
    
        for_each(
            std::execution::par, 
            query.minus_words.begin(), query.minus_words.end(),
            [&, document_predicate](auto word){
                if (word_to_document_freqs_.count(word) != 0) {
                for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                    document_to_relevance.erase(document_id);
                }
                }
            }
        );
        std::map<int, double> map_id_relevance = document_to_relevance.BuildOrdinaryMap();
        std::vector<Document> matched_documents;
        for (const auto [document_id, relevance] : map_id_relevance) 
            matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
        return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const std::execution::sequenced_policy&, const SearchServer::Query& query,
                                      DocumentPredicate document_predicate) const {
    ConcurrentMap<int, double> document_to_relevance(document_ids_.size()); 
        for_each(
            std::execution::seq, 
            query.plus_words.begin(), query.plus_words.end(),
            [&, document_predicate](auto word){
                if (word_to_document_freqs_.count(word) != 0) { 
                    const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
                    for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                        const auto& document_data = documents_.at(document_id);
                        if (document_predicate(document_id, document_data.status, document_data.rating)) {
                            document_to_relevance[document_id].get() += term_freq * inverse_document_freq; 
                        }
                    }
                } 
            }
        );
    
        for_each(
            std::execution::seq, 
            query.minus_words.begin(), query.minus_words.end(),
            [&, document_predicate](auto word){
                if (word_to_document_freqs_.count(word) != 0) {
                for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                    document_to_relevance.erase(document_id);
                }
                }
            }
        );
        std::map<int, double> map_id_relevance = document_to_relevance.BuildOrdinaryMap();
        std::vector<Document> matched_documents;
        for (const auto [document_id, relevance] : map_id_relevance) 
            matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
        return matched_documents;
}
  
template <typename Policy>
void SearchServer::RemoveDocument(const Policy& policy, int document_id){
    std::vector<std::string_view> to_erase(static_cast<int>(word_freqs_.at(document_id).size()));
    if(document_ids_.count(document_id)==0){return;}
    documents_.erase(document_id);
    document_ids_.erase(document_id); 
    transform(policy, word_freqs_.at(document_id).begin(), word_freqs_.at(document_id).end(),to_erase.begin(), [](auto& word){ return  (word.first);});
    auto lamda=[this,document_id](auto temp){
        word_to_document_freqs_.at(temp).erase(document_id);
    };
    word_freqs_.erase(document_id);
    for_each(policy, to_erase.begin(),to_erase.end(),lamda);
}
 
