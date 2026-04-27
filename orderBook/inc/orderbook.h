#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <cstdint>
#include <map>
#include <vector>
#include <iostream>

class OrderBook {
private:
    int64_t lastUpdateId = 0;
    // Bids: highest price first (prices stored as uint64_t ticks = price × 1e8)
    std::map<uint64_t, double, std::greater<uint64_t>> bids;
    // Asks: lowest price first
    std::map<uint64_t, double> asks;
public:
    void updateBid(uint64_t price, double qty);
    void updateAsk(uint64_t price, double qty);
    void applySnapshot(int64_t snapshotLastUpdateId,
                       const std::vector<std::pair<uint64_t, double>>& snapshotBids,
                       const std::vector<std::pair<uint64_t, double>>& snapshotAsks);
    void setLastUpdateId(int64_t id);
    int64_t getLastUpdateId() const;
    void display() const;
    OrderBook();
    ~OrderBook();
    OrderBook(const OrderBook& source);
    OrderBook& operator=(const OrderBook& source);
    bool operator==(const OrderBook& other) const;
    uint64_t getBestBid() const;
    uint64_t getBestAsk() const;
};

#endif // ORDERBOOK_HPP
