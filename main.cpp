#include "symbolBook.h"
#include "json.hpp"
#include <iostream>
#include <string>
#include <cmath>
#include <iomanip>

using json = nlohmann::json;

// Convert a Binance price string to uint64_t ticks (price × 1e8).
// Returns 0 for any invalid input — 0 is rejected by isValidPrice.
static uint64_t toTick(const std::string& s) {
    double d = std::stod(s);
    if (!std::isfinite(d) || d <= 0.0) return 0;
    return static_cast<uint64_t>(std::round(d * 1e8));
}

// Parse one [price, qty] level. Returns false on any structural or numeric
// failure so the caller can skip that level without aborting the whole message.
static bool parseLevel(const json& level, uint64_t& price, double& qty) {
    if (!level.is_array() || level.size() < 2) return false;
    if (!level[0].is_string() || !level[1].is_string()) return false;
    try {
        price = toTick(level[0].get<std::string>());
        qty   = std::stod(level[1].get<std::string>());
    } catch (...) {
        return false;
    }
    return price != 0 && std::isfinite(qty) && qty >= 0.0;
}

static void printBook(const OrderBook& book, const std::string& symbol, long long updateId) {
    std::cout << std::fixed << std::setprecision(8);
    std::cout << "[" << symbol << " u=" << updateId << "]"
              << "  Best Bid: " << (book.getBestBid() / 1e8)
              << "  |  Best Ask: " << (book.getBestAsk() / 1e8) << "\n";
}

int main() {
    SymbolBook s_symbolBook;
    std::string str_line;

    std::cout << "Order book engine ready. Pipe JSON lines to stdin.\n"
              << "  Snapshot format : {\"s\":\"SYMBOL\",\"bids\":[[\"price\",\"qty\"],...],\"asks\":[...]}\n"
              << "  Update format   : {\"e\":\"depthUpdate\",\"s\":\"SYMBOL\",\"U\":1,\"u\":1,\"b\":[...],\"a\":[...]}\n\n";

    while (std::getline(std::cin, str_line)) {
        // Trim leading/trailing whitespace
        str_line.erase(0, str_line.find_first_not_of(" \t\r\n"));
        str_line.erase(str_line.find_last_not_of(" \t\r\n") + 1);

        if (str_line.empty()) continue;

        // If the line doesn't start with '{', it might be a prefix like "Update 1:".
        // Try to find the first '{' and parse from there.
        size_t start_pos = str_line.find('{');
        if (start_pos == std::string::npos) continue;
        std::string json_part = str_line.substr(start_pos);

        try {
            auto j = json::parse(json_part);

            // Snapshot: has "bids"/"asks" keys, no "e" event-type field.
            if ((j.contains("bids") || j.contains("asks")) && !j.contains("e")) {
                std::string symbol = j.value("s", j.value("symbol", ""));
                if (symbol.empty()) {
                    throw std::runtime_error("MALFORMED DATA: Snapshot missing mandatory symbol field ('s' or 'symbol')");
                }

                Snapshot s_snapshot;
                s_snapshot.strSymbol      = symbol;
                s_snapshot.llLastUpdateId = j.value("lastUpdateId", 0LL);

                for (const auto& bid : j.value("bids", json::array())) {
                    uint64_t price; double qty;
                    if (parseLevel(bid, price, qty)) s_snapshot.bids.push_back({price, qty});
                }
                for (const auto& ask : j.value("asks", json::array())) {
                    uint64_t price; double qty;
                    if (parseLevel(ask, price, qty)) s_snapshot.asks.push_back({price, qty});
                }

                s_symbolBook.applySnapshot(s_snapshot);

                std::cout << "=== Snapshot: " << s_snapshot.strSymbol
                          << " (" << s_snapshot.bids.size() << " bids, "
                          << s_snapshot.asks.size() << " asks) ===\n";
                s_symbolBook.getOrderBook(s_snapshot.strSymbol).display();
                std::cout << "\n";

            } else {
                // Depth update: Binance wire format — "b"/"a" for bids/asks.
                // Skip non-depthUpdate frames (subscribe acks, error payloads, etc.)
                // so they don't get misrouted into the book.
                std::string eventType = j.value("e", "");
                if (eventType != "depthUpdate") continue;

                std::string symbol = j.value("s", "");
                if (symbol.empty()) {
                    throw std::runtime_error("MALFORMED DATA: Update missing mandatory symbol field ('s')");
                }

                DepthUpdate s_update;
                s_update.str_eventType   = eventType;
                s_update.lleventTime     = j.value("E", 0LL);
                s_update.strSymbol       = symbol;
                s_update.llFirstUpdateId = j.value("U", 0LL);
                s_update.llFinalUpdateId = j.value("u", 0LL);

                for (const auto& bid : j.value("b", json::array())) {
                    uint64_t price; double qty;
                    if (parseLevel(bid, price, qty)) s_update.bids.push_back({price, qty});
                }
                for (const auto& ask : j.value("a", json::array())) {
                    uint64_t price; double qty;
                    if (parseLevel(ask, price, qty)) s_update.asks.push_back({price, qty});
                }

                s_symbolBook.handleDepthUpdate(s_update);

                try {
                    printBook(s_symbolBook.getOrderBook(s_update.strSymbol),
                              s_update.strSymbol, s_update.llFinalUpdateId);
                } catch (...) {}
            }

        } catch (const std::exception& e) {
            std::cerr << "Parse error: " << e.what() << "\n";
        }
    }

    return 0;
}
