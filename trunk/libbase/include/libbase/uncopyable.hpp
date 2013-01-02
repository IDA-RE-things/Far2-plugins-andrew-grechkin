#ifndef _LIBBASE_UNCOPYABLE_HPP_
#define _LIBBASE_UNCOPYABLE_HPP_

namespace Base {

	class Uncopyable {
		typedef Uncopyable this_type;

	protected:
		~Uncopyable()
		{
		}

		Uncopyable()
		{
		}

	private:
		Uncopyable(const this_type &) = delete;

		this_type & operator = (const this_type &) = delete;
	};

}

#endif
