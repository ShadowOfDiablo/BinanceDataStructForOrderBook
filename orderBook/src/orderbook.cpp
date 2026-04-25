#include "orderbook.hpp"
#include <iomanip>

void OrderBook::updateBid(double dPrice, double dQuantity) {
    if (dQuantity == 0) {
        bids.erase(dPrice);
    } else {
        bids[dPrice] = dQuantity;
    }
}

void OrderBook::updateAsk(double dPrice, double dQuantity) {
    if (dQuantity == 0) {
        asks.erase(dPrice);
    } else {
        asks[dPrice] = dQuantity;
    }
}

void OrderBook::display() const {
    std::cout << "\n--- Order Book ---\n";
    std::cout << std::fixed << std::setprecision(4);
    
    std::cout << "Asks:\n";
    for (auto it = asks.rbegin(); it != asks.rend(); ++it) {
        std::cout << it->first << " : " << it->second << "\n";
    }
    
    std::cout << "------------------\n";
    
    std::cout << "Bids:\n";
    for (const auto& [dPrice, dQuantity] : bids) {
        std::cout << dPrice << " : " << dQuantity << "\n";
    }
    std::cout << "------------------\n";
}

void OrderBook::setLastUpdateId(int64_t id) {
    lastUpdateId = id;
}

int64_t OrderBook::getLastUpdateId() const {
    return lastUpdateId;
}

OrderBook::OrderBook() : lastUpdateId(0) {
}

OrderBook::~OrderBook() {
    bids.clear();
    asks.clear();
    lastUpdateId = 0;
}

OrderBook::OrderBook(const OrderBook& source) {
    lastUpdateId = source.lastUpdateId;
    bids = source.bids;
    asks = source.asks;
}

OrderBook& OrderBook::operator=(const OrderBook& source) {
    if (this != &source) {
        lastUpdateId = source.lastUpdateId;
        bids = source.bids;
        asks = source.asks;
    }
    return *this;
}

bool OrderBook::operator==(const OrderBook& other) const {
    return lastUpdateId == other.lastUpdateId && bids == other.bids && asks == other.asks;
}