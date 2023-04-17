#include <cstdlib>
#include <map>
#include <mutex>
#include <string>
#include <vector>

using namespace std::string_literals;






template<typename Key, typename Value>
class ConcurrentMap {
public:
	static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");
 
	struct Bucked{
			std::map<Key,Value> map;
			std::mutex mutex;
		};
 
	struct Access {
		Access() = default;
		Access(Bucked &bucket, const Key &key):access(bucket.mutex), ref_to_value(bucket.map[key]) {
		}
		std::lock_guard<std::mutex> access;
        
        Value& get(){
           return ref_to_value;
        }
        
		Value &ref_to_value;
        
		~Access() = default;
	};
 
	ConcurrentMap() = default;
    
    void erase(const Key& key) {
        buckets_[static_cast<uint64_t>(key) % buckets_.size()].map.erase(key);
    }
    
	explicit ConcurrentMap(size_t bucket_count) :
			buckets_(bucket_count) {
	}
 
	Access operator[]( const Key &key) {
		return {buckets_[static_cast<uint64_t>(static_cast<uint64_t>(key) % buckets_.size())], key};
	}
    
	std::map<Key, Value> BuildOrdinaryMap() {
		std::map<Key, Value> result;
		for(auto &buc : buckets_){
            buc.mutex.lock();
			result.insert(buc.map.begin(), buc.map.end());
            buc.mutex.unlock();
		}
		return result;
	}
private:
	std::vector<Bucked> buckets_;
};