#include <libbase/message.hpp>

namespace Base {

	Message::~Message()
	{
	}

	Message::Message(const type_t & type, const code_t & code, const param_t & param, const data_t & data):
		m_type(type),
		m_code(code),
		m_param(param),
		m_data(data)
	{
	}

	Message::type_t Message::get_type() const
	{
		return m_type;
	}

	Message::code_t Message::get_code() const
	{
		return m_code;
	}

	Message::param_t Message::get_param() const
	{
		return m_param;
	}

	Message::data_t Message::get_data() const
	{
		return m_data;
	}

	void Message::set_type(const type_t & in)
	{
		m_type = in;
	}

	void Message::set_code(const code_t & in)
	{
		m_code = in;
	}

	void Message::set_param(const param_t & in)
	{
		m_param = in;
	}

	void Message::set_data(const data_t & in)
	{
		m_data = in;
	}
}
