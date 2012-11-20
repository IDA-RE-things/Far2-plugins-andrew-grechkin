#include <libbase/observer_p.hpp>
#include <libbase/message.hpp>

#include <algorithm>
#include <vector>

#include "MessageManager.hpp"

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

	struct SimpleMessageManager: public MessageManager, private std::vector<mapping> {
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
		emplace(std::upper_bound(begin(), end(), subject), subject, observer);
	}

	void SimpleMessageManager::unregister_observer(Observable_p * subject, Observer_p * observer)
	{
		auto range = std::equal_range(begin(), end(), subject);
		erase(remove(range.first, range.second, observer), range.second);
	}

	void SimpleMessageManager::unregister_all(Observable_p * subject)
	{
		auto range = std::equal_range(begin(), end(), subject);
		erase(range.first, range.second);
	}

	void SimpleMessageManager::unregister_all(Observer_p * observer)
	{
		erase(remove(begin(), end(), observer), end());
	}

	void SimpleMessageManager::notify(const Observable_p * subject, Message const& event) const
	{
		auto range = std::equal_range(begin(), end(), subject);
		std::for_each(range.first, range.second, [event](mapping const& tmp) {
			tmp.second->notify(event);
		});
	}

	MessageManager * get_simple_message_manager()
	{
		static SimpleMessageManager ret;
		return &ret;
	}

}
