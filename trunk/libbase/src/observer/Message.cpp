#include <libbase/message.hpp>

namespace Base {

	Message::~Message()
	{
	}

	Message::Message(type_t type, code_t code, param_t param, data_t data):
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
}
