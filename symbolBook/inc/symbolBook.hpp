#include <string>
#include <unordered_map>
#include "orderbook.hpp"
class SymbolBook {
private:
    std::unordered_map<std::string, OrderBook> order_books_;

public:
    void display() const;
    void getOrderBook(const std::string& symbol) const;
    void updateOrderBook(const std::string& symbol, double price, double quantity, bool is_bid);
    
};