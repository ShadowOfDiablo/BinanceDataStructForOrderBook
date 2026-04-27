#include "symbolBook.hpp"
#include "json.hpp"
#include <iostream>
#include <string>
#include <cmath>

using json = nlohmann::json;

// Convert a Binance price string to uint64_t ticks (price × 1e8).
// Returns 0 for any invalid input — 0 is rejected by isValidPrice.
static uint64_t toTick(const std::string& s) {
    double d = std::stod(s);
    if (!std::isfinite(d) || d <= 0.0)
    { 
        return 0;
    }
    return static_cast<uint64_t>(std::round(d * 1e8));
}

int main() {
    SymbolBook s_symbolBook;
    std::string str_line;

    std::cout << "Order Book Engine started. Waiting for JSON input on stdin " << std::endl;

    while (std::getline(std::cin, str_line)) {
        if (str_line.empty()) continue;

        try {
            auto j = json::parse(str_line);

            if (j.value("type", "") == "snapshot") {
                Snapshot s_snapshot;
                s_snapshot.strSymbol = j.value("symbol", "");
                s_snapshot.llLastUpdateId = j.value("lastUpdateId", 0LL);
                for (const auto& bid : j["bids"]) {
                    if (!bid.is_array() || bid.size() < 2) continue;
                    s_snapshot.bids.push_back({toTick(bid[0].get<std::string>()),
                                               std::stod(bid[1].get<std::string>())});
                }
                for (const auto& ask : j["asks"]) {
                    if (!ask.is_array() || ask.size() < 2) continue;
                    s_snapshot.asks.push_back({toTick(ask[0].get<std::string>()),
                                               std::stod(ask[1].get<std::string>())});
                }
                s_symbolBook.applySnapshot(s_snapshot);
            } else {
                DepthUpdate s_update;
                s_update.str_eventType = j.value("e", "");
                s_update.lleventTime = j.value("E", 0LL);
                s_update.strSymbol = j.value("s", "");
                s_update.llFirstUpdateId = j.value("U", 0LL);
                s_update.llFinalUpdateId = j.value("u", 0LL);

                for (const auto& bid : j.value("b", json::array())) {
                    if (!bid.is_array() || bid.size() < 2) continue;
                    s_update.bids.push_back({toTick(bid[0].get<std::string>()),
                                             std::stod(bid[1].get<std::string>())});
                }
                for (const auto& ask : j.value("a", json::array())) {
                    if (!ask.is_array() || ask.size() < 2) continue;
                    s_update.asks.push_back({toTick(ask[0].get<std::string>()),
                                             std::stod(ask[1].get<std::string>())});
                }

                s_symbolBook.handleDepthUpdate(s_update);
                // std::cerr << "Update applied for " << s_update.strSymbol
                //           << " u=" << s_update.llFinalUpdateId << "\n";
            }

        } catch (const std::exception& e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        }
    }

    return 0;
}
