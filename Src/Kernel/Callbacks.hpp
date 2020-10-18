#ifndef M_CALLBACKS
#define M_CALLBACKS
#pragma once

#include <Kernel/Asserts.hpp>
#include <Kernel/Types.hpp>

#include <functional>
#include <list>

namespace m
{
    template<typename rType, typename...Args>
	class Callback {
	public:
		using functionType = rType(Args...);

		Callback() {}

		template <typename CArgs>
		Callback(CArgs* a_owner, rType(CArgs::* a_func)(Args...))
		{
			m_func = [a_owner, a_func](Args...a_args)->rType { return (a_owner->*a_func)(a_args...); };
		}

		Callback(std::function<functionType> a_func)
		{
			m_func = a_func;
		}

		template <typename CArgs>
		void set(CArgs* a_owner, rType (CArgs::*a_func)(Args...))
		{
			m_func = [a_owner, a_func](Args...a_args)->rType { return (a_owner->*a_func)(a_args...);};
		}

		void set(std::function<functionType> a_func)
		{
			m_func = a_func;
		}

		rType call(Args...a_args)
		{
			mHardAssert(Bool(m_func));
			return m_func(a_args...);
		}

        rType operator()(Args...a_args)
        {
            call(a_args...);
        }

		friend bool operator==(const Callback& lhs, const Callback& rhs)
		{
			return lhs.m_func.template target<functionType>() == rhs.m_func.template target<functionType>();
		}

	private:
		std::function<functionType> m_func;
	};

	template<typename...Args>
	class Signal
	{
	public:
		using handle = typename std::list<Callback<void, Args...>>::iterator;

		handle attachToSignal(Callback<void, Args...> a_callback)
		{
			m_callbacks.push_front(a_callback);
			return m_callbacks.begin();

		}

		void detachFromSignal(Callback<void, Args...> a_callback)
		{
			m_callbacks.remove(a_callback);
		}

		void detachFromSignal(const handle& a_handle)
		{
			m_callbacks.erase(a_handle);
		}

		void call(Args...a_args)
		{
			for (Callback<void, Args...> cbs : m_callbacks)
			{
				cbs.call(a_args...);
			}
		}

		void clear()
		{
			m_callbacks.clear();
		}

	private:
		std::list<Callback<void, Args...>> m_callbacks;
	};
};

#endif //M_CALLBACKS