#include "symbolBook.hpp"

void SymbolBook::display() const {
    for (const auto& [strSymbol, sOrderBook] : orderBooks) {
        std::cout << "Symbol: " << strSymbol << "\n";
        sOrderBook.display();
    }
}

void SymbolBook::getOrderBook(const std::string& strSymbol) const {
    auto it = orderBooks.find(strSymbol);
    if (it != orderBooks.end()) {
        std::cout << "Symbol: " << strSymbol << "\n";
        it->second.display();
    } else {
        std::cout << "Symbol not found: " << strSymbol << "\n";
    }
}

void SymbolBook::updateOrderBook(const std::string& strSymbol, double dPrice, double dQuantity, bool bIsBid) {
    if (bIsBid) {
        orderBooks[strSymbol].updateBid(dPrice, dQuantity);
    } else {
        orderBooks[strSymbol].updateAsk(dPrice, dQuantity);
    }
}

void SymbolBook::handleDepthUpdate(const DepthUpdate& sUpdate) {
    auto& sBook = orderBooks[sUpdate.strSymbol];
    for (const auto& [dPrice, dQuantity] : sUpdate.bids) {
        sBook.updateBid(dPrice, dQuantity);
    }
    for (const auto& [dPrice, dQuantity] : sUpdate.asks) {
        sBook.updateAsk(dPrice, dQuantity);
    }
}
