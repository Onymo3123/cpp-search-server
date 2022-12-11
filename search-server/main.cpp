#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <iostream>


using namespace std;




void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(),
                    "Stop words must be excluded from documents"s);
    }
}

/*
Разместите код остальных тестов здесь
*/
 void TestComputeRelevance() {
    
        double check[3] = {0.274653,0.101366,0.101366};
        SearchServer search_server;
        search_server.SetStopWords("и в на"s);
        search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3});
        search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
        search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL,
                              {5, -12, 2, 1});
        int a = 0;
        for (const Document& document : search_server.FindTopDocuments("ухоженный кот"s)) {
            if(a < 3){
                ASSERT(abs(check[a]-document.relevance)<1e-6);    
            }
             a++;   
        }
        
    }
     
    void TestComputeRating() {
    
        double check[3] = {5,8,-3};
        SearchServer search_server;
        search_server.SetStopWords("и в на"s);
        search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, 8, 8, 8, 8});
        search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {-7, -1, -2, -3, -4});
        search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL,
                              {5, -12, 2, 1, 15, 20});
        int a = 0;
        for (const Document& document : search_server.FindTopDocuments("ухоженный кот"s)) {
            if(a < 3){
                ASSERT(abs(check[a]-document.rating)<1e-6);    
            }
             a++;   
        }
        
    } 
    
       
    void TestComputeStatActual() {
    
       int id = 0;
        SearchServer search_server;
        search_server.SetStopWords("и в на"s);
        search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, 8, 8, 8, 8});
        search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::IRRELEVANT, {-7, -1, -2, -3, -4});
        search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::BANNED,
                              {5, -12, 2, 1, 15, 20});
        search_server.AddDocument(3, "ухоженный пёс выразительные кот глаза"s, DocumentStatus::REMOVED,
                              {5, -12, 2, 1, 15, 20});
        int a = 0;
        for (const Document& document : search_server.FindTopDocuments("ухоженный кот"s, DocumentStatus::ACTUAL)) {
            if(a < 1){
                ASSERT(abs(id-document.id)<1e-6);  
            }
             a++;   
        }
        
    } 
    
    void TestComputeStatIrrelevant() {
    
        int id = 1;
        SearchServer search_server;
        search_server.SetStopWords("и в на"s);
        search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, 8, 8, 8, 8});
        search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::IRRELEVANT, {-7, -1, -2, -3, -4});
        search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::BANNED,
                              {5, -12, 2, 1, 15, 20});
        search_server.AddDocument(3, "ухоженный пёс выразительные кот глаза"s, DocumentStatus::REMOVED,
                              {5, -12, 2, 1, 15, 20});
        int a = 0;
        for (const Document& document : search_server.FindTopDocuments("ухоженный кот"s, DocumentStatus::IRRELEVANT)) {
            if(a < 1){
                ASSERT(abs(id-document.id)<1e-6);  
            }
             a++;   
        }
        
    }
    
    void TestComputeStatBanned() {
    
        int id = 2;
        SearchServer search_server;
        search_server.SetStopWords("и в на"s);
        search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, 8, 8, 8, 8});
        search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::IRRELEVANT, {-7, -1, -2, -3, -4});
        search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::BANNED,
                              {5, -12, 2, 1, 15, 20});
        search_server.AddDocument(3, "ухоженный пёс выразительные кот глаза"s, DocumentStatus::REMOVED,
                              {5, -12, 2, 1, 15, 20});
    
        int a = 0;
        for (const Document& document : search_server.FindTopDocuments("ухоженный кот"s, DocumentStatus::BANNED)) {
            if(a < 1){
                ASSERT(abs(id-document.id)<1e-6);
            }
             a++;   
        }
        
    }
    
    void TestComputeStatRemoved() {
    
        int id = 3;
        SearchServer search_server;
        search_server.SetStopWords("и в на"s);
        search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, 8, 8, 8, 8});
        search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::IRRELEVANT, {-7, -1, -2, -3, -4});
        search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::BANNED,
                              {5, -12, 2, 1, 15, 20});
        search_server.AddDocument(3, "ухоженный пёс выразительные кот глаза"s, DocumentStatus::REMOVED,
                              {5, -12, 2, 1, 15, 20});
        int a = 0;
        for (const Document& document : search_server.FindTopDocuments("ухоженный кот"s, DocumentStatus::REMOVED)) {
            if(a < 1){
                ASSERT(abs(id-document.id)<1e-6); 
            }
             a++;   
        }
        
    }
    
    void TestCastomPredict() {
    
        int id = 2;
        SearchServer search_server;
        search_server.SetStopWords("и в на"s);
        search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, 8, 8, 8, 8});
        search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::IRRELEVANT, {-7, -1, -2, -3, -4});
        search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::BANNED,
                              {5, -12, 2, 1, 15, 20});
        search_server.AddDocument(3, "ухоженный пёс выразительные кот глаза"s, DocumentStatus::REMOVED,
                              {5, -12, 2, 1, 15, 20});
        int a = 0;
        for (const Document& document : search_server.FindTopDocuments("ухоженный кот"s, 
               [](int document_id, DocumentStatus status, int rating) { return document_id ==  2; })) {
            if(a < 1){
                ASSERT(abs(id-document.id)<1e-6);   
            }
             a++;   
        }
        
    }
    
    

   



// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestComputeRelevance);
    RUN_TEST(TestComputeRating);
    RUN_TEST(TestComputeStatActual);
    RUN_TEST(TestComputeStatIrrelevant);
    RUN_TEST(TestComputeStatBanned);
    RUN_TEST(TestComputeStatRemoved);
    RUN_TEST(TestCastomPredict);
    
    // Не забудьте вызывать остальные тесты здесь
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished"s << endl;
}