#include <benchmark/benchmark.h>
#include "orderbook.h"
#include "symbolBook.h"

// Prices are stored as integer ticks (price × 1e8) so that map keys are exact
// integers with no floating-point comparison ambiguity. The two constants below
// define the synthetic price space used across all benchmarks: a base of 1.0
// and a step of 0.0001 between adjacent levels, mirroring a realistic tight spread.
static constexpr uint64_t BENCH_BASE = 100000000ULL;  // 1.0 × 1e8 — midpoint price
static constexpr uint64_t BENCH_STEP = 10000ULL;       // 0.0001 × 1e8 — per-level spacing

static SymbolBook makeSymbolBook(int numSymbols, int levelsPerSide) {
    SymbolBook sb;
    for (int i = 0; i < numSymbols; ++i) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "SYM%04d", i);
        Snapshot snap;
        snap.strSymbol      = buf;
        snap.llLastUpdateId = 0;
        for (int l = 1; l <= levelsPerSide; ++l) {
            snap.bids.push_back({BENCH_BASE - l * BENCH_STEP, static_cast<double>(l)});
            snap.asks.push_back({BENCH_BASE + l * BENCH_STEP, static_cast<double>(l)});
        }
        sb.applySnapshot(snap);
    }
    return sb;
}

// Benchmarks for SymbolBook operations: applying snapshots, handling depth updates,
// and retrieving best bid/ask prices.

static void BM_ApplySnapshot(benchmark::State& state) {
    const int levels = state.range(0);
    Snapshot snap;
    snap.strSymbol      = "BENCH";
    snap.llLastUpdateId = 0;
    for (int l = 1; l <= levels; ++l) {
        snap.bids.push_back({BENCH_BASE - l * BENCH_STEP, static_cast<double>(l)});
        snap.asks.push_back({BENCH_BASE + l * BENCH_STEP, static_cast<double>(l)});
    }

    for (auto _ : state) {
        SymbolBook sb;
        sb.applySnapshot(snap);
        benchmark::DoNotOptimize(sb);
    }
    state.SetItemsProcessed(state.iterations() * levels * 2);
    state.SetLabel(std::to_string(levels) + " levels/side");
}
BENCHMARK(BM_ApplySnapshot)->Arg(100)->Arg(500)->Arg(1000)->Arg(5000);

static void BM_HandleDepthUpdate(benchmark::State& state) {
    const int numSymbols = state.range(0);
    auto sb = makeSymbolBook(numSymbols, 100);

    int seq = 1;
    for (auto _ : state) {
        for (int i = 0; i < numSymbols; ++i) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "SYM%04d", i);
            DepthUpdate d;
            d.strSymbol       = buf;
            d.llFirstUpdateId = seq;
            d.llFinalUpdateId = seq;
            d.bids.push_back({BENCH_BASE - (seq % 100) * BENCH_STEP, static_cast<double>(seq)});
            d.asks.push_back({BENCH_BASE + (seq % 100) * BENCH_STEP, static_cast<double>(seq)});
            sb.handleDepthUpdate(d);
        }
        ++seq;
    }
    state.SetItemsProcessed(state.iterations() * numSymbols);
    state.SetLabel(std::to_string(numSymbols) + " symbols");
}
BENCHMARK(BM_HandleDepthUpdate)->Arg(1)->Arg(100)->Arg(1000)->Arg(2000);

static void BM_GetBestBidAsk(benchmark::State& state) {
    const int numSymbols = state.range(0);
    auto sb = makeSymbolBook(numSymbols, 100);

    int i = 0;
    for (auto _ : state) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "SYM%04d", i % numSymbols);
        const auto& book = sb.getOrderBook(buf);
        benchmark::DoNotOptimize(book.getBestBid());
        benchmark::DoNotOptimize(book.getBestAsk());
        ++i;
    }
    state.SetItemsProcessed(state.iterations());
    state.SetLabel(std::to_string(numSymbols) + " symbols");
}
BENCHMARK(BM_GetBestBidAsk)->Arg(1)->Arg(100)->Arg(1000)->Arg(2000);

BENCHMARK_MAIN();
