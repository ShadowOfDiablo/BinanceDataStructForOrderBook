#include "symbolBook.hpp"
#include "json.hpp"
#include <iostream>
#include <string>

using json = nlohmann::json;

int main() {
    SymbolBook s_symbolBook;
    std::string str_line;

    std::cout << "Order Book Engine started. Waiting for JSON input on stdin " << std::endl;

    while (std::getline(std::cin, str_line)) {
        if (str_line.empty()) continue;

        try {
            auto j = json::parse(str_line);

            // Map Binance JSON to DepthUpdate struct
            DepthUpdate s_update;
            s_update.str_eventType = j.value("e", "");
            s_update.lleventTime = j.value("E", 0LL);
            s_update.strSymbol = j.value("s", "");
            s_update.llFirstUpdateId = j.value("U", 0LL);
            s_update.llFinalUpdateId = j.value("u", 0LL);

            // Parse Bids
            if (j.contains("b") && j["b"].is_array()) {
                for (const auto& bid : j["b"]) {
                    double dPrice = std::stod(bid[0].get<std::string>());
                    double dQuantity = std::stod(bid[1].get<std::string>());
                    s_update.bids.push_back({dPrice, dQuantity});
                }
            }

            // Parse Asks
            if (j.contains("a") && j["a"].is_array()) {
                for (const auto& ask : j["a"]) {
                    double dPrice = std::stod(ask[0].get<std::string>());
                    double dQuantity = std::stod(ask[1].get<std::string>());
                    s_update.asks.push_back({dPrice, dQuantity});
                }
            }

            s_symbolBook.handleDepthUpdate(s_update);
            
            // Clear screen and display for visualization (optional)
            // std::cout << "\033[2J\033[H"; 
            std::cout << "Update received for " << s_update.strSymbol << " (" << s_update.lleventTime << ")" << std::endl;
            s_symbolBook.getOrderBook(s_update.strSymbol);

        } catch (const std::exception& e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        }
    }

    return 0;
}
