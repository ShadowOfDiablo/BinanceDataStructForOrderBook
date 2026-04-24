#pragma once

#include <map>
#include <vector>
#include <iostream>

class OrderBook {
private:
    // Bids: highest price first
    std::map<double, double, std::greater<double>> bids;
    // Asks: lowest price first
    std::map<double, double> asks;
public:
    void updateBid(double dPrice, double dQuantity);
    void updateAsk(double dPrice, double dQuantity);
    
    void display() const;
};
