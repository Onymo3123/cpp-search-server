#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        
        document_count_++;
        if(!words.empty()){
        int words_in_doc = words.size();
        
       for(string g: words){
        word_to_document_freqs_[g][document_id] += 1.0/words_in_doc ;
        
       
       

    }}}

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    int count_documents = 0;

    map<int, int> count_word_;

    struct Query{
        set<string> plus_words;
        set<string> minus_words;
    };
     /*struct DocumentContent {
        int id = 0;
        vector<string> words;
    };

    vector<DocumentContent> documents_;*/

    int document_count_ = 0;

    map<string, map<int, double>> word_to_document_freqs_;

    map<string, set<int>> word_to_documents_;

    set<string> stop_words_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    Query ParseQuery(const string& text) const {
        Query query_words;
        for (string word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-'){

            query_words.minus_words.insert(word.substr(1));
            }else{
            query_words.plus_words.insert(word);
            }
        }
        /*for (string word : query_words.plus_words ){
            if (query_words.minus_words.count(word)>0){
                query_words.plus_words.erase(word);
            }
        }*/

        return query_words;
    }

    vector<Document> FindAllDocuments(const Query& query_words) const {
        map<int, double> document_to_relevance;
        vector<Document> matched_documents;
        
        double EDF = 0;
              if(!query_words.plus_words.empty()){
                  for(string word:query_words.plus_words){
                      if  (word_to_document_freqs_.count(word)>0){
                          
                          EDF = log(1.0*document_count_/word_to_document_freqs_.at(word).size());
                          
                          if (query_words.minus_words.count(word)){//не знаю законно ли но я решил что минус слова лучше не добавлять, чем потом убирать 
                              EDF = -1000;
                              
                          }
                          
                            for(auto& d:word_to_document_freqs_.at(word)){
                               document_to_relevance[d.first]+=(EDF*d.second); 
                            }                      
                  }}
                  
                  for(auto& document:document_to_relevance){
                      if (document.second>-1){//но из-за того что ln дает 0 приходится захватывать его
                     matched_documents.push_back({document.first,document.second});
                      }
                  }
                  
              }
        return matched_documents;
    }

    static int MatchDocument(const string& content, const Query& query_words) {
        if (query_words.plus_words.empty()) {
            return 0;
        }



        set<string> matched_words;




            if ((query_words.minus_words.count(content) != 0)){
                 return 0;
            }
            if ((query_words.plus_words.count(content) != 0)) {
                matched_words.insert(content);
            }

        return static_cast<int>(matched_words.size());
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}

