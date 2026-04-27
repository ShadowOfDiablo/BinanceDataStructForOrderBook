#include <gtest/gtest.h>
#include "orderbook.h"
#include "symbolBook.h"
#include <chrono>
#include <iomanip>
#include <cmath>
#include <limits>
#include <fstream>

static double rssmegabytes() {
    std::ifstream f("/proc/self/status");
    std::string line;
    while (std::getline(f, line)) {
        if (line.rfind("VmRSS:", 0) == 0) {
            long kb = 0;
            std::sscanf(line.c_str(), "VmRSS: %ld", &kb);
            return kb / 1024.0;
        }
    }
    return -1.0;
}

// Prevents dead-code elimination of timing loops.
static volatile uint64_t gSink = 0;

// Prices are stored as integer ticks (price × 1e8) for exact map-key comparison.
// BNBBTC sample prices used by SampleDataTest:
static constexpr uint64_t BNBBTC_0020 = 200000ULL;   // 0.0020
static constexpr uint64_t BNBBTC_0022 = 220000ULL;   // 0.0022
static constexpr uint64_t BNBBTC_0024 = 240000ULL;   // 0.0024
static constexpr uint64_t BNBBTC_0025 = 250000ULL;   // 0.0025
static constexpr uint64_t BNBBTC_0026 = 260000ULL;   // 0.0026
static constexpr uint64_t BNBBTC_0027 = 270000ULL;   // 0.0027
static constexpr uint64_t BNBBTC_0028 = 280000ULL;   // 0.0028
static constexpr uint64_t BNBBTC_0030 = 300000ULL;   // 0.0030
static constexpr uint64_t BNBBTC_0032 = 320000ULL;   // 0.0032

// Generic prices for malformed-data tests:
static constexpr uint64_t PRICE_50  = 5000000000ULL;
static constexpr uint64_t PRICE_100 = 10000000000ULL;
static constexpr uint64_t PRICE_101 = 10100000000ULL;

// Scale test: 1.0 base, 0.0001 step.
static constexpr uint64_t SCALE_BASE = 100000000ULL;
static constexpr uint64_t SCALE_STEP = 10000ULL;

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
    // u=4 < lastUpdateId=5 — must be dropped.
    symbolBook.handleDepthUpdate(makeUpdate(1, 5, {{BNBBTC_0024, 10}}, {{BNBBTC_0026, 100}}));
    symbolBook.handleDepthUpdate(makeUpdate(1, 4, {{BNBBTC_0020, 999}}, {}));
    EXPECT_EQ(symbolBook.getOrderBook(SYMBOL).getBestBid(), BNBBTC_0024);
}

TEST_F(SampleDataTest, GapDiscardsBookAndRequiresResnapshot) {
    // U=5 > lastUpdateId(0)+1 — gap discards the book; resync requires a fresh snapshot.
    symbolBook.handleDepthUpdate(makeUpdate(5, 10, {{BNBBTC_0020, 999}}, {}));
    EXPECT_THROW(symbolBook.getOrderBook(SYMBOL), std::runtime_error);

    symbolBook.handleDepthUpdate(makeUpdate(11, 11, {{BNBBTC_0024, 1}}, {}));
    EXPECT_THROW(symbolBook.getOrderBook(SYMBOL), std::runtime_error);

    Snapshot resync;
    resync.strSymbol      = SYMBOL;
    resync.llLastUpdateId = 100;
    resync.bids = {{BNBBTC_0030, 1.0}};
    resync.asks = {{BNBBTC_0032, 1.0}};
    symbolBook.applySnapshot(resync);
    EXPECT_EQ(symbolBook.getOrderBook(SYMBOL).getBestBid(), BNBBTC_0030);
    EXPECT_EQ(symbolBook.getOrderBook(SYMBOL).getBestAsk(), BNBBTC_0032);
}

TEST_F(SampleDataTest, UnknownSymbolThrows) {
    EXPECT_THROW(symbolBook.getOrderBook("UNKNOWN"), std::runtime_error);
}

TEST(EdgeCaseTest, UpdateBeforeSnapshotRejected) {
    // No snapshot first — both forms must drop without leaking a stub entry.
    SymbolBook sb;
    DepthUpdate u;
    u.strSymbol       = "GHOST";
    u.llFirstUpdateId = 1;
    u.llFinalUpdateId = 1;
    u.bids = {{PRICE_100, 1.0}};
    sb.handleDepthUpdate(u);
    EXPECT_THROW(sb.getOrderBook("GHOST"), std::runtime_error);

    DepthUpdate noSeq;
    noSeq.strSymbol = "GHOST2";
    noSeq.bids = {{PRICE_100, 1.0}};
    sb.handleDepthUpdate(noSeq);
    EXPECT_THROW(sb.getOrderBook("GHOST2"), std::runtime_error);
}

TEST(EdgeCaseTest, ZeroPriceIgnored) {
    OrderBook book;
    book.updateBid(0ULL, 5.0);  // tick 0 is the invalid-price sentinel
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
    book.updateBid(0ULL, 99.0);
    book.updateBid(BNBBTC_0022, std::numeric_limits<double>::quiet_NaN());
    EXPECT_EQ(book.getBestBid(), BNBBTC_0024);
}

TEST(EdgeCaseTest, SnapshotReapplicationClearsOldLevels) {
    OrderBook book;
    book.applySnapshot(1, {{BNBBTC_0024, 10.0}, {BNBBTC_0020, 5.0}}, {{BNBBTC_0026, 3.0}});
    book.applySnapshot(2, {{BNBBTC_0030, 1.0}}, {{BNBBTC_0032, 1.0}});
    EXPECT_EQ(book.getBestBid(), BNBBTC_0030);
    EXPECT_EQ(book.getBestAsk(), BNBBTC_0032);
}

TEST(EdgeCaseTest, MalformedUpdateUGreaterThanFinalU) {
    // U=20, u=15 with lastUpdateId=10 — gap fires first, book is discarded.
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

    EXPECT_THROW(sb.getOrderBook("TSTUSDT"), std::runtime_error);
}

TEST(EdgeCaseTest, AllCorruptLevelsInSnapshotProduceEmptyBook) {
    // All levels invalid (zero tick, negative qty, NaN, zero qty) — book stays empty.
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
    double memAfterSnapshots = rssmegabytes();

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
              << " us/update)\n"
              << "[ MEMORY  ] RSS after snapshots : "
              << std::fixed << std::setprecision(1) << memAfterSnapshots << " MB\n"
#ifdef EXTENDED_DEPTH
              << "[ MEMORY  ] Depth mode          : extended (no cap)\n";
#else
              << "[ MEMORY  ] Depth mode          : normal (" << OrderBook::MAX_DEPTH << " levels/side)\n";
#endif
}

// Binance's REST snapshot API caps at 5000 levels per side — this is the true worst case
// for extended mode. Skipped in normal mode: levels would just be pruned to MAX_DEPTH anyway.
#ifdef EXTENDED_DEPTH
TEST(ScaleTest, BinanceMaxSnapshotDepth) {
    constexpr int NUM_SYMBOLS = 2000;
    constexpr int LEVELS      = 5000; // Binance /api/v3/depth?limit=5000 maximum

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
    double memAfterSnapshots = rssmegabytes();

    for (int i = 0; i < NUM_SYMBOLS; ++i) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "SYM%04d", i);
        const auto& book = symbolBook.getOrderBook(buf);
        EXPECT_GT(book.getBestBid(), 0ULL) << "bad best bid for " << buf;
        EXPECT_GT(book.getBestAsk(), 0ULL) << "bad best ask for " << buf;
        EXPECT_LT(book.getBestBid(), book.getBestAsk()) << "crossed book for " << buf;
    }

    auto snapshotMs = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    std::cout << "\n"
              << "[ TIMING  ] Snapshot  : " << NUM_SYMBOLS << " symbols x "
              << LEVELS << " levels = " << snapshotMs << " ms\n"
              << "[ MEMORY  ] RSS after snapshots : "
              << std::fixed << std::setprecision(1) << memAfterSnapshots << " MB\n"
              << "[ MEMORY  ] Depth mode          : extended (no cap, Binance max = 5000 levels/side)\n";
}
#endif

// Latency micro-tests
// Wall-clock averages over many iterations — shows the real ns cost of each
// individual operation. For tighter statistical CPU-cycle measurements, use
// ./benchmark.sh instead.

TEST(LatencyTest, InsertPriceLevel) {
    // Normal mode: book pre-filled to MAX_DEPTH; each insert is a tree insert +
    //              an immediate prune of the worst level — the hot steady-state path.
    // Extended mode: book pre-filled to 500 levels; grows by 1 per iteration.
    constexpr int ITERS = 500'000;

#ifndef EXTENDED_DEPTH
    constexpr int DEPTH = static_cast<int>(OrderBook::MAX_DEPTH);
#else
    constexpr int DEPTH = 500;
#endif

    OrderBook book;
    for (int i = 1; i <= DEPTH; ++i)
        book.updateBid(SCALE_BASE + static_cast<uint64_t>(i) * SCALE_STEP, static_cast<double>(i));

    // Each inserted price is strictly better than the previous best bid,
    // so in normal mode the worst level is always pruned on the same call.
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < ITERS; ++i)
        book.updateBid(SCALE_BASE + static_cast<uint64_t>(DEPTH + 1 + i) * SCALE_STEP, 1.0);
    auto t1 = std::chrono::steady_clock::now();

    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    std::cout << "[ LATENCY ] Insert bid level"
#ifndef EXTENDED_DEPTH
              << " (at cap=" << OrderBook::MAX_DEPTH << ", includes prune)"
#else
              << " (no cap, starting depth=" << DEPTH << ")"
#endif
              << ": " << (ns / ITERS) << " ns/op\n";
}

TEST(LatencyTest, RemovePriceLevel) {
    // Alternates remove (qty=0) + reinsert to keep the book populated.
    // Total time divided by 2×ITERS so each individual op cost is reported.
    constexpr int ITERS = 500'000;

    OrderBook book;
    for (int i = 1; i <= 20; ++i)
        book.updateBid(SCALE_BASE + static_cast<uint64_t>(i) * SCALE_STEP, static_cast<double>(i));

    const uint64_t target = SCALE_BASE + 10 * SCALE_STEP; // mid-book level

    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < ITERS; ++i) {
        book.updateBid(target, 0.0); // remove
        book.updateBid(target, 1.0); // reinsert so the next iteration has something to remove
    }
    auto t1 = std::chrono::steady_clock::now();

    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    std::cout << "[ LATENCY ] Remove bid level (remove+reinsert ÷ 2): "
              << (ns / (2 * ITERS)) << " ns/op\n";
}

TEST(LatencyTest, UpdateExistingLevel) {
    // Qty change on a level already in the book: map find + value overwrite,
    // no structural tree change and no prune.
    constexpr int ITERS = 1'000'000;

    OrderBook book;
    book.updateBid(SCALE_BASE, 10.0);

    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < ITERS; ++i)
        book.updateBid(SCALE_BASE, static_cast<double>((i & 0xFF) + 1)); // qty 1-256, never 0
    auto t1 = std::chrono::steady_clock::now();

    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    std::cout << "[ LATENCY ] Update existing bid qty: " << (ns / ITERS) << " ns/op\n";
}

TEST(LatencyTest, AddNewSymbol) {
    // Time applySnapshot() for a brand-new symbol.
    // Snapshot objects are pre-built so vector allocation is outside the timed region.
    constexpr int ITERS = 10'000;
#ifndef EXTENDED_DEPTH
    constexpr int LEVELS = static_cast<int>(OrderBook::MAX_DEPTH);
#else
    constexpr int LEVELS = 500;
#endif

    std::vector<Snapshot> snaps;
    snaps.reserve(ITERS);
    for (int i = 0; i < ITERS; ++i) {
        char buf[24];
        std::snprintf(buf, sizeof(buf), "LATSYM%05d", i);
        Snapshot s;
        s.strSymbol      = buf;
        s.llLastUpdateId = 0;
        for (int l = 1; l <= LEVELS; ++l) {
            s.bids.push_back({SCALE_BASE - static_cast<uint64_t>(l) * SCALE_STEP, static_cast<double>(l)});
            s.asks.push_back({SCALE_BASE + static_cast<uint64_t>(l) * SCALE_STEP, static_cast<double>(l)});
        }
        snaps.push_back(std::move(s));
    }

    SymbolBook sb;
    auto t0 = std::chrono::steady_clock::now();
    for (auto& snap : snaps)
        sb.applySnapshot(snap);
    auto t1 = std::chrono::steady_clock::now();

    auto us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    std::cout << "[ LATENCY ] Add new symbol (" << LEVELS << " levels/side): "
              << (us / ITERS) << " us/symbol  ("
              << (us * 1000 / (ITERS * LEVELS * 2)) << " ns/level)\n";
}

TEST(LatencyTest, BestPriceLookup) {
    constexpr int ITERS = 10'000'000;

    OrderBook book;
    book.updateBid(SCALE_BASE, 10.0);
    book.updateAsk(SCALE_BASE + SCALE_STEP, 10.0);

    uint64_t sum = 0;
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < ITERS; ++i)
        sum += book.getBestBid() + book.getBestAsk();
    auto t1 = std::chrono::steady_clock::now();
    gSink = sum;

    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    std::cout << "[ LATENCY ] Best bid+ask lookup: " << (ns / ITERS) << " ns/op\n";
}
