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
