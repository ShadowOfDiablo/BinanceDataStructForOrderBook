#include "symbolBook.hpp"
#include <iostream>

int main() {
    SymbolBook symbol_book;
    symbol_book.updateOrderBook("AAPL", 150.0, 100.0, true);
    symbol_book.updateOrderBook("AAPL", 151.0, 50.0, false);
    symbol_book.updateOrderBook("GOOG", 2800.0, 10.0, true);
    symbol_book.updateOrderBook("GOOG", 2810.0, 5.0, false);

    std::cout << "Displaying all order books:\n";
    symbol_book.display();
    std::cout << "\nGetting order book for AAPL:\n";
    symbol_book.getOrderBook("AAPL");
    std::cout << "\nGetting order book for GOOG:\n";
    symbol_book.getOrderBook("GOOG");

    return 0;
}
