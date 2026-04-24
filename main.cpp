#include "symbolBook.hpp"
#include <iostream>

int main() {
    SymbolBook s_symbolBook;

    // Example DepthUpdate
    DepthUpdate s_update;
    s_update.str_eventType = "depthUpdate";
    s_update.lleventTime = 1672515782136;
    s_update.strSymbol = "BNBBTC";
    s_update.llFirstUpdateId = 157;
    s_update.llFinalUpdateId = 160;
    s_update.bids = {{0.0024, 10.0}};
    s_update.asks = {{0.0026, 100.0}};

    std::cout << "Handling initial depth update for BNBBTC...\n";
    s_symbolBook.handleDepthUpdate(s_update);
    s_symbolBook.display();

    // Example of an update that removes a price level (quantity = 0)
    DepthUpdate s_update2;
    s_update2.strSymbol = "BNBBTC";
    s_update2.bids = {{0.0024, 0.0}}; // Remove bid at 0.0024
    s_update2.asks = {{0.0027, 50.0}}; // Add new ask at 0.0027

    std::cout << "\nHandling subsequent depth update for BNBBTC (removing 0.0024 bid)...\n";
    s_symbolBook.handleDepthUpdate(s_update2);
    s_symbolBook.display();

    return 0;
}
