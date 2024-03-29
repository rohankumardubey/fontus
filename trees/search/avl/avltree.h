#ifndef AVLTREE_H
#define AVLTREE_H

#include <bits/stdc++.h>

namespace fontus {

template <typename T>
struct AVLTreeNode {
	T value;
	int height;
	std::unique_ptr<AVLTreeNode<T>> left;
	std::unique_ptr<AVLTreeNode<T>> right;

	AVLTreeNode(T val) : value(std::move(val)), height(1), left(), right() {}
	AVLTreeNode(AVLTreeNode&& that) = default;
	AVLTreeNode& operator=(AVLTreeNode&& that) = default;
	AVLTreeNode(const AVLTreeNode& that) = delete;
	AVLTreeNode& operator=(const AVLTreeNode& that) = delete;

	bool insert(T val) {
		if (val < value) {
			if (left) {
				bool result = left->insert(std::move(val));
				if (result) {
					height = compute_height();
					left = rebalance(std::move(left));
				}
				return result;
			}
			left = std::make_unique<AVLTreeNode<T>>(std::move(val));
			height = compute_height();
			return true;
		} else if (val > value) {
			if (right) {
				bool result = right->insert(std::move(val));
				if (result) {
					height = compute_height();
					right = rebalance(std::move(right));
				}
				return result;
			}
			right = std::make_unique<AVLTreeNode<T>>(std::move(val));
			height = compute_height();
			return true;
		}
		return false;
	}

	std::optional<T> find(const T& val) {
		if (val < value && left) {
			return left->find(val);
		}
		if (val > value && right) {
			return right->find(val);
		}
		if (val == value) {
			return value;
		}

		return std::nullopt;
	}

	std::pair<bool, std::unique_ptr<AVLTreeNode<T>>> remove(const T& val) {
		if (value == val) {
			if (!left) {
				// The caller would release the unique_ptr on which this was
				// called, and replace it with the `right` unique_ptr which
				// may be null.
				return std::make_pair(true, std::move(right));
			} else if (!right) {
				// The caller would release the unique_ptr on which this was
				// called, and replace it with the `right` unique_ptr which
				// cannot be null.
				return std::make_pair(true, std::move(left));
			} else {
				if (left->height <= right->height) {
					std::tuple<bool, T, std::unique_ptr<AVLTreeNode<T>>> result =
						right->remove_smallest();
					if (std::get<0>(result)) {
						right = std::move(std::get<2>(result));
						height = compute_height();
					}
					value = std::move(std::get<1>(result));
				} else {
					std::tuple<bool, T, std::unique_ptr<AVLTreeNode<T>>> result =
						left->remove_largest();
					if (std::get<0>(result)) {
						left = std::move(std::get<2>(result));
						height = compute_height();
					}
					value = std::move(std::get<1>(result));
				}
				return std::make_pair(false, std::unique_ptr<AVLTreeNode<T>>());
			}
		} else if (val < value && left) {
			std::pair<bool, std::unique_ptr<AVLTreeNode<T>>> result =
				left->remove(val);
			if (std::get<0>(result)) {
				left = std::move(std::get<1>(result));
				height = compute_height();
			}
			left = rebalance(std::move(left));
			return std::make_pair(false, std::unique_ptr<AVLTreeNode<T>>());
		} else if (val > value && right) {
			std::pair<bool, std::unique_ptr<AVLTreeNode<T>>> result =
				right->remove(val);
			if (std::get<0>(result)) {
				right = std::move(std::get<1>(result));
				height = compute_height();
			}
			right = rebalance(std::move(right));
			return std::make_pair(false, std::unique_ptr<AVLTreeNode<T>>());
		}
		return std::make_pair(false, std::unique_ptr<AVLTreeNode<T>>());
	}

	static std::unique_ptr<AVLTreeNode<T>> rebalance(
			std::unique_ptr<AVLTreeNode<T>> base) {
		int balance = base->compute_balance();
		if (balance < -1) {
			if (base->left->compute_balance() > 0) {
				base->left = rotate_left(std::move(base->left));
			}
			if (base->left->compute_balance() < 0) {
				return rotate_right(std::move(base));
			}
		} else if (balance > 1) {
			if (base->right->compute_balance() < 0) {
				base->right = rotate_right(std::move(base->right));
			}
			if (base->right->compute_balance() > 0) {
				return rotate_left(std::move(base));
			}
		}
		return std::move(base);
	}

private:
	std::tuple<bool, T, std::unique_ptr<AVLTreeNode<T>>> remove_smallest() {
		if (!left) {
			// There is no smaller node than this one so return the right
			// sub-tree (which could be null). The caller should replace
			// its current child with this one.
			return std::make_tuple(true, std::move(value), std::move(right));
		}
		bool replace = false;
		std::tuple<bool, T, std::unique_ptr<AVLTreeNode<T>>> result =
			left->remove_smallest();
		if (std::get<0>(result)) {
			left = std::move(std::get<2>(result));
			height = compute_height();
		}

		// pass a fake node in the second arg as it would not be used
		// since the first arg is false.
		return std::make_tuple(false, std::move(std::get<1>(result)),
				std::unique_ptr<AVLTreeNode<T>>());
	}

	std::tuple<bool, T, std::unique_ptr<AVLTreeNode<T>>> remove_largest() {
		if (!right) {
			// There is no larger node than this one so return the left
			// sub-tree (which could be null). The caller should replace
			// its current child with this one.
			return std::make_tuple(true, std::move(value), std::move(left));
		}
		bool replace = false;
		std::tuple<bool, T, std::unique_ptr<AVLTreeNode<T>>> result =
			right->remove_largest();
		if (std::get<0>(result)) {
			right = std::move(std::get<2>(result));
			height = compute_height();
		}

		// pass a fake node in the second arg as it would not be used
		// since the first arg is false.
		return std::make_tuple(false, std::move(std::get<1>(result)),
				std::unique_ptr<AVLTreeNode<T>>());
	}

	int compute_height() const {
		return 1 + std::max(left ? left->height : 0,
				right ? right->height : 0);
	}

	int compute_balance() const {
		return (right ? right->height : 0) - (left ? left->height : 0);
	}

	static std::unique_ptr<AVLTreeNode<T>> rotate_left(std::unique_ptr<AVLTreeNode<T>> base) {
		std::unique_ptr<AVLTreeNode<T>> new_root = std::move(base->right);
		base->right = std::move(new_root->left);
		base->height = base->compute_height();
		new_root->left = std::move(base);
		new_root->height = new_root->compute_height();
		return new_root;
	}

	static std::unique_ptr<AVLTreeNode<T>> rotate_right(std::unique_ptr<AVLTreeNode<T>> base) {
		std::unique_ptr<AVLTreeNode<T>> new_root = std::move(base->left);
		base->left = std::move(new_root->right);
		base->height = base->compute_height();
		new_root->right = std::move(base);
		new_root->height = new_root->compute_height();
		return new_root;
	}


};

template <typename T>
class AVLTree {
public:
	AVLTree() : root() {}
	~AVLTree() {}

	AVLTree(const AVLTree<T>&) = delete;
	AVLTree<T>& operator=(const AVLTree<T>&) = delete;

	AVLTree<T>(AVLTree<T>&& that) noexcept : root(nullptr) {
		std::swap(root, that.root);
	}

	AVLTree<T>& operator=(AVLTree<T>&& that) noexcept {
		AVLTree<T> tmp(std::move(that));
		std::swap(root, tmp.root);
		return *this;
	}

	std::optional<T> find(const T& value) {
		return root->find(value);
	}

	bool insert(T&& value) {
		if (!root) {
			root = std::make_unique<AVLTreeNode<T>>(std::move(value));
			++count;
			return true;
		}
		if (root->insert(std::move(value))) {
			++count;
			root = AVLTreeNode<T>::rebalance(std::move(root));
			return true;
		}
		return false;
	}

	std::optional<T> remove(const T& value) {
		if (!root) {
			return std::nullopt;
		}
		std::pair<bool, std::unique_ptr<AVLTreeNode<T>>> result =
			root->remove(value);
		if (result.first) {
			root = std::move(result.second);
		}
		root = AVLTreeNode<T>::rebalance(std::move(root));
		return std::nullopt;
	}

	void traverse() {
		std::queue<AVLTreeNode<T>*> queue;
		if (root) {
			queue.push(root.get());
			queue.push(nullptr);
		}

		while (!queue.empty()) {
			if (!queue.front()) {
				std::cout << '\n';
				if (queue.size() > 1) {
					// Add an end of level marker
					// only if this is not the last entry.
					queue.push(nullptr);
				}
				queue.pop();
				continue;
			}
			if (queue.front()->left) {
				queue.push(queue.front()->left.get());
			}
			if (queue.front()->right) {
				queue.push(queue.front()->right.get());
			}
			std::cout << queue.front()->value << '[' << queue.front()->height << "] ";
			queue.pop();
		}
	}

private:
	std::unique_ptr<AVLTreeNode<T>> root;
	int count;
};

} // namespace fontus

#endif /* AVLTREE_H */
