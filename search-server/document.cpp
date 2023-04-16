#include "document.h" 
#include <string> 

std::ostream& operator<<(std::ostream& o, const Document doc) {
            o << "{ " 
            << "document_id = " << doc.id << ", "
            << "relevance = " << doc.relevance << ", " 
            << "rating = " << doc.rating << " }"; 
    return o; 
}