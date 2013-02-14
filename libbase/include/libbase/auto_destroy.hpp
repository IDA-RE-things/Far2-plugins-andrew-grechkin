#ifndef _LIBBASE_AUTO_DESTROY_HPP_
#define _LIBBASE_AUTO_DESTROY_HPP_

namespace Base {

	struct Destroyable {
		virtual ~Destroyable();

		virtual void destroy() const = 0;
	};

	template<typename Type>
	struct auto_destroy {
		~auto_destroy()
		{
			m_ptr->destroy();
		}

		auto_destroy(Type ptr) :
			m_ptr(ptr)
		{
		}

		Type operator ->()
		{
			return m_ptr;
		}

		const Type operator ->() const
		{
			return m_ptr;
		}

	private:
		Type m_ptr;
	};

}

#endif
