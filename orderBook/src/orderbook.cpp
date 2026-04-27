#include "orderbook.h"
#include <iomanip>
#include <cmath>

static bool isValidPrice(uint64_t p)    { return p > 0; }
static bool isValidQuantity(double q)   { return std::isfinite(q) && q >= 0.0; }

void OrderBook::updateBid(uint64_t price, double qty) {
    if (!isValidPrice(price) || !isValidQuantity(qty)) return;
    if (qty == 0.0) {
        bids.erase(price);
    } else {
        bids[price] = qty;
    }
}

void OrderBook::updateAsk(uint64_t price, double qty) {
    if (!isValidPrice(price) || !isValidQuantity(qty)) return;
    if (qty == 0.0) {
        asks.erase(price);
    } else {
        asks[price] = qty;
    }
}

void OrderBook::display() const {
    std::cout << "\n--- Order Book ---\n";
    std::cout << std::fixed << std::setprecision(8);

    std::cout << "Asks:\n";
    for (auto it = asks.rbegin(); it != asks.rend(); ++it) {
        std::cout << (static_cast<double>(it->first) / 1e8) << " : " << it->second << "\n";
    }

    std::cout << "------------------\n";

    std::cout << "Bids:\n";
    for (const auto& [price, qty] : bids) {
        std::cout << (static_cast<double>(price) / 1e8) << " : " << qty << "\n";
    }
    std::cout << "------------------\n";
}

void OrderBook::applySnapshot(int64_t snapshotLastUpdateId,
                              const std::vector<std::pair<uint64_t, double>>& snapshotBids,
                              const std::vector<std::pair<uint64_t, double>>& snapshotAsks) {
    bids.clear();
    asks.clear();
    for (const auto& [price, qty] : snapshotBids)
        if (isValidPrice(price) && qty > 0.0) bids[price] = qty;
    for (const auto& [price, qty] : snapshotAsks)
        if (isValidPrice(price) && qty > 0.0) asks[price] = qty;
    lastUpdateId = snapshotLastUpdateId;
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

uint64_t OrderBook::getBestBid() const {
    return bids.empty() ? 0 : bids.begin()->first;
}

uint64_t OrderBook::getBestAsk() const {
    return asks.empty() ? 0 : asks.begin()->first;
}
