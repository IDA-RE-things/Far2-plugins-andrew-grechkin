#ifndef _LIBBASE_MESSAGE_HPP_
#define _LIBBASE_MESSAGE_HPP_

#include <stdint.h>
#include <libbase/uncopyable.hpp>

namespace Base {

	/// Forward declarations =======================================================================
	struct Message;
	struct MessageManager;
	struct Queue;
	struct Queue_impl;
	struct Observer_p;
	struct Observable_p;

	///===================================================================================== Message
	struct Message {
		typedef ssize_t type_t;
		typedef ssize_t code_t;
		typedef ssize_t param_t;
		typedef void * data_t;

		~Message();

		Message(type_t type = type_t(), code_t code = code_t(), param_t param = param_t(), data_t data = nullptr);

		type_t get_type() const;

		code_t get_code() const;

		param_t get_param() const;

		data_t get_data() const;

	private:
		type_t m_type;
		code_t m_code;
		param_t m_param;
		data_t m_data;
	};

	///======================================================================================= Queue
	struct Queue: private Base::Uncopyable {

		static const size_t WAIT_FOREVER;

		~Queue();

		Queue();

		void put_message(Message const& message);

		bool get_message(Message & message, size_t timeout_msec = WAIT_FOREVER);

	private:
		Queue_impl * m_impl;
	};

	///==================================================================================== Delivery
	namespace Delivery {
		typedef ssize_t SubscribtionId;

		typedef bool (*filter_t)(Message const& message);

		SubscribtionId Subscribe(Message::type_t type_mask, Message::code_t code_mask, Queue * queue, filter_t filter = nullptr);

		void Unsubscribe(SubscribtionId id);

		void Unsubscribe(Queue const* queue);

		void Propagate(Message const& message);
	};

	///================================================================================== Observer_p
	struct Observer_p {
		virtual ~Observer_p();

		virtual void notify(Message const& event) = 0;

	public:
		Observer_p();

		Observer_p(MessageManager * manager) :
			m_manager(manager)
		{
		}

	private:
		MessageManager * m_manager;
	};

	///================================================================================ Observable_p
	struct Observable_p {
		virtual ~Observable_p();

	public:
		Observable_p();

		Observable_p(MessageManager * manager) :
			m_manager(manager),
			m_changed(false)
		{
		}

		void register_observer(Observer_p * observer);

		void unregister_observer(Observer_p * observer);

		void notify_all(Message const& event) const;

		void set_changed(bool changed) const;

		bool get_changed() const;

	private:
		MessageManager * m_manager;
		mutable bool m_changed;
	};

}

#endif
