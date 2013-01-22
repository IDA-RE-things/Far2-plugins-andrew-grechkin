#include <libbase/messaging.hpp>
#include <libbase/lock.hpp>

#include <vector>

namespace {

	struct DeliveryMapping {
		DeliveryMapping(Base::Queue * queue, Base::Message::type_t type_mask, Base::Message::code_t code_mask, Base::Delivery::filter_t filter):
			m_queue(queue),
			m_type_mask(type_mask),
			m_code_mask(code_mask),
			m_filter(filter)
		{
		}

		bool operator () (const Base::Message & message) const
		{
			if (check_mask(message) && check_filter(message))
			{
				m_queue->put_message(message);
				return true;
			}
			return false;
		}

		bool operator == (const Base::Queue * queue) const
		{
			return m_queue == queue;
		}

	private:
		bool check_mask(const Base::Message & message) const
		{
			return (m_type_mask & message.get_type()) && (m_code_mask & message.get_code());
		}

		bool check_filter(const Base::Message & message) const
		{
			return !m_filter || m_filter(message);
		}

		Base::Queue * m_queue;
		Base::Message::type_t m_type_mask;
		Base::Message::code_t m_code_mask;
		Base::Delivery::filter_t m_filter;
	};

	typedef std::pair<Base::Delivery::SubscribtionId, DeliveryMapping> dm_t;

	struct Delivery_impl: private Base::Lock::CriticalSection, private std::vector<dm_t> {

		Delivery_impl():
			m_id_generator(0)
		{
		}

		static Delivery_impl & inst();

		Base::Delivery::SubscribtionId Subscribe(Base::Queue * queue, Base::Message::type_t type_mask, Base::Message::code_t code_mask, Base::Delivery::filter_t filter);

		void Unsubscribe(Base::Delivery::SubscribtionId id);

		void Unsubscribe(const Base::Queue * queue);

		void SendRound(const Base::Message & message) const;

	private:
		Base::Delivery::SubscribtionId GetNextId()
		{
			return ++m_id_generator;
		}

		Base::Delivery::SubscribtionId m_id_generator;

		static Delivery_impl m_instance;
	};

	Delivery_impl Delivery_impl::m_instance;

	Delivery_impl & Delivery_impl::inst()
	{
		return m_instance;
	}

	Base::Delivery::SubscribtionId Delivery_impl::Subscribe(Base::Queue * queue, Base::Message::type_t type_mask, Base::Message::code_t code_mask, Base::Delivery::filter_t filter)
	{
		lock();
		auto id = GetNextId();
		emplace_back(id, DeliveryMapping(queue, type_mask, code_mask, filter));
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

	void Delivery_impl::Unsubscribe(const Base::Queue * queue)
	{
		lock();
		for (auto it = rbegin(); it != rend(); ++it) {
			if (it->second == queue)
				erase(it.base());
		}
		release();
	}

	void Delivery_impl::SendRound(const Base::Message & message) const
	{
		lock();
//		std::for_each(begin(), end(), [&](dm_t const& item) {
//			item.second(message);
//		});
		for (const dm_t & item : *this) {
			item.second(message);
		}
		release();
	}

}

namespace Base {
	namespace Delivery {

		SubscribtionId Subscribe(Queue * queue, Message::type_t type_mask, Message::code_t code_mask, filter_t filter)
		{
			return Delivery_impl::inst().Subscribe(queue, type_mask, code_mask, filter);
		}

		void Unsubscribe(SubscribtionId id)
		{
			Delivery_impl::inst().Unsubscribe(id);
		}

		void Unsubscribe(const Queue * queue)
		{
			Delivery_impl::inst().Unsubscribe(queue);
		}

		void SendRound(const Message & message)
		{
			Delivery_impl::inst().SendRound(message);
		}

	}
}
