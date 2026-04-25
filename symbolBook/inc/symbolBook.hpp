#include <string>
#include <unordered_map>
#include <vector>
#include "orderbook.hpp"

struct Snapshot {
    std::string strSymbol;
    long long llLastUpdateId;
    std::vector<std::pair<double, double>> bids;
    std::vector<std::pair<double, double>> asks;
};

struct DepthUpdate {
    std::string str_eventType;
    long long lleventTime;
    std::string strSymbol;
    long long llFirstUpdateId;
    long long llFinalUpdateId;
    std::vector<std::pair<double, double>> bids;
    std::vector<std::pair<double, double>> asks;
};

class SymbolBook {
private:
    std::unordered_map<std::string, OrderBook> orderBooks;

public:
    void display() const;
    const OrderBook& getOrderBook(const std::string& strSymbol) const;
    void updateOrderBook(const std::string& strSymbol, double dPrice, double dQuantity, bool bIsBid);
    void applySnapshot(const Snapshot& sSnapshot);
    void handleDepthUpdate(const DepthUpdate& sUpdate);
    ~SymbolBook() = default;
};
