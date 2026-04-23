#include "orderbook.hpp"
#include <iomanip>

void OrderBook::updateBid(double price, double quantity) {
    if (quantity == 0) {
        bids_.erase(price);
    } else {
        bids_[price] = quantity;
    }
}

void OrderBook::updateAsk(double price, double quantity) {
    if (quantity == 0) {
        asks_.erase(price);
    } else {
        asks_[price] = quantity;
    }
}

void OrderBook::display() const {
    std::cout << "\n--- Order Book ---\n";
    std::cout << std::fixed << std::setprecision(4);
    
    std::cout << "Asks:\n";
    for (auto it = asks_.rbegin(); it != asks_.rend(); ++it) {
        std::cout << it->first << " : " << it->second << "\n";
    }
    
    std::cout << "------------------\n";
    
    std::cout << "Bids:\n";
    for (const auto& [price, qty] : bids_) {
        std::cout << price << " : " << qty << "\n";
    }
    std::cout << "------------------\n";
}
