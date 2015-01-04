#pragma once

#include <emscripten/emscripten.h>

#include <vector>
#include <future>
#include <cstdint>

namespace wonder_rabbit_project
{
  namespace wonderland
  {
    namespace loader
    {
      using buffer_t = std::vector<std::uint8_t>;
      using future_t = std::future<buffer_t>;

      namespace detail
      {
        using promise_t = std::promise<buffer_t>;
      }

      template < class T = void >
      auto load(const std::string& url)
        -> future_t
      {
        auto promise_ptr = new detail::promise_t();
        auto future = promise_ptr->get_future();

        constexpr auto request_type = "GET";
        constexpr auto request_parameter = "";
        auto argument = promise_ptr;
        constexpr auto free_after_on_load = true;

        auto on_load     = [](unsigned handle, void* user_data, void* data, unsigned size)
        {
          auto promise_ptr = reinterpret_cast< detail::promise_t* >( user_data );
          auto typed_data = reinterpret_cast< std::uint8_t* >( data );
          buffer_t buffer( typed_data, typed_data + size);
          promise_ptr->set_value(std::move(buffer));
          delete promise_ptr;
        };
        
        auto on_error    = [](unsigned, void* user_data, int, const char*)
        {
          auto promise_ptr = reinterpret_cast< detail::promise_t* >( user_data );
          delete promise_ptr;
        };
        
        auto on_progress = [](unsigned, void*, int, int)
        {
        };

        // async process!
        //   ref: http://kripken.github.io/emscripten-site/docs/api_reference/emscripten.h.html#c.emscripten_async_wget2_data
        emscripten_async_wget2_data
        ( url.data()
        , request_type
        , request_parameter
        , argument
        , free_after_on_load
        , on_load
        , on_error
        , on_progress
        );

        return future;
      }

    }
  }
}

