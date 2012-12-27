#include <libbase/message.hpp>
#include <libbase/lock.hpp>

#include <vector>

namespace {

	struct DeliveryMapping {
		DeliveryMapping(Base::Message::type_t type_mask, Base::Message::code_t code_mask, Base::Queue * queue, Base::Delivery::filter_t filter):
			m_type_mask(type_mask),
			m_code_mask(code_mask),
			m_queue(queue),
			m_filter(filter)
		{
		}

		bool operator () (Base::Message const& message) const
		{
			if (check_mask(message) && check_filter(message))
			{
				m_queue->put_message(message);
				return true;
			}
			return false;
		}

		bool operator == (Base::Queue const* queue) const
		{
			return m_queue == queue;
		}

	private:
		bool check_mask(Base::Message const& message) const
		{
			return (m_type_mask & message.get_type()) && (m_code_mask & message.get_code());
		}

		bool check_filter(Base::Message const& message) const
		{
			return !m_filter || m_filter(message);
		}

		Base::Message::type_t m_type_mask;
		Base::Message::code_t m_code_mask;
		Base::Queue * m_queue;
		Base::Delivery::filter_t m_filter;
	};

	typedef std::pair<Base::Delivery::SubscribtionId, DeliveryMapping> dm_t;

	struct Delivery_impl: private Base::Lock::CriticalSection, private std::vector<dm_t> {

		Delivery_impl():
			m_id_generator(0)
		{
		}

		static Delivery_impl & inst();

		Base::Delivery::SubscribtionId Subscribe(Base::Message::type_t type_mask, Base::Message::code_t code_mask, Base::Queue * queue, Base::Delivery::filter_t filter);

		void Unsubscribe(Base::Delivery::SubscribtionId id);

		void Unsubscribe(Base::Queue const* queue);

		void Propagate(Base::Message const& message) const;

	private:
		Base::Delivery::SubscribtionId GetNextId()
		{
			return ++m_id_generator;
		}

		Base::Delivery::SubscribtionId m_id_generator;
	};

	Delivery_impl & Delivery_impl::inst()
	{
		static Delivery_impl instance;
		return instance;
	}

	Base::Delivery::SubscribtionId Delivery_impl::Subscribe(Base::Message::type_t type_mask, Base::Message::code_t code_mask, Base::Queue * queue, Base::Delivery::filter_t filter)
	{
		lock();
		auto id = GetNextId();
		emplace_back(id, DeliveryMapping(type_mask, code_mask, queue, filter));
		release();
		return id;
	}

	void Delivery_impl::Unsubscribe(Base::Delivery::SubscribtionId id)
	{
		lock();
		auto it = std::find_if(begin(), end(), [&](dm_t const& item) {return item.first == id;});
		if (it != end())
			erase(it);
		release();
	}

	void Delivery_impl::Unsubscribe(Base::Queue const* queue)
	{
		lock();
		for (auto it = rbegin(); it != rend(); ++it) {
			if (it->second == queue)
				erase(it.base());
		}
		release();
	}

	void Delivery_impl::Propagate(Base::Message const& message) const
	{
		lock();
//		std::for_each(begin(), end(), [&](dm_t const& item) {
//			item.second(message);
//		});
		for (dm_t const& item : *this) {
			item.second(message);
		}
		release();
	}

}

namespace Base {
	namespace Delivery {

		SubscribtionId Subscribe(Message::type_t type_mask, Message::code_t code_mask, Queue * queue, filter_t filter)
		{
			return Delivery_impl::inst().Subscribe(type_mask, code_mask, queue, filter);
		}

		void Unsubscribe(SubscribtionId id)
		{
			Delivery_impl::inst().Unsubscribe(id);
		}

		void Unsubscribe(Queue const* queue)
		{
			Delivery_impl::inst().Unsubscribe(queue);
		}

		void Propagate(Message const& message)
		{
			Delivery_impl::inst().Propagate(message);
		}

	}
}
