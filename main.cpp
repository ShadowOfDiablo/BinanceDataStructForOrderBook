#include "orderbook.hpp"
#include <iostream>

int main() {
    OrderBook book;

    // Add some sample data
    book.updateBid(100.5, 10.0);
    book.updateBid(100.4, 15.0);
    book.updateAsk(100.6, 8.0);
    book.updateAsk(100.7, 12.0);

    std::cout << "Initial Book:" << std::endl;
    book.display();

    // Update a level
    std::cout << "\nUpdating Bid 100.5 to quantity 5.0..." << std::endl;
    book.updateBid(100.5, 5.0);
    
    // Remove a level
    std::cout << "Removing Ask 100.7..." << std::endl;
    book.updateAsk(100.7, 0.0);

    book.display();

    return 0;
}
