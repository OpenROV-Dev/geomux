#pragma once

// Include
#include <memory>

namespace util
{
	// Correlary to make_shared
	template<typename T, typename ...Args>
	std::unique_ptr<T> make_unique( Args&& ...args )
	{
		return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
	}
}