#include <gtest/gtest.h>
#include <random>
#include <vector>
#include "segtree.hpp"

using std::vector;
using kirkon::segtree;
using kirkon::tourist_like_segtree_tag;

using ll = int64_t;
using ull = uint64_t;
struct sum_segtree_node {
  static constexpr ll NO_LAZY = std::numeric_limits<ll>::max();
  ll data{0};
  ll set_value{NO_LAZY};
  void apply(int l, int r, ll value) {
    data = value * 1LL * (r - l + 1);
    set_value = value;
  }

  friend sum_segtree_node unite(const sum_segtree_node& a, const sum_segtree_node& b) {
    sum_segtree_node res;
    res.data = a.data + b.data;
    return res;
  }

  // no lazy propagation
  void push(int l, int r, sum_segtree_node& left_son, sum_segtree_node& right_son) {
    if (set_value != NO_LAZY) {
      int y = (l + r) >> 1;
      left_son.apply(l, y, set_value);
      right_son.apply(y + 1, r, set_value);
      set_value = NO_LAZY;
    }
    return;
  }
};

TEST(SegtreeTest, TouristLikeSegtee) {
  vector<int> a = {1, 2, 3, 4, 5};
  segtree<sum_segtree_node, tourist_like_segtree_tag> segtree(a);
  for (int i = 0; i < a.size(); ++i) {
    EXPECT_EQ(segtree.get(i, i).data, a[i]);
  }
}

TEST(SegtreeTest, RangeQuery) {
    vector<int> arr = {1, 2, 3, 4, 5};
    segtree<sum_segtree_node, tourist_like_segtree_tag> seg(arr);

    EXPECT_EQ(seg.get(0, 4).data, 15);
    EXPECT_EQ(seg.get(1, 3).data, 9);
    EXPECT_EQ(seg.get(0, 0).data, 1);
    EXPECT_EQ(seg.get(2, 2).data, 3);
}

TEST(SegtreeTest, PointUpdate) {
    vector<int> arr = {3, 4, 5, 6, 7};
    segtree<sum_segtree_node, tourist_like_segtree_tag> seg(arr);

    seg.modify(2, 2, 10);
    EXPECT_EQ(seg.get(2, 2).data, 10);

    EXPECT_EQ(seg.get(0, 4).data, 30);
    EXPECT_EQ(seg.get(1, 3).data, 20);
}

TEST(SegtreeTest, RangeUpdate) {
    vector<int> arr = {1, 1, 1, 1, 1, 1, 1};
    segtree<sum_segtree_node, tourist_like_segtree_tag> seg(arr);

    seg.modify(1, 5, 5);
    EXPECT_EQ(seg.get(0, 6).data, 27);
    EXPECT_EQ(seg.get(1, 5).data, 25);
    EXPECT_EQ(seg.get(0, 0).data, 1);
    EXPECT_EQ(seg.get(6, 6).data, 1);
}

TEST(SegtreeTest, SequentialUpdates) {
    vector<int> arr = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    segtree<sum_segtree_node, tourist_like_segtree_tag> seg(arr);

    seg.modify(3, 5, 100);
    EXPECT_EQ(seg.get(3, 5).data, 300);
    EXPECT_EQ(seg.get(0, 8).data, 1 + 2 + 3 + 100 + 100 + 100 + 7 + 8 + 9);

    seg.modify(4, 4, 50);
    EXPECT_EQ(seg.get(3, 5).data, 100 + 50 + 100);

    seg.modify(0, 2, 10);
    EXPECT_EQ(seg.get(0, 8).data, 10 + 10 + 10 + 100 + 50 + 100 + 7 + 8 + 9);
}

std::mt19937_64 rng{123};
ull myRand(ull B) {
  return static_cast<ull>(rng()) % B;
}

TEST(SegtreeTest, StressTest) {
    // Stress test for sum segtree using a naive simulation.
    // We consider n <= 1000 and perform numQueries <= 1000 operations.
    const int n = 10000;
    const int numQueries = 10000;
    // Start with an array of zeros.
    vector<int> initial(n, 0);

    // Build segtree and construct naive simulation.
    segtree<sum_segtree_node, tourist_like_segtree_tag> seg(initial);
    vector<int> naive = initial;

    // Using myRand (the global rng in myRand is already seeded)
    
    for (int q = 0; q < numQueries; q++) {
        // Randomly decide: 0 for update, 1 for query.
        int op = myRand(2);
        int l = myRand(n);
        int r = myRand(n);
        if (l > r) {
            std::swap(l, r);
        }
        // Update operation: assign all values in range [l, r] to a new random value.
        if (op == 0) {
            // Pick a random value in the range [-1000, 1000].
            int newValue = static_cast<int>(myRand(2001)) - 1000;
            seg.modify(l, r, newValue);
            for (int i = l; i <= r; i++) {
                naive[i] = newValue;
            }
        } else {
            // Query operation: compare sum over [l, r] with the naive sum.
            auto node = seg.get(l, r);
            long long expectedSum = 0;
            for (int i = l; i <= r; i++) {
                expectedSum += naive[i];
            }
            EXPECT_EQ(node.data, expectedSum)
                << "Mismatch at iteration " << q << " for query range [" << l << ", " << r << "]";
        }
    }
}

TEST(SegtreeTest, PerformanceTest) {
    // Performance test: do many operations on a large segtree.
    // This test does not check for correctness, it only verifies that
    // a sequence of operations (updates and queries) finishes within 1 second.
    const int n = 1000000;        
    const int numQueries = 1000000;

    vector<int> initial(n, 0);
    segtree<sum_segtree_node, tourist_like_segtree_tag> seg(initial);

    auto start = std::chrono::high_resolution_clock::now();
    for (int q = 0; q < numQueries; q++) {
        int op = myRand(2);
        int l = myRand(n);
        int r = myRand(n);
        if (l > r) {
            std::swap(l, r);
        }
        if (op == 0) {
            int newValue = static_cast<int>(myRand(2001)) - 1000;
            seg.modify(l, r, newValue);
        } else {
            seg.get(l, r);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();
    std::cerr << "Performance test took " << elapsed << " seconds" << std::endl;
    // EXPECT_LT(elapsed, 1.0) << "Performance test took too long: " << elapsed << " seconds";
}

