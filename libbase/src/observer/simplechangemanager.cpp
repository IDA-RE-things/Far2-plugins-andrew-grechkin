#include <libbase/observer_p.hpp>

#include <algorithm>
#include <vector>


namespace Base {

	typedef std::pair<Observable_p *, Observer_p *> mapping;

	bool operator == (const mapping & left, const mapping & right) {
		return left.first == right.first && left.second == right.second;
	}

	bool operator == (const mapping & left, const Observer_p * right) {
		return left.second == right;
	}

	bool operator < (const mapping & left, const Observable_p * right) {
		return left.first < right;
	}

	bool operator < (const Observable_p * left, const mapping & right) {
		return left < right.first;
	}


	struct SimpleChangeManager: public ChangeManager, private std::vector<mapping> {
		SimpleChangeManager()
		{
		}

		~SimpleChangeManager() override;

		void register_observer(Observable_p * subject, Observer_p * observer) override;

		void unregister_observer(Observable_p * subject, Observer_p * observer) override;

		void unregister_all(Observable_p * subject) override;

		void unregister_all(Observer_p * observer) override;

		void notify(const Observable_p * subject, const Event & event) const override;

	};


	SimpleChangeManager::~SimpleChangeManager() {
	}

	void SimpleChangeManager::register_observer(Observable_p * subject, Observer_p * observer) {
		emplace(std::upper_bound(begin(), end(), subject), subject, observer);
	}

	void SimpleChangeManager::unregister_observer(Observable_p * subject, Observer_p * observer) {
		auto range = std::equal_range(begin(), end(), subject);
		erase(remove(range.first, range.second, mapping(subject, observer)), range.second);
	}

	void SimpleChangeManager::unregister_all(Observable_p * subject) {
		auto range = std::equal_range(begin(), end(), subject);
		erase(range.first, range.second);
	}

	void SimpleChangeManager::unregister_all(Observer_p * observer) {
		erase(remove(begin(), end(), observer), end());
	}

	void SimpleChangeManager::notify(const Observable_p * subject, const Event & event) const {
		auto range = std::equal_range(begin(), end(), subject);
		for (auto it = range.first; it != range.second; ++it) {
			it->second->notify(event);
		}
	}


	ChangeManager * get_simple_change_manager() {
		static SimpleChangeManager ret;
		return &ret;
	}

}
