#pragma once

#include <map>
#include <vector>
#include <iostream>

class OrderBook {
private:
    int64_t lastUpdateId = 0;
    // Bids: highest price first
    std::map<double, double, std::greater<double>> bids;
    // Asks: lowest price first
    std::map<double, double> asks;
public:
    void updateBid(double dPrice, double dQuantity);
    void updateAsk(double dPrice, double dQuantity);
    void applySnapshot(int64_t snapshotLastUpdateId,
                       const std::vector<std::pair<double, double>>& snapshotBids,
                       const std::vector<std::pair<double, double>>& snapshotAsks);
    void setLastUpdateId(int64_t id);
    int64_t getLastUpdateId() const;
    void display() const;
    OrderBook();
    ~OrderBook();
    OrderBook(const OrderBook& source);
    OrderBook& operator=(const OrderBook& source);
    bool operator==(const OrderBook& other) const;
    double getBestBid() const;
    double getBestAsk() const;
};
