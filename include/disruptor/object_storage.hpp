#ifndef OBJECT_STORAGE_HPP_
#define OBJECT_STORAGE_HPP_

#include <cassert>
#include <cstring>
#include <mutex>
#include "macro.hpp"

INTERNAL_NAMESPACE_BEGIN

template<size_t OBJECT_COUNT, typename Deleter>
class ObjectStorage;

class Storable {
private:
	std::atomic<uintptr_t> rc;
	Storable **back_ptr;

	template<size_t OBJECT_COUNT, typename Deleter>
	friend class ObjectStorage;
};

template<size_t OBJECT_COUNT = 16, typename DELETER>
class ObjectStorage {
public:

	ObjectStorage(Storable *obj) {
		state.store(0, std::memory_order_relaxed);
		memset(objects, 0, sizeof(objects));
		objects[0] = obj;
		obj->rc.store(PERSISTENT, std::memory_order_relaxed);
		obj->back_ptr = &objects[0];
	}

	~ObjectStorage() {
		uintptr_t idx;
		Storable* obj;
		idx = state & OBJECT_MASK;
		obj = objects[idx];
		obj->rc -= (state & COUNT_MASK) / OBJECT_COUNT * TEMPORAL + PERSISTENT;
		dispose(obj);
	}

	Storable *aquire() {
		uintptr_t prev;
		uintptr_t idx;
		prev = (uintptr_t) state.fetch_add(COUNT_INC);
		idx = prev & OBJECT_MASK;
		return objects[idx];
	}

	void release(Storable *obj) {
		uintptr_t prev = obj->rc.fetch_add(TEMPORAL) + TEMPORAL;
		if (prev == 0) {
			dispose(obj);
		}
	}

	Storable* write_lock() noexcept {
		uintptr_t idx;
		write_mtx.lock();
		idx = state.load(std::memory_order_relaxed) & OBJECT_MASK;
		return objects[idx];
	}

	void write_unlock(Storable* obj) noexcept {
		uintptr_t prev;
		uintptr_t idx;

		uintptr_t old_cnt;
		uintptr_t old_idx;

		uintptr_t cnt_dif;
		uintptr_t cnt_res;

		Storable* old_obj;

		for (;;) {
			for (idx = 0; idx != OBJECT_COUNT; idx += 1) {
				if (objects[idx] == 0)
					break;
			}
			if (idx != OBJECT_COUNT)
				break;
			std::this_thread::yield();
		}

		objects[idx] = obj;
		obj->rc = PERSISTENT;
		obj->back_ptr = &objects[idx];

		prev = state.exchange(idx, std::memory_order_acq_rel);

		old_cnt = prev & COUNT_MASK;
		old_idx = prev & OBJECT_MASK;
		old_obj = objects[old_idx];

		assert(old_idx != idx);
		assert(old_obj->back_ptr == &objects[old_idx]);

		cnt_dif = (uintptr_t) - (intptr_t) (old_cnt / OBJECT_COUNT * TEMPORAL + PERSISTENT);
		cnt_res = (uintptr_t) old_obj->rc.fetch_add(cnt_dif, std::memory_order_acq_rel) + cnt_dif;

		write_mtx.unlock();

		if (cnt_res == 0) {
			dispose(old_obj);
		}

	}
private:

	void dispose(Storable *obj) {
		assert(obj->rc.load() == 0);
		assert(obj->back_ptr[0] == obj);
		obj->back_ptr[0] = 0;
		DELETER()(obj);
	}

	static const size_t OBJECT_MASK = OBJECT_COUNT - 1;
	static const size_t COUNT_MASK = (~OBJECT_MASK);
	static const uintptr_t COUNT_INC = OBJECT_COUNT;
	static const uintptr_t PERSISTENT = 1;
	static const uintptr_t TEMPORAL = 2;


	std::atomic<uintptr_t> state; // "outer" counter + index to
	Storable *objects[OBJECT_COUNT];
	std::mutex write_mtx;
};

INTERNAL_NAMESPACE_END

#endif /* OBJECT_STORAGE_HPP_ */
