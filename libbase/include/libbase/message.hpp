#ifndef _LIBBASE_MESSAGING_HPP_
#define _LIBBASE_MESSAGING_HPP_

#include <stdint.h>
#include <libbase/uncopyable.hpp>

namespace Base {

	/// Forward declarations =======================================================================
	struct Message;
	struct MessageManager;
	struct Queue;
	struct Observer_p;
	struct Observable_p;

	///===================================================================================== Message
	struct Message {
		typedef ssize_t type_t;
		typedef ssize_t code_t;
		typedef ssize_t param_t;
		typedef void * data_t;

		~Message();

		Message(const type_t & type = type_t(), const code_t & code = code_t(), const param_t & param = param_t(), const data_t & data = nullptr);

		type_t get_type() const;

		code_t get_code() const;

		param_t get_param() const;

		data_t get_data() const;

		void set_type(const type_t & in);

		void set_code(const code_t & in);

		void set_param(const param_t & in);

		void set_data(const data_t & in);

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

		Queue(Queue && right);

		Queue & operator = (Queue && right);

		void swap(Queue & right);

		void put_message(Message const& message);

		bool get_message(Message & message, size_t timeout_msec = WAIT_FOREVER);

	private:
		struct Queue_impl;
		Queue_impl * m_impl;
	};

	///==================================================================================== Delivery
	namespace Delivery {
		typedef ssize_t SubscribtionId;

		typedef bool (*filter_t)(Message const& message);

		SubscribtionId Subscribe(Queue * queue, Message::type_t type_mask, Message::code_t code_mask, filter_t filter = nullptr);

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
