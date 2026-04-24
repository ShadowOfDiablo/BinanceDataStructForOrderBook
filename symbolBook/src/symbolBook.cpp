#include "symbolBook.hpp"

void SymbolBook::display() const {
    for (const auto& [symbol, order_book] : order_books_) {
        std::cout << "Symbol: " << symbol << "\n";
        order_book.display();
    }
}

void SymbolBook::getOrderBook(const std::string& symbol) const {
    auto it = order_books_.find(symbol);
    if (it != order_books_.end()) {
        std::cout << "Symbol: " << symbol << "\n";
        it->second.display();
    } else {
        std::cout << "Symbol not found: " << symbol << "\n";
    }
}

void SymbolBook::updateOrderBook(const std::string& symbol, double price, double quantity, bool is_bid) {
    if (is_bid) {
        order_books_[symbol].updateBid(price, quantity);
    } else {
        order_books_[symbol].updateAsk(price, quantity);
    }
}