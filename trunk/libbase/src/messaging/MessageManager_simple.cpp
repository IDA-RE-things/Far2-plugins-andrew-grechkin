#include "MessageManager.hpp"
#include <libbase/messaging.hpp>
#include <libbase/lock.hpp>

#include <algorithm>
#include <vector>

namespace Base {

	typedef std::pair<Observable_p *, Observer_p *> mapping;

	inline bool operator ==(const mapping & left, const Observer_p * right)
	{
		return left.second == right;
	}

	inline bool operator <(const mapping & left, const Observable_p * right)
	{
		return left.first < right;
	}

	inline bool operator <(const Observable_p * left, const mapping & right)
	{
		return left < right.first;
	}

	struct SimpleMessageManager: public MessageManager, private Base::Lock::CriticalSection, private std::vector<mapping> {
		SimpleMessageManager()
		{
		}

		~SimpleMessageManager();

		void register_observer(Observable_p * subject, Observer_p * observer) override;

		void unregister_observer(Observable_p * subject, Observer_p * observer) override;

		void unregister_all(Observable_p * subject) override;

		void unregister_all(Observer_p * observer) override;

		void notify(const Observable_p * subject, Message const& event) const override;
	};

	SimpleMessageManager::~SimpleMessageManager()
	{
	}

	void SimpleMessageManager::register_observer(Observable_p * subject, Observer_p * observer)
	{
		lock();
		emplace(std::upper_bound(begin(), end(), subject), subject, observer);
		release();
	}

	void SimpleMessageManager::unregister_observer(Observable_p * subject, Observer_p * observer)
	{
		lock();
		auto range = std::equal_range(begin(), end(), subject);
		erase(remove(range.first, range.second, observer), range.second);
		release();
	}

	void SimpleMessageManager::unregister_all(Observable_p * subject)
	{
		lock();
		auto range = std::equal_range(begin(), end(), subject);
		erase(range.first, range.second);
		release();
	}

	void SimpleMessageManager::unregister_all(Observer_p * observer)
	{
		lock();
		erase(remove(begin(), end(), observer), end());
		release();
	}

	void SimpleMessageManager::notify(const Observable_p * subject, const Message & event) const
	{
		lock();
		auto range = std::equal_range(begin(), end(), subject);
		std::for_each(range.first, range.second, [event](const mapping & pair) {
			pair.second->notify(event);
		});
		release();
	}

	MessageManager * get_simple_message_manager()
	{
		static SimpleMessageManager ret;
		return &ret;
	}

}
