#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <cstdint>
#include <map>
#include <vector>
#include <iostream>

class OrderBook {
public:
    // Normal mode: 20 levels per side — matches Binance's @depth20 partial stream cap.
    // Extended mode (-DEXTENDED_DEPTH=ON): no cap; Binance snapshots go up to 5000 levels/side.
#ifndef EXTENDED_DEPTH
    static constexpr std::size_t MAX_DEPTH = 20;
#endif

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
    ~OrderBook() = default;
    OrderBook(const OrderBook& source);
    OrderBook& operator=(const OrderBook& source);
    uint64_t getBestBid() const;
    uint64_t getBestAsk() const;
};

#endif // ORDERBOOK_HPP
