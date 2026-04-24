#include <string>
#include <unordered_map>
#include <vector>
#include "orderbook.hpp"

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
    void getOrderBook(const std::string& strSymbol) const;
    void updateOrderBook(const std::string& strSymbol, double dPrice, double dQuantity, bool bIsBid);
    void handleDepthUpdate(const DepthUpdate& sUpdate);
};
