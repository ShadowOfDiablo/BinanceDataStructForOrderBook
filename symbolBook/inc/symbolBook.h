#ifndef SYMBOLBOOK_HPP
#define SYMBOLBOOK_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include "orderbook.h"

struct Snapshot {
    std::string strSymbol;
    long long llLastUpdateId;
    std::vector<std::pair<uint64_t, double>> bids;
    std::vector<std::pair<uint64_t, double>> asks;
};

struct DepthUpdate {
    std::string str_eventType;
    long long lleventTime;
    std::string strSymbol;
    long long llFirstUpdateId;
    long long llFinalUpdateId;
    std::vector<std::pair<uint64_t, double>> bids;
    std::vector<std::pair<uint64_t, double>> asks;
};

class SymbolBook {
private:
    std::unordered_map<std::string, OrderBook> orderBooks;

public:
    void display() const;
    const OrderBook& getOrderBook(const std::string& strSymbol) const;
    void updateOrderBook(const std::string& strSymbol, uint64_t price, double qty, bool bIsBid);
    void applySnapshot(const Snapshot& sSnapshot);
    void handleDepthUpdate(const DepthUpdate& sUpdate);
    ~SymbolBook() = default;
};

#endif // SYMBOLBOOK_HPP
