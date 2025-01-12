#pragma once
#include <map>
#include <mutex>
#include <optional>
#include <utility>

namespace scaf {

template <typename Key, typename Value, template<typename, typename> typename UnderlyingType = std::map>
class SynchronizedMap {
public:
    virtual ~SynchronizedMap() = default;

    constexpr std::optional<Value> get(const Key& key) {
        std::scoped_lock guard(accessMutex);
        if (auto it = map.find(key); it != map.end()){
            return it->second;
        } else {
            return std::nullopt;
        }
    }

    constexpr std::optional<Value> getAndErase(const Key& key) {
        std::scoped_lock guard(accessMutex);
        if (auto it = map.find(key); it != map.end()){
            auto tmp = std::move(it->second);
            map.erase(it);
            return tmp;
        } else {
            return std::nullopt;
        }
    }

    constexpr void erase(const Key& key) {
        std::scoped_lock guard(accessMutex);
        map.erase(key);
    }

    constexpr auto emplace(Key&& key, Value&& value) {
        std::scoped_lock guard(accessMutex);
        [[maybe_unused]] auto [iterator, inserted] = map.emplace(std::forward<Key>(key), std::forward<Value>(value));
        return iterator->second;
    }

    constexpr bool contains(const Key& key) {
        return map.contains(key);
    }

protected:
    UnderlyingType<Key, Value> map;
    std::mutex accessMutex;
};
    
}
