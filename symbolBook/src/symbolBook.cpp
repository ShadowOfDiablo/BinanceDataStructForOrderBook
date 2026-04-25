#include "symbolBook.hpp"

void SymbolBook::display() const {
    for (const auto& [strSymbol, sOrderBook] : orderBooks) {
        std::cout << "Symbol: " << strSymbol << "\n";
        sOrderBook.display();
    }
}

const OrderBook& SymbolBook::getOrderBook(const std::string& strSymbol) const {
    auto it = orderBooks.find(strSymbol);
    if (it == orderBooks.end())
    {
        throw std::runtime_error("Symbol not found: " + strSymbol);
    }
    return it->second;
}

void SymbolBook::updateOrderBook(const std::string& strSymbol, double dPrice, double dQuantity, bool bIsBid) {
    if (bIsBid) {
        orderBooks[strSymbol].updateBid(dPrice, dQuantity);
    } else {
        orderBooks[strSymbol].updateAsk(dPrice, dQuantity);
    }
}

void SymbolBook::handleDepthUpdate(const DepthUpdate& sUpdate)
{
    auto& sBook = orderBooks[sUpdate.strSymbol];

    if (sBook.getLastUpdateId() > sUpdate.llFinalUpdateId) {
        std::cout << "Stale update for " << sUpdate.strSymbol << ". Ignoring.\n";
        return;
    }

    if (sUpdate.llFirstUpdateId > sBook.getLastUpdateId() + 1) {
        std::cout << "Gap detected for " << sUpdate.strSymbol
                  << ". Expected " << sBook.getLastUpdateId() + 1
                  << " but got U=" << sUpdate.llFirstUpdateId << ". Need resync.\n";
        return;
    }

    for (const auto& [dPrice, dQuantity] : sUpdate.bids)
        sBook.updateBid(dPrice, dQuantity);
    for (const auto& [dPrice, dQuantity] : sUpdate.asks)
        sBook.updateAsk(dPrice, dQuantity);

    sBook.setLastUpdateId(sUpdate.llFinalUpdateId);
}