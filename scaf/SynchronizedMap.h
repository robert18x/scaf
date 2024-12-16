#pragma once
#include <map>
#include <mutex>
#include <optional>
#include <utility>

namespace scaf {

template <typename Key, typename Value>
class SynchronizedMap {
public:
    constexpr std::optional<Value> get(const Key& key) {
        std::scoped_lock guard(accessMutex);
        if (auto it = activeConversations.find(key); it != activeConversations.end()){
            return it->second;
        } else {
            return std::nullopt;
        }
    }

    constexpr void erase(const Key& key) {
        std::scoped_lock guard(accessMutex);
        activeConversations.erase(key);
    }

    constexpr auto emplace(Key&& key, Value&& value) {
        std::scoped_lock guard(accessMutex);
        [[maybe_unused]] auto [iterator, inserted] = activeConversations.emplace(std::forward<Key>(key), std::forward<Value>(value));
        return iterator->second;
    }

private:
    std::map<Key, Value> activeConversations;
    std::mutex accessMutex;
};
    
}
