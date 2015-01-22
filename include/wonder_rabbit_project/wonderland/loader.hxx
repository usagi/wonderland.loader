#pragma once

#ifndef EMSCRIPTEN
  #include "loader.detail/native.hxx"
#else
  #include "loader.detail/emscripten.hxx"
#endif

#include <regex>

// common implementations
namespace wonder_rabbit_project
{
  namespace wonderland
  {
    namespace loader
    {
      // sugar pattern: url to urls
      template < class T = void >
      auto load
      ( const std::string& url
      , const bool auto_limit = true
      , const std::int_fast8_t auto_retry = 3
      , std::size_t initial_buffer_reserve_size = 1024
      )
        -> future_t
      { return load<T>( std::vector<std::string>( { url } ), auto_limit, auto_retry, initial_buffer_reserve_size ); }

      // helper: get a basename from a path
      template < class T = void >
      auto basename(const std::string& path)
        -> std::string
      {
        std::cmatch match;
        
        if ( !std::regex_match( path.data(), match, std::regex(".*/(.[^/?]*)") ) )
          throw std::runtime_error("an unknown url pattern, cannot extract a filename.");
        
        auto filename = match.str(1);
        
        return filename;
      }

      // helper: check ready
      template < class T = void >
      auto is_ready(const future_t& future)
        -> bool
      {
        return future.wait_for( std::chrono::seconds(0) ) == std::future_status::ready;
      }

    }
  }
}
