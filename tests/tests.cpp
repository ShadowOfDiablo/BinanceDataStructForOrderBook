#include <gtest/gtest.h>
#include "orderbook.h"
#include "symbolBook.h"
#include <chrono>
#include <iomanip>
#include <cmath>
#include <limits>

// Prices in this codebase are stored as integer ticks: tick = price × 1e8.
// This eliminates floating-point comparison uncertainty when prices are used as
// map keys — the same tick always means the same price level, unambiguously.
//
// BNBBTC sample prices (from Binance depth stream API example used in SampleDataTest):
static constexpr uint64_t BNBBTC_0020 = 200000ULL;   // 0.0020 BTC per BNB
static constexpr uint64_t BNBBTC_0022 = 220000ULL;   // 0.0022 BTC per BNB
static constexpr uint64_t BNBBTC_0024 = 240000ULL;   // 0.0024 BTC per BNB — initial best bid/ask
static constexpr uint64_t BNBBTC_0025 = 250000ULL;   // 0.0025 BTC per BNB — new best bid in update 4
static constexpr uint64_t BNBBTC_0026 = 260000ULL;   // 0.0026 BTC per BNB — initial best ask above spread
static constexpr uint64_t BNBBTC_0027 = 270000ULL;   // 0.0027 BTC per BNB
static constexpr uint64_t BNBBTC_0028 = 280000ULL;   // 0.0028 BTC per BNB
static constexpr uint64_t BNBBTC_0030 = 300000ULL;   // 0.0030 BTC per BNB — used in snapshot reapplication test
static constexpr uint64_t BNBBTC_0032 = 320000ULL;   // 0.0032 BTC per BNB — used in snapshot reapplication test

// Generic prices for edge case and malformed-data tests (TSTUSDT, not BNBBTC):
static constexpr uint64_t PRICE_50  = 5000000000ULL;   // 50.0  — injected by malformed update, should be ignored
static constexpr uint64_t PRICE_100 = 10000000000ULL;  // 100.0 — initial bid in MalformedUpdate test
static constexpr uint64_t PRICE_101 = 10100000000ULL;  // 101.0 — initial ask in MalformedUpdate test

// Scale test prices: base 1.0 ± 0.0001 steps, used for the 2000-symbol and
// corruption-resilience tests where specific price semantics do not matter.
static constexpr uint64_t SCALE_BASE = 100000000ULL;  // 1.0 × 1e8 — midpoint of the scale test range
static constexpr uint64_t SCALE_STEP = 10000ULL;       // 0.0001 × 1e8 — spacing between adjacent levels

// OrderBook unit tests

TEST(OrderBookTest, EmptyBookBestPrices) {
    OrderBook book;
    EXPECT_EQ(book.getBestBid(), 0ULL);
    EXPECT_EQ(book.getBestAsk(), 0ULL);
}

TEST(OrderBookTest, ApplySnapshotSetsLastUpdateId) {
    OrderBook book;
    book.applySnapshot(42, {{BNBBTC_0024, 14.7}}, {{BNBBTC_0026, 3.6}});
    EXPECT_EQ(book.getLastUpdateId(), 42);
}

TEST(OrderBookTest, BestBidIsHighest) {
    OrderBook book;
    book.updateBid(BNBBTC_0020, 9.7);
    book.updateBid(BNBBTC_0024, 14.7);
    book.updateBid(BNBBTC_0022, 6.4);
    EXPECT_EQ(book.getBestBid(), BNBBTC_0024);
}

TEST(OrderBookTest, BestAskIsLowest) {
    OrderBook book;
    book.updateAsk(BNBBTC_0028, 1.0);
    book.updateAsk(BNBBTC_0024, 14.9);
    book.updateAsk(BNBBTC_0026, 3.6);
    EXPECT_EQ(book.getBestAsk(), BNBBTC_0024);
}

TEST(OrderBookTest, UpdateBidZeroQuantityRemovesLevel) {
    OrderBook book;
    book.updateBid(BNBBTC_0024, 14.7);
    book.updateBid(BNBBTC_0024, 0.0);
    EXPECT_EQ(book.getBestBid(), 0ULL);
}

TEST(OrderBookTest, UpdateAskZeroQuantityRemovesLevel) {
    OrderBook book;
    book.updateAsk(BNBBTC_0026, 3.6);
    book.updateAsk(BNBBTC_0026, 0.0);
    EXPECT_EQ(book.getBestAsk(), 0ULL);
}

TEST(OrderBookTest, UpdateBidAdjustsQuantity) {
    OrderBook book;
    book.updateBid(BNBBTC_0024, 14.7);
    book.updateBid(BNBBTC_0024, 10.0);
    EXPECT_EQ(book.getBestBid(), BNBBTC_0024);
}

// SymbolBook integration tests
class SampleDataTest : public ::testing::Test {
protected:
    SymbolBook symbolBook;
    const std::string SYMBOL = "BNBBTC";

    void SetUp() override {
        Snapshot snap;
        snap.strSymbol      = SYMBOL;
        snap.llLastUpdateId = 0;
        snap.bids = {{BNBBTC_0024, 14.7}, {BNBBTC_0022, 6.4}, {BNBBTC_0020, 9.7}};
        snap.asks = {{BNBBTC_0024, 14.9}, {BNBBTC_0026, 3.6}, {BNBBTC_0028, 1.0}};
        symbolBook.applySnapshot(snap);
    }

    DepthUpdate makeUpdate(long long U, long long u,
                           std::vector<std::pair<uint64_t, double>> bids,
                           std::vector<std::pair<uint64_t, double>> asks) {
        DepthUpdate d;
        d.str_eventType   = "depthUpdate";
        d.strSymbol       = SYMBOL;
        d.llFirstUpdateId = U;
        d.llFinalUpdateId = u;
        d.bids            = std::move(bids);
        d.asks            = std::move(asks);
        return d;
    }
};

TEST_F(SampleDataTest, AfterSnapshot_BestPrices) {
    const auto& book = symbolBook.getOrderBook(SYMBOL);
    EXPECT_EQ(book.getBestBid(), BNBBTC_0024);
    EXPECT_EQ(book.getBestAsk(), BNBBTC_0024);
}

TEST_F(SampleDataTest, Update1_AdjustLevels) {
    symbolBook.handleDepthUpdate(makeUpdate(1, 1, {{BNBBTC_0024, 10}}, {{BNBBTC_0026, 100}}));
    const auto& book = symbolBook.getOrderBook(SYMBOL);
    EXPECT_EQ(book.getBestBid(), BNBBTC_0024);
    EXPECT_EQ(book.getBestAsk(), BNBBTC_0024);
}

TEST_F(SampleDataTest, Update2_RemovesAskLevel) {
    symbolBook.handleDepthUpdate(makeUpdate(1, 1, {{BNBBTC_0024, 10}},  {{BNBBTC_0026, 100}}));
    symbolBook.handleDepthUpdate(makeUpdate(2, 2, {{BNBBTC_0024, 8}},   {{BNBBTC_0028, 0}}));
    const auto& book = symbolBook.getOrderBook(SYMBOL);
    EXPECT_EQ(book.getBestBid(), BNBBTC_0024);
    EXPECT_EQ(book.getBestAsk(), BNBBTC_0024);
}

TEST_F(SampleDataTest, Update3_RemovesBidLevel) {
    symbolBook.handleDepthUpdate(makeUpdate(1, 1, {{BNBBTC_0024, 10}},  {{BNBBTC_0026, 100}}));
    symbolBook.handleDepthUpdate(makeUpdate(2, 2, {{BNBBTC_0024, 8}},   {{BNBBTC_0028, 0}}));
    symbolBook.handleDepthUpdate(makeUpdate(3, 3, {{BNBBTC_0024, 0}},   {{BNBBTC_0026, 15}, {BNBBTC_0027, 5}}));
    const auto& book = symbolBook.getOrderBook(SYMBOL);
    EXPECT_EQ(book.getBestBid(), BNBBTC_0022);
    EXPECT_EQ(book.getBestAsk(), BNBBTC_0024);
}

TEST_F(SampleDataTest, Update4_NewBestBid) {
    symbolBook.handleDepthUpdate(makeUpdate(1, 1, {{BNBBTC_0024, 10}},  {{BNBBTC_0026, 100}}));
    symbolBook.handleDepthUpdate(makeUpdate(2, 2, {{BNBBTC_0024, 8}},   {{BNBBTC_0028, 0}}));
    symbolBook.handleDepthUpdate(makeUpdate(3, 3, {{BNBBTC_0024, 0}},   {{BNBBTC_0026, 15}, {BNBBTC_0027, 5}}));
    symbolBook.handleDepthUpdate(makeUpdate(4, 4, {{BNBBTC_0025, 100}}, {{BNBBTC_0026, 0},  {BNBBTC_0027, 5}}));
    const auto& book = symbolBook.getOrderBook(SYMBOL);
    EXPECT_EQ(book.getBestBid(), BNBBTC_0025);
    EXPECT_EQ(book.getBestAsk(), BNBBTC_0024);
}

TEST_F(SampleDataTest, Update5_FinalState) {
    symbolBook.handleDepthUpdate(makeUpdate(1, 1, {{BNBBTC_0024, 10}},  {{BNBBTC_0026, 100}}));
    symbolBook.handleDepthUpdate(makeUpdate(2, 2, {{BNBBTC_0024, 8}},   {{BNBBTC_0028, 0}}));
    symbolBook.handleDepthUpdate(makeUpdate(3, 3, {{BNBBTC_0024, 0}},   {{BNBBTC_0026, 15}, {BNBBTC_0027, 5}}));
    symbolBook.handleDepthUpdate(makeUpdate(4, 4, {{BNBBTC_0025, 100}}, {{BNBBTC_0026, 0},  {BNBBTC_0027, 5}}));
    symbolBook.handleDepthUpdate(makeUpdate(5, 5, {{BNBBTC_0025, 0}},   {{BNBBTC_0026, 15}, {BNBBTC_0024, 0}}));
    const auto& book = symbolBook.getOrderBook(SYMBOL);
    EXPECT_EQ(book.getBestBid(), BNBBTC_0022);
    EXPECT_EQ(book.getBestAsk(), BNBBTC_0026);
}

TEST_F(SampleDataTest, StaleUpdateIsIgnored) {
    symbolBook.handleDepthUpdate(makeUpdate(1, 1, {{BNBBTC_0024, 10}}, {{BNBBTC_0026, 100}}));
    // u=1 <= lastUpdateId=1 — stale, book must not change
    symbolBook.handleDepthUpdate(makeUpdate(1, 1, {{BNBBTC_0020, 999}}, {}));
    const auto& book = symbolBook.getOrderBook(SYMBOL);
    EXPECT_EQ(book.getBestBid(), BNBBTC_0024);
}

TEST_F(SampleDataTest, GapDetectedBookUnchanged) {
    // lastUpdateId=0 after snapshot; U=5 > 0+1 — gap
    symbolBook.handleDepthUpdate(makeUpdate(5, 10, {{BNBBTC_0020, 999}}, {}));
    const auto& book = symbolBook.getOrderBook(SYMBOL);
    EXPECT_EQ(book.getBestBid(), BNBBTC_0024);
}

TEST_F(SampleDataTest, UnknownSymbolThrows) {
    EXPECT_THROW(symbolBook.getOrderBook("UNKNOWN"), std::runtime_error);
}

// Edge case / data integrity tests

TEST(EdgeCaseTest, ZeroPriceIgnored) {
    OrderBook book;
    book.updateBid(0ULL, 5.0);  // tick=0 is the sentinel for "invalid price"
    EXPECT_EQ(book.getBestBid(), 0ULL);
}

TEST(EdgeCaseTest, NegativeQuantityIgnored) {
    OrderBook book;
    book.updateBid(BNBBTC_0024, -1.0);
    EXPECT_EQ(book.getBestBid(), 0ULL);
}

TEST(EdgeCaseTest, NaNQuantityIgnored) {
    OrderBook book;
    book.updateBid(BNBBTC_0024, std::numeric_limits<double>::quiet_NaN());
    EXPECT_EQ(book.getBestBid(), 0ULL);
}

TEST(EdgeCaseTest, ValidLevelNotAffectedByBadUpdate) {
    OrderBook book;
    book.updateBid(BNBBTC_0024, 10.0);
    book.updateBid(0ULL, 99.0);                                             // zero price — invalid tick
    book.updateBid(BNBBTC_0022, std::numeric_limits<double>::quiet_NaN()); // NaN qty — invalid
    EXPECT_EQ(book.getBestBid(), BNBBTC_0024);
}

TEST(EdgeCaseTest, SnapshotReapplicationClearsOldLevels) {
    OrderBook book;
    book.applySnapshot(1, {{BNBBTC_0024, 10.0}, {BNBBTC_0020, 5.0}}, {{BNBBTC_0026, 3.0}});
    // Second snapshot at different price levels — old levels must be fully replaced
    book.applySnapshot(2, {{BNBBTC_0030, 1.0}}, {{BNBBTC_0032, 1.0}});
    EXPECT_EQ(book.getBestBid(), BNBBTC_0030);
    EXPECT_EQ(book.getBestAsk(), BNBBTC_0032);
}

TEST(EdgeCaseTest, MalformedUpdateUGreaterThanFinalU) {
    // U=20 > u=15 is internally inconsistent; also a gap from lastUpdateId=10.
    // The gap check (U > lastUpdateId+1) fires first and rejects the update,
    // so PRICE_50 must never enter the book — best bid must remain PRICE_100.
    SymbolBook sb;
    Snapshot snap;
    snap.strSymbol      = "TSTUSDT";
    snap.llLastUpdateId = 10;
    snap.bids = {{PRICE_100, 1.0}};
    snap.asks = {{PRICE_101, 1.0}};
    sb.applySnapshot(snap);

    DepthUpdate bad;
    bad.strSymbol       = "TSTUSDT";
    bad.llFirstUpdateId = 20;
    bad.llFinalUpdateId = 15;
    bad.bids = {{PRICE_50, 999.0}};
    sb.handleDepthUpdate(bad);

    EXPECT_EQ(sb.getOrderBook("TSTUSDT").getBestBid(), PRICE_100);
}

TEST(EdgeCaseTest, AllCorruptLevelsInSnapshotProduceEmptyBook) {
    // All bid entries are invalid: zero tick (invalid price), negative qty, NaN qty.
    // All ask entries are invalid: zero tick, zero qty (snapshot requires qty > 0).
    // Result: both sides of the book must be empty after applySnapshot.
    OrderBook book;
    book.applySnapshot(1,
        {{0ULL, 5.0}, {1ULL, -1.0}, {2ULL, std::numeric_limits<double>::quiet_NaN()}},
        {{0ULL, 2.0}, {3ULL, 0.0}});
    EXPECT_EQ(book.getBestBid(), 0ULL);
    EXPECT_EQ(book.getBestAsk(), 0ULL);
}

TEST(EdgeCaseTest, ScaleWithCorruptedUpdates) {
    // 2000 symbols each seeded with valid levels, then hit with all-corrupt updates.
    // Best bid/ask must be unchanged — corruption must not penetrate the book.
    constexpr int NUM_SYMBOLS = 2000;
    SymbolBook sb;

    for (int i = 0; i < NUM_SYMBOLS; ++i) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "SYM%04d", i);
        Snapshot snap;
        snap.strSymbol      = buf;
        snap.llLastUpdateId = 0;
        snap.bids = {{SCALE_BASE,             10.0},  // 1.0
                     {SCALE_BASE - SCALE_STEP, 5.0}}; // 0.9999
        snap.asks = {{SCALE_BASE + SCALE_STEP, 10.0}, // 1.0001
                     {SCALE_BASE + 2 * SCALE_STEP, 5.0}}; // 1.0002
        sb.applySnapshot(snap);
    }

    for (int i = 0; i < NUM_SYMBOLS; ++i) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "SYM%04d", i);
        DepthUpdate d;
        d.strSymbol       = buf;
        d.llFirstUpdateId = 1;
        d.llFinalUpdateId = 1;
        d.bids = {{0ULL, 5.0}, {1ULL, std::numeric_limits<double>::quiet_NaN()}};
        d.asks = {{0ULL, 2.0}, {2ULL, -1.0}};
        sb.handleDepthUpdate(d);
    }

    for (int i = 0; i < NUM_SYMBOLS; ++i) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "SYM%04d", i);
        const auto& book = sb.getOrderBook(buf);
        EXPECT_EQ(book.getBestBid(), SCALE_BASE)             << buf;
        EXPECT_EQ(book.getBestAsk(), SCALE_BASE + SCALE_STEP) << buf;
    }
}

// 2000-symbol scale test (500 levels, 100 updates each)

TEST(ScaleTest, TwoThousandSymbols) {
    constexpr int NUM_SYMBOLS = 2000;
    constexpr int LEVELS      = 500;
    constexpr int UPDATES     = 100;

    SymbolBook symbolBook;

    auto t0 = std::chrono::steady_clock::now();

    for (int i = 0; i < NUM_SYMBOLS; ++i) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "SYM%04d", i);

        Snapshot snap;
        snap.strSymbol      = buf;
        snap.llLastUpdateId = 0;
        for (int l = 1; l <= LEVELS; ++l) {
            snap.bids.push_back({SCALE_BASE - l * SCALE_STEP, static_cast<double>(l)});
            snap.asks.push_back({SCALE_BASE + l * SCALE_STEP, static_cast<double>(l)});
        }
        symbolBook.applySnapshot(snap);
    }

    auto t1 = std::chrono::steady_clock::now();

    for (int i = 0; i < NUM_SYMBOLS; ++i) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "SYM%04d", i);

        for (int u = 1; u <= UPDATES; ++u) {
            DepthUpdate d;
            d.strSymbol       = buf;
            d.llFirstUpdateId = u;
            d.llFinalUpdateId = u;
            d.bids.push_back({SCALE_BASE - u * SCALE_STEP, static_cast<double>(u * 2)});
            d.asks.push_back({SCALE_BASE + u * SCALE_STEP, static_cast<double>(u * 2)});
            symbolBook.handleDepthUpdate(d);
        }
    }

    auto t2 = std::chrono::steady_clock::now();

    for (int i = 0; i < NUM_SYMBOLS; ++i) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "SYM%04d", i);
        const auto& book = symbolBook.getOrderBook(buf);
        EXPECT_GT(book.getBestBid(), 0ULL)  << "bad best bid for " << buf;
        EXPECT_GT(book.getBestAsk(), 0ULL)  << "bad best ask for " << buf;
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
