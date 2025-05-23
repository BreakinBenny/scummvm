/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef COMMON_LIST_INTERN_H
#define COMMON_LIST_INTERN_H

#include "common/util.h"

namespace Common {

template<typename T> class List;


namespace ListInternal {
	struct NodeBase {
		NodeBase *_prev = nullptr;
		NodeBase *_next = nullptr;

		constexpr NodeBase() = default;
		constexpr NodeBase(NodeBase *prev, NodeBase *next) : _prev(prev), _next(next) {}
	};

	template<typename T>
	struct Node : public NodeBase {
		T _data;

		Node(const T &x) : _data(x) {}
		Node(T &&x) : _data(Common::move(x)) {}
		template<class... TArgs>
		Node(TArgs&&... args) : _data(Common::forward<TArgs>(args)...) {}
	};

	template<typename T> struct ConstIterator;

	template<typename T>
	struct Iterator {
		typedef Iterator<T>	Self;
		typedef Node<T> *	NodePtr;
		typedef T &			ValueRef;
		typedef T *			ValuePtr;
		typedef T			ValueType;

		NodeBase *_node;

		Iterator() : _node(nullptr) {}
		explicit Iterator(NodeBase *node) : _node(node) {}

		// Prefix inc
		Self &operator++() {
			if (_node)
				_node = _node->_next;
			return *this;
		}
		// Postfix inc
		Self operator++(int) {
			Self tmp(_node);
			++(*this);
			return tmp;
		}
		// Prefix dec
		Self &operator--() {
			if (_node)
				_node = _node->_prev;
			return *this;
		}
		// Postfix dec
		Self operator--(int) {
			Self tmp(_node);
			--(*this);
			return tmp;
		}
		ValueRef operator*() const {
			assert(_node);
			return static_cast<NodePtr>(_node)->_data;
		}
		ValuePtr operator->() const {
			return &(operator*());
		}

		bool operator==(const Self &x) const {
			return _node == x._node;
		}

		bool operator!=(const Self &x) const {
			return _node != x._node;
		}
	};

	template<typename T>
	struct ConstIterator {
		typedef ConstIterator<T>	Self;
		typedef const Node<T> *	NodePtr;
		typedef const T &		ValueRef;
		typedef const T *		ValuePtr;

		const NodeBase *_node;

		ConstIterator() : _node(nullptr) {}
		explicit ConstIterator(const NodeBase *node) : _node(node) {}
		ConstIterator(const Iterator<T> &x) : _node(x._node) {}

		// Prefix inc
		Self &operator++() {
			if (_node)
				_node = _node->_next;
			return *this;
		}
		// Postfix inc
		Self operator++(int) {
			Self tmp(_node);
			++(*this);
			return tmp;
		}
		// Prefix dec
		Self &operator--() {
			if (_node)
				_node = _node->_prev;
			return *this;
		}
		// Postfix dec
		Self operator--(int) {
			Self tmp(_node);
			--(*this);
			return tmp;
		}
		ValueRef operator*() const {
			assert(_node);
			return static_cast<NodePtr>(_node)->_data;
		}
		ValuePtr operator->() const {
			return &(operator*());
		}

		bool operator==(const Self &x) const {
			return _node == x._node;
		}

		bool operator!=(const Self &x) const {
			return _node != x._node;
		}
	};


	template<typename T>
	bool operator==(const Iterator<T>& a, const ConstIterator<T>& b) {
		return a._node == b._node;
	}

	template<typename T>
	bool operator!=(const Iterator<T>& a, const ConstIterator<T>& b) {
		return a._node != b._node;
	}
}


} // End of namespace Common

#endif
