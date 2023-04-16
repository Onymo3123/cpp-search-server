#include "search_server.h" 
#include <string> 
#include <algorithm>
#include <stdexcept> 
#include <cmath> 
#include <numeric> 

SearchServer::SearchServer(const std::string_view& stop_words_text): SearchServer(SplitIntoWords(stop_words_text))  {}  

SearchServer::SearchServer(const std::string& stop_words_text): SearchServer(SplitIntoWords(stop_words_text))  {} 

void SearchServer::AddDocument(int document_id, const std::string_view& document, DocumentStatus status,
                     const std::vector<int>& ratings) { 
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw std::invalid_argument("Invalid document_id");
    } 
    base.push_back({document.begin(), document.end()});
    const auto words = SplitIntoWordsNoStop(base.back());
    const double inv_word_count = 1.0 / words.size();
    for (const std::string_view word : words) { 
        word_to_document_freqs_[word][document_id] += inv_word_count;
        word_freqs_[document_id][word] += inv_word_count; 
    } 
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.insert(document_id); 
    } 

int SearchServer::GetDocumentCount() const { 
    return documents_.size(); 
} 

int SearchServer::GetDocumentId(int index) const {
    return document_ids_.count(index)?index:-1;
}

void SearchServer::RemoveDocument(int document_id){ 
    if(document_ids_.count(document_id)==0){return;}
    for (auto& [word, freq] : word_freqs_.at(document_id)) {
        word_to_document_freqs_.at(word).erase(document_id);
    }
    { 
       auto doc = documents_.find(document_id);
       documents_.erase(doc);
    } 
    { 
       auto doc = find(document_ids_.begin(),document_ids_.end(),document_id);
       document_ids_.erase(doc); 
    } 
} 

void SearchServer::RemoveDocument(const std::execution::sequenced_policy& policy, int document_id){
    std::vector<std::string*> to_erase(static_cast<int>(word_freqs_.at(document_id).size()));
    if(document_ids_.count(document_id)==0){return;}
    documents_.erase(document_id);
    document_ids_.erase(document_id);
    transform(std::execution::seq, word_freqs_.at(document_id).begin(), word_freqs_.at(document_id).end(),to_erase.begin(), [](auto& word){ return new std::string(word.first);});
     auto lamda=[this,document_id](auto temp){
        word_to_document_freqs_.at(*temp).erase(document_id);
    };
    word_freqs_.erase(document_id);
    for_each(std::execution::seq, to_erase.begin(),to_erase.end(),lamda);
}

void SearchServer::RemoveDocument(const std::execution::parallel_policy&, int document_id){
    std::vector<std::string*> to_erase(static_cast<int>(word_freqs_.at(document_id).size()));
    if(document_ids_.count(document_id)==0){return;}
    documents_.erase(document_id);
    document_ids_.erase(document_id); 
    transform(std::execution::par, word_freqs_.at(document_id).begin(), word_freqs_.at(document_id).end(),to_erase.begin(), [](auto& word){ return new std::string(word.first);});
    auto lamda=[this,document_id](auto temp){
        word_to_document_freqs_.at(*temp).erase(document_id);
    };
    word_freqs_.erase(document_id);
    for_each(std::execution::par, to_erase.begin(),to_erase.end(),lamda);
}

std::set<int>::iterator SearchServer::begin() {
    return std::begin(document_ids_);
}
 
std::set<int>::iterator SearchServer::end() {
    return std::end(document_ids_);
}

std::set<int>::const_iterator SearchServer::begin() const{
    return std::begin(document_ids_);
}
 
std::set<int>::const_iterator SearchServer::end() const{
    return std::end(document_ids_);
}

const std::map<std::string_view, double>& SearchServer::GetWordFrequencies (int document_id) const{
    static std::map<std::string_view, double> result;
    for(auto page:word_freqs_){ 
        if(page.first == document_id){
            result = (page.second);
        }  
    } 
    return result; 
} 


std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::sequenced_policy& , const std::string_view& raw_query, int document_id) const {
    return MatchDocument( raw_query, document_id);
}   

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::parallel_policy& , const std::string_view& raw_query, int document_id) const {
    const auto query = ParseQuery(raw_query, 1); 
    if(query.minus_words.size()){
	   if(std::any_of(std::execution::par, query.minus_words.begin(), query.minus_words.end(),
	   [this, document_id](const std::string_view word){
	   return ((word_to_document_freqs_.count(word) != 0)&&(word_to_document_freqs_.at(word).count(document_id)));
	   }))
	   return { std::vector<std::string_view>{}, documents_.at(document_id).status };
	}
	std::vector<std::string_view> matched_words(query.plus_words.size());
	auto it = std::copy_if(std::execution::par, query.plus_words.begin(), query.plus_words.end(), matched_words.begin(),
	[this, document_id](const std::string_view word){
	return ((word_to_document_freqs_.count(word) != 0)&&(word_to_document_freqs_.at(word).count(document_id)));
	});
	std::sort( matched_words.begin(), it);
	matched_words.erase(unique( matched_words.begin(), it), matched_words.end());
    return {matched_words, documents_.at(document_id).status};
}  

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::string_view& raw_query, 
                                                        int document_id) const {
    const auto query = ParseQuery(raw_query, 0); 
    std::vector<std::string_view> matched_words; 
    for (const std::string_view word : query.minus_words) { 
        if (word_to_document_freqs_.count(word) == 0) { 
            continue; 
        } 
        if (word_to_document_freqs_.at(word).count(document_id)) { 
            matched_words.clear(); 
            return {matched_words, documents_.at(document_id).status};
        } 
    }
    for (const std::string_view word : query.plus_words) { 
        if (word_to_document_freqs_.count(word) == 0) { 
            continue; 
        } 

        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word); 
        } 
    } 
    return {matched_words, documents_.at(document_id).status};
}

bool SearchServer::IsStopWord(const std::string_view& word) const { 
    return stop_words_.count(word) > 0;
} 

bool SearchServer::IsValidWord(const std::string_view& word) {
    return std::none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' '; 
    }); 

} 

std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(const std::string_view& text) const { 
    std::vector<std::string_view> words; 
    for (const std::string_view word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) { 
            throw std::invalid_argument("Word " + std::string(word) + " is invalid"); 
        } 
        if (!IsStopWord(word)) { 
            words.push_back(word); 
        } 
    } 
    return words; 
} 

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) { 
    if (ratings.empty()) { 
        return 0; 
    } 
    int rating_sum = 0; 
    rating_sum = std::accumulate(ratings.begin(),ratings.end(),0);
    return rating_sum / static_cast<int>(ratings.size());
} 

SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string_view& text) const { 
    if (text.empty()) { 
        throw std::invalid_argument("Query word is empty");
    }
    std::string_view word = text; 
    bool is_minus = false; 
    if (word[0] == '-') {
        is_minus = true; 
        word = word.substr(1); 
    } 
    if (word.empty() || word[0] == '-' || !IsValidWord(word)) { 
        throw std::invalid_argument("Query word " + std::string(text) + " is invalid");
    } 
     return {word, is_minus, IsStopWord(word)}; 
} 

SearchServer::Query SearchServer::ParseQuery(const std::string_view& text, bool parall) const { 
    Query result;
    for (const std::string_view word : SplitIntoWords(text)) { 
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) { 
            if (query_word.is_minus) { 
                result.minus_words.push_back(query_word.data); 
            } else { 
                result.plus_words.push_back(query_word.data); 
            } 
        } 
    } 
    if(!parall){
    std::sort( result.plus_words.begin(), result.plus_words.end());
    std::sort(result.minus_words.begin(), result.minus_words.end());
	result.plus_words.erase(unique( result.plus_words.begin(), result.plus_words.end()), result.plus_words.end());
    result.minus_words.erase(unique( result.minus_words.begin(), result.minus_words.end()), result.minus_words.end());
    }
    return result;
} 

double SearchServer::ComputeWordInverseDocumentFreq(const std::string_view word) const { 
    return std::log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
} 

