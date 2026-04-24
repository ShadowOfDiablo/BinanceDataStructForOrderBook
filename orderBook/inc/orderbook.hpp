#pragma once

#include <map>
#include <vector>
#include <iostream>

struct PriceLevel {
    double price;
    double quantity;
};

class OrderBook {
private:
    // Bids: highest price first
    std::map<double, double, std::greater<double>> bids_;
    // Asks: lowest price first
    std::map<double, double> asks_;
public:
    void updateBid(double price, double quantity);
    void updateAsk(double price, double quantity);
    
    void display() const;
};
