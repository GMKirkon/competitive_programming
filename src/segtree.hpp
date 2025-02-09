#pragma once

#include <cassert>
#include <vector>
#include <concepts>


namespace kirkon {
using std::vector;

/**
 * @brief Interface for segtree node.
 */
struct segtree_node_interface {
  /**
   * @brief Apply the node to the range [l, r). 
   * @param l The left bound of the range.
   * @param r The right bound of the range.
   * @param ... The arguments to apply.
   */
  virtual void apply(int l, int r, ...) = 0;
  
  /**
   * @brief Unite two nodes.
   * @param a The first node.
   * @param b The second node.
   * @return The united node.
   */
  virtual segtree_node_interface& unite(const segtree_node_interface& a, const segtree_node_interface& b) = 0;

  /**
   * @brief Push the node to the children.
   * @param x The index of the node.
   * @param l The left bound of the range.
   * @param r The right bound of the range.
   * @param left_son The left son.
   * @param right_son The right son.
   */
  virtual void push(int l, int r, const segtree_node_interface& left_son, const segtree_node_interface& right_son) = 0;
};

/**
 * @brief Concept for segtree node.
 *
 * Concept instead of interface to avoid virtual function calls.
 * @tparam T The type of the node.
 * @tparam unite The function to unite two nodes.
 * @tparam push The function to push the node to the children.
 * @tparam apply The function to apply the node to the range [l, r).
 */
template<typename T>
concept SegtreeNodeWithPushConcept = requires(T t, T left, T right, const T const_left, const T const_right, int l, int r) {
    { t.data };
    { ([](auto& t, int l, int r) -> void {
          if constexpr (requires { t.apply(l, r, 0); }) {
              t.apply(l, r, 0);
          } else if constexpr (requires { t.apply(l, r, 0LL); }) {
              t.apply(l, r, 0LL);
          } else {
              t.apply(l, r);
          }
      })(t, l, r) } -> std::same_as<void>; // TODO find counterexample where smth should be added to concept
    { t.push(l, r, left, right) } -> std::same_as<void>;
    { unite(const_left, const_right) } -> std::convertible_to<T>;
};

/**
 * @brief Concept for segtree node WITHOUT PUSH.
 *
 * Concept instead of interface to avoid virtual function calls.
 * @tparam T The type of the node.
 * @tparam unite The function to unite two nodes.
 * @tparam apply The function to apply the node to the range [l, r).
 */
template<typename T>
concept SegtreeNodeWithoutPushConcept = requires(T t, const T const_left, const T const_right, int l, int r) {
    { t.data };
    { ([](auto& t, int l, int r) -> void {
          if constexpr (requires { t.apply(l, r, 0); }) {
              t.apply(l, r, 0);
          } else if constexpr (requires { t.apply(l, r, 0LL); }) {
              t.apply(l, r, 0LL);
          } else {
              t.apply(l, r);
          }
      })(t, l, r) } -> std::same_as<void>; // TODO find counterexample where smth should be added to concept
    { unite(const_left, const_right) } -> std::convertible_to<T>;
};


struct tourist_like_segtree_tag {};
struct bottom_up_segtree_tag {};
struct top_down_segtree_tag {};

template<class T, class Tag>
struct segtree;

template<class T>
struct segtree_traits {
  using node = T;
};

/**
 * @brief Abstract class for tourist-like segtree. Used to avoid copy-paste for Push and NoPush implementations.
 * @tparam Derived The derived class.
 */
template<class Derived, class NodeType>
struct abstract_tourist_like_segtree {
  using node = NodeType;
  void push(int x, int l, int r) {
    static_cast<Derived*>(this)->push(x, l, r);
  }

  void pull(int x, int z) {
    tree[x] = unite(tree[x + 1], tree[z]);
  }

  int n;
  vector<node> tree;
  void build(int x, int l, int r) {
    if (l == r) {
      return;
    }
    int y = (l + r) >> 1;
    int z = x + ((y - l + 1) << 1);
    build(x + 1, l, y);
    build(z, y + 1, r);
    pull(x, z);
  }
  
  template <typename M>
  void build(int x, int l, int r, const vector<M> &v) {
    if (l == r) {
      tree[x].apply(l, r, v[l]);
      return;
    }
    int y = (l + r) >> 1;
    int z = x + ((y - l + 1) << 1);
    build(x + 1, l, y, v);
    build(z, y + 1, r, v);
    pull(x, z);
  }
  node get(int x, int l, int r, int ll, int rr) {
    if (ll <= l && r <= rr) {
      return tree[x];
    }
    int y = (l + r) >> 1;
    int z = x + ((y - l + 1) << 1);
    push(x, l, r);
    node res{};
    if (rr <= y) {
      res = get(x + 1, l, y, ll, rr);
    } else {
      if (ll > y) {
        res = get(z, y + 1, r, ll, rr);
      } else {
        res = unite(get(x + 1, l, y, ll, rr), get(z, y + 1, r, ll, rr));
      }
    }
    pull(x, z);
    return res;
  }
  template <typename... M>
  void modify(int x, int l, int r, int ll, int rr, const M&... v) {
    if (ll <= l && r <= rr) {
      tree[x].apply(l, r, v...);
      return;
    }
    int y = (l + r) >> 1;
    int z = x + ((y - l + 1) << 1);
    push(x, l, r);
    if (ll <= y) {
      modify(x + 1, l, y, ll, rr, v...);
    }
    if (rr > y) {
      modify(z, y + 1, r, ll, rr, v...);
    }
    pull(x, z);
  }
  abstract_tourist_like_segtree(int _n) : n(_n) {
    assert(n > 0);
    tree.resize(2 * n - 1);
    build(0, 0, n - 1);
  }
  template <typename M>
  abstract_tourist_like_segtree(const vector<M> &v) {
    n = v.size();
    assert(n > 0);
    tree.resize(2 * n - 1);
    build(0, 0, n - 1, v);
  }
  node get(int ll, int rr) {
    assert(0 <= ll && ll <= rr && rr <= n - 1);
    return get(0, 0, n - 1, ll, rr);
  }
  node get(int p) {
    assert(0 <= p && p <= n - 1);
    return get(0, 0, n - 1, p, p);
  }
  template <typename... M>
  void modify(int ll, int rr, const M&... v) {
    assert(0 <= ll && ll <= rr && rr <= n - 1);
    modify(0, 0, n - 1, ll, rr, v...);
  }
};

template<class T>
requires SegtreeNodeWithPushConcept<T>
struct segtree<T, tourist_like_segtree_tag> : abstract_tourist_like_segtree<segtree<T, tourist_like_segtree_tag>, T> {
public:
  using Base = abstract_tourist_like_segtree<segtree<T, tourist_like_segtree_tag>, T>;
  using Base::get;
  using Base::modify;
  using node = T;
  
  void push(int x, int l, int r) {
    int y = (l + r) >> 1;
    int z = x + ((y - l + 1) << 1);
    this->tree[x].push(l, r, this->tree[x + 1], this->tree[z]);
  }

  segtree(int _n) : Base(_n) {}
  template<class M>
  segtree(const vector<M>& v) : Base(v) {}
};

template<class T>
requires SegtreeNodeWithoutPushConcept<T> && (!SegtreeNodeWithPushConcept<T>)
struct segtree<T, tourist_like_segtree_tag> : abstract_tourist_like_segtree<segtree<T, tourist_like_segtree_tag>, T> {
public:
  using Base = abstract_tourist_like_segtree<segtree<T, tourist_like_segtree_tag>, T>;
  using Base::get;
  using Base::modify;
  using node = T;
  
  void push([[maybe_unused]] int x, [[maybe_unused]] int l, [[maybe_unused]] int r) { }

  segtree(int _n) : Base(_n) {}
  template<class M>
  segtree(const vector<M>& v) : Base(v) {}
};

template<class T>
requires SegtreeNodeWithoutPushConcept<T> && (!SegtreeNodeWithPushConcept<T>)
struct segtree<T, bottom_up_segtree_tag> {
  using node = T;
  int n;
  vector<T> tree;
  segtree(int n) : n(n) {
    tree.resize(2 * n - 1);
  }

  template<class M>
  void build(const vector<M>& v) {
    for (int i = n; i < 2 * n - 1; ++i) {
      tree[i].apply(i, i, v[i - n]);  
    }
    for (int i = n - 1; i > 0; --i) {
      tree[i] = unite(tree[i << 1], tree[i << 1 | 1]);
    }
  }

  T get(int l, int r) {
    T left_res{};
    T right_res{};
    for (l += n, r += n; l < r; l >>= 1, r >>= 1) {
      if (l & 1) left_res = unite(left_res, tree[l++]);
      if (r & 1) right_res = unite(tree[--r], right_res);
    }
    return unite(left_res, right_res);
  }

  T get(int p) {
    return tree[p + n];
  }

  void modify(int p, const T& v) {
    for (tree[p += n] = v; p >>= 1;) {
      tree[p] = unite(tree[p << 1], tree[p << 1 | 1]);
    }
  }
};
}
