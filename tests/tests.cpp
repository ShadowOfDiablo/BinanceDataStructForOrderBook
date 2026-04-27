#include <gtest/gtest.h>
#include "orderbook.hpp"
#include "symbolBook.hpp"
#include <chrono>
#include <iomanip>
#include <cmath>
#include <limits>

// ── OrderBook unit tests ──────────────────────────────────────────────────────

TEST(OrderBookTest, EmptyBookBestPrices) {
    OrderBook book;
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0);
    EXPECT_DOUBLE_EQ(book.getBestAsk(), 0.0);
}

TEST(OrderBookTest, ApplySnapshotSetsLastUpdateId) {
    OrderBook book;
    book.applySnapshot(42, {{0.0024, 14.7}}, {{0.0026, 3.6}});
    EXPECT_EQ(book.getLastUpdateId(), 42);
}

TEST(OrderBookTest, BestBidIsHighest) {
    OrderBook book;
    book.updateBid(0.0020, 9.7);
    book.updateBid(0.0024, 14.7);
    book.updateBid(0.0022, 6.4);
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0024);
}

TEST(OrderBookTest, BestAskIsLowest) {
    OrderBook book;
    book.updateAsk(0.0028, 1.0);
    book.updateAsk(0.0024, 14.9);
    book.updateAsk(0.0026, 3.6);
    EXPECT_DOUBLE_EQ(book.getBestAsk(), 0.0024);
}

TEST(OrderBookTest, UpdateBidZeroQuantityRemovesLevel) {
    OrderBook book;
    book.updateBid(0.0024, 14.7);
    book.updateBid(0.0024, 0.0);
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0);
}

TEST(OrderBookTest, UpdateAskZeroQuantityRemovesLevel) {
    OrderBook book;
    book.updateAsk(0.0026, 3.6);
    book.updateAsk(0.0026, 0.0);
    EXPECT_DOUBLE_EQ(book.getBestAsk(), 0.0);
}

TEST(OrderBookTest, UpdateBidAdjustsQuantity) {
    OrderBook book;
    book.updateBid(0.0024, 14.7);
    book.updateBid(0.0024, 10.0);
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0024);
}

// SymbolBook integration tests (PDF sample data) 
class SampleDataTest : public ::testing::Test {
protected:
    SymbolBook symbolBook;
    const std::string SYMBOL = "BNBBTC";

    void SetUp() override {
        Snapshot snap;
        snap.strSymbol     = SYMBOL;
        snap.llLastUpdateId = 0;
        snap.bids = {{0.0024, 14.7}, {0.0022, 6.4}, {0.0020, 9.7}};
        snap.asks = {{0.0024, 14.9}, {0.0026, 3.6}, {0.0028, 1.0}};
        symbolBook.applySnapshot(snap);
    }

    DepthUpdate makeUpdate(long long U, long long u,
                           std::vector<std::pair<double,double>> bids,
                           std::vector<std::pair<double,double>> asks) {
        DepthUpdate d;
        d.str_eventType    = "depthUpdate";
        d.strSymbol        = SYMBOL;
        d.llFirstUpdateId  = U;
        d.llFinalUpdateId  = u;
        d.bids             = std::move(bids);
        d.asks             = std::move(asks);
        return d;
    }
};

TEST_F(SampleDataTest, AfterSnapshot_BestPrices) {
    const auto& book = symbolBook.getOrderBook(SYMBOL);
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0024);
    EXPECT_DOUBLE_EQ(book.getBestAsk(), 0.0024);
}

TEST_F(SampleDataTest, Update1_AdjustLevels) {
    symbolBook.handleDepthUpdate(makeUpdate(1, 1, {{0.0024, 10}}, {{0.0026, 100}}));
    const auto& book = symbolBook.getOrderBook(SYMBOL);
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0024);
    EXPECT_DOUBLE_EQ(book.getBestAsk(), 0.0024);
}

TEST_F(SampleDataTest, Update2_RemovesAskLevel) {
    symbolBook.handleDepthUpdate(makeUpdate(1, 1, {{0.0024, 10}},  {{0.0026, 100}}));
    symbolBook.handleDepthUpdate(makeUpdate(2, 2, {{0.0024, 8}},   {{0.0028, 0}}));
    const auto& book = symbolBook.getOrderBook(SYMBOL);
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0024);
    EXPECT_DOUBLE_EQ(book.getBestAsk(), 0.0024);
}

TEST_F(SampleDataTest, Update3_RemovesBidLevel) {
    symbolBook.handleDepthUpdate(makeUpdate(1, 1, {{0.0024, 10}},  {{0.0026, 100}}));
    symbolBook.handleDepthUpdate(makeUpdate(2, 2, {{0.0024, 8}},   {{0.0028, 0}}));
    symbolBook.handleDepthUpdate(makeUpdate(3, 3, {{0.0024, 0}},   {{0.0026, 15}, {0.0027, 5}}));
    const auto& book = symbolBook.getOrderBook(SYMBOL);
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0022);
    EXPECT_DOUBLE_EQ(book.getBestAsk(), 0.0024);
}

TEST_F(SampleDataTest, Update4_NewBestBid) {
    symbolBook.handleDepthUpdate(makeUpdate(1, 1, {{0.0024, 10}},  {{0.0026, 100}}));
    symbolBook.handleDepthUpdate(makeUpdate(2, 2, {{0.0024, 8}},   {{0.0028, 0}}));
    symbolBook.handleDepthUpdate(makeUpdate(3, 3, {{0.0024, 0}},   {{0.0026, 15}, {0.0027, 5}}));
    symbolBook.handleDepthUpdate(makeUpdate(4, 4, {{0.0025, 100}}, {{0.0026, 0},  {0.0027, 5}}));
    const auto& book = symbolBook.getOrderBook(SYMBOL);
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0025);
    EXPECT_DOUBLE_EQ(book.getBestAsk(), 0.0024);
}

TEST_F(SampleDataTest, Update5_FinalState) {
    symbolBook.handleDepthUpdate(makeUpdate(1, 1, {{0.0024, 10}},  {{0.0026, 100}}));
    symbolBook.handleDepthUpdate(makeUpdate(2, 2, {{0.0024, 8}},   {{0.0028, 0}}));
    symbolBook.handleDepthUpdate(makeUpdate(3, 3, {{0.0024, 0}},   {{0.0026, 15}, {0.0027, 5}}));
    symbolBook.handleDepthUpdate(makeUpdate(4, 4, {{0.0025, 100}}, {{0.0026, 0},  {0.0027, 5}}));
    symbolBook.handleDepthUpdate(makeUpdate(5, 5, {{0.0025, 0}},   {{0.0026, 15}, {0.0024, 0}}));
    const auto& book = symbolBook.getOrderBook(SYMBOL);
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0022);
    EXPECT_DOUBLE_EQ(book.getBestAsk(), 0.0026);
}

TEST_F(SampleDataTest, StaleUpdateIsIgnored) {
    symbolBook.handleDepthUpdate(makeUpdate(1, 1, {{0.0024, 10}}, {{0.0026, 100}}));
    // u=1 <= lastUpdateId=1 - stale, book must not change
    symbolBook.handleDepthUpdate(makeUpdate(1, 1, {{0.0020, 999}}, {}));
    const auto& book = symbolBook.getOrderBook(SYMBOL);
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0024);
}

TEST_F(SampleDataTest, GapDetectedBookUnchanged) {
    // lastUpdateId=0 after snapshot; U=5 > 0+1 - gap
    symbolBook.handleDepthUpdate(makeUpdate(5, 10, {{0.0020, 999}}, {}));
    const auto& book = symbolBook.getOrderBook(SYMBOL);
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0024);
}

TEST_F(SampleDataTest, UnknownSymbolThrows) {
    EXPECT_THROW(symbolBook.getOrderBook("UNKNOWN"), std::runtime_error);
}

// ── Edge case / data integrity tests ─────────────────────────────────────────

TEST(EdgeCaseTest, NegativePriceIgnored) {
    OrderBook book;
    book.updateBid(-1.0, 5.0);
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0);
}

TEST(EdgeCaseTest, ZeroPriceIgnored) {
    OrderBook book;
    book.updateBid(0.0, 5.0);
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0);
}

TEST(EdgeCaseTest, NaNPriceIgnored) {
    OrderBook book;
    book.updateBid(std::numeric_limits<double>::quiet_NaN(), 5.0);
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0);
}

TEST(EdgeCaseTest, InfinitePriceIgnored) {
    OrderBook book;
    book.updateBid(std::numeric_limits<double>::infinity(), 5.0);
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0);
}

TEST(EdgeCaseTest, NegativeQuantityIgnored) {
    OrderBook book;
    book.updateBid(0.0024, -1.0);
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0);
}

TEST(EdgeCaseTest, NaNQuantityIgnored) {
    OrderBook book;
    book.updateBid(0.0024, std::numeric_limits<double>::quiet_NaN());
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0);
}

TEST(EdgeCaseTest, ValidLevelNotAffectedByBadUpdate) {
    OrderBook book;
    book.updateBid(0.0024, 10.0);
    book.updateBid(-1.0, 99.0);   // bad — must not corrupt the book
    book.updateBid(std::numeric_limits<double>::quiet_NaN(), 5.0);
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0024);
}

TEST(EdgeCaseTest, SnapshotReapplicationClearsOldLevels) {
    OrderBook book;
    book.applySnapshot(1, {{0.0024, 10.0}, {0.0020, 5.0}}, {{0.0026, 3.0}});
    // Reapply with different levels — old ones must be gone
    book.applySnapshot(2, {{0.0030, 1.0}}, {{0.0032, 1.0}});
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0030);
    EXPECT_DOUBLE_EQ(book.getBestAsk(), 0.0032);
}

TEST(EdgeCaseTest, MalformedUpdateUGreaterThanFinalU) {
    // U > u is internally inconsistent — the update should be discarded
    // (gap check: U > lastUpdateId+1 catches it when lastUpdateId=0 and U=5)
    SymbolBook sb;
    Snapshot snap;
    snap.strSymbol = "TSTUSDT";
    snap.llLastUpdateId = 10;
    snap.bids = {{100.0, 1.0}};
    snap.asks = {{101.0, 1.0}};
    sb.applySnapshot(snap);

    DepthUpdate bad;
    bad.strSymbol       = "TSTUSDT";
    bad.llFirstUpdateId = 20;  // U=20 > u=15, internally malformed
    bad.llFinalUpdateId = 15;  // also a gap from lastUpdateId=10
    bad.bids = {{50.0, 999.0}};
    sb.handleDepthUpdate(bad);

    EXPECT_DOUBLE_EQ(sb.getOrderBook("TSTUSDT").getBestBid(), 100.0);
}

TEST(EdgeCaseTest, AllCorruptLevelsInSnapshotProduceEmptyBook) {
    OrderBook book;
    book.applySnapshot(1,
        {{-1.0, 5.0}, {0.0, 3.0}, {std::numeric_limits<double>::quiet_NaN(), 1.0}},
        {{std::numeric_limits<double>::infinity(), 2.0}});
    EXPECT_DOUBLE_EQ(book.getBestBid(), 0.0);
    EXPECT_DOUBLE_EQ(book.getBestAsk(), 0.0);
}

TEST(EdgeCaseTest, ScaleWithCorruptedUpdates) {
    constexpr int NUM_SYMBOLS = 2000;
    SymbolBook sb;

    for (int i = 0; i < NUM_SYMBOLS; ++i) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "SYM%04d", i);
        Snapshot snap;
        snap.strSymbol      = buf;
        snap.llLastUpdateId = 0;
        snap.bids = {{1.0, 10.0}, {0.9, 5.0}};
        snap.asks = {{1.1, 10.0}, {1.2, 5.0}};
        sb.applySnapshot(snap);
    }

    // Fire corrupt updates at every symbol — book must stay intact
    for (int i = 0; i < NUM_SYMBOLS; ++i) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "SYM%04d", i);
        DepthUpdate d;
        d.strSymbol       = buf;
        d.llFirstUpdateId = 1;
        d.llFinalUpdateId = 1;
        d.bids = {{-1.0, 5.0}, {std::numeric_limits<double>::quiet_NaN(), 3.0}};
        d.asks = {{0.0,  2.0}, {std::numeric_limits<double>::infinity(), 1.0}};
        sb.handleDepthUpdate(d);
    }

    for (int i = 0; i < NUM_SYMBOLS; ++i) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "SYM%04d", i);
        const auto& book = sb.getOrderBook(buf);
        EXPECT_DOUBLE_EQ(book.getBestBid(), 1.0) << buf;
        EXPECT_DOUBLE_EQ(book.getBestAsk(), 1.1) << buf;
    }
}

// 2000-symbol scale test (with 500 levels and 100 updates each) to verify performance and memory usage.

TEST(ScaleTest, TwoThousandSymbols) {
    constexpr int NUM_SYMBOLS  = 2000;
    constexpr int LEVELS       = 500;   // price levels per side in snapshot
    constexpr int UPDATES      = 100;   // diff updates per symbol

    SymbolBook symbolBook;

    auto t0 = std::chrono::steady_clock::now();

    // Apply snapshot to every symbol
    for (int i = 0; i < NUM_SYMBOLS; ++i) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "SYM%04d", i);

        Snapshot snap;
        snap.strSymbol      = buf;
        snap.llLastUpdateId = 0;
        for (int l = 1; l <= LEVELS; ++l) {
            snap.bids.push_back({1.0 - l * 0.0001, static_cast<double>(l)});
            snap.asks.push_back({1.0 + l * 0.0001, static_cast<double>(l)});
        }
        symbolBook.applySnapshot(snap);
    }

    auto t1 = std::chrono::steady_clock::now();

    // Apply UPDATES diffs to every symbol
    for (int i = 0; i < NUM_SYMBOLS; ++i) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "SYM%04d", i);

        for (int u = 1; u <= UPDATES; ++u) {
            DepthUpdate d;
            d.strSymbol       = buf;
            d.llFirstUpdateId = u;
            d.llFinalUpdateId = u;
            d.bids.push_back({1.0 - u * 0.0001, static_cast<double>(u * 2)});
            d.asks.push_back({1.0 + u * 0.0001, static_cast<double>(u * 2)});
            symbolBook.handleDepthUpdate(d);
        }
    }

    auto t2 = std::chrono::steady_clock::now();

    // Verify every symbol has sane best prices
    for (int i = 0; i < NUM_SYMBOLS; ++i) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "SYM%04d", i);
        const auto& book = symbolBook.getOrderBook(buf);
        EXPECT_GT(book.getBestBid(), 0.0)  << "bad best bid for " << buf;
        EXPECT_GT(book.getBestAsk(), 0.0)  << "bad best ask for " << buf;
        EXPECT_LT(book.getBestBid(), book.getBestAsk()) << "crossed book for " << buf;
    }

    auto snapshotMs = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    auto updatesMs  = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    std::cout << "\n"
              << "[ TIMING  ] Snapshot  : " << NUM_SYMBOLS << " symbols x "
              << LEVELS << " levels = " << snapshotMs << " ms\n"
              << "[ TIMING  ] Updates   : " << NUM_SYMBOLS << " symbols x "
              << UPDATES << " diffs   = " << updatesMs << " ms"
              << "  (" << std::fixed << std::setprecision(3)
              << (double(updatesMs) / (NUM_SYMBOLS * UPDATES) * 1000.0)
              << " us/update)\n";
}
