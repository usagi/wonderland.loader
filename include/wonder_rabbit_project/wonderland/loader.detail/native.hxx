#pragma once

#include <future>
#include <chrono>
#include <string>
#include <cstdint>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

namespace wonder_rabbit_project
{
  namespace wonderland
  {
    namespace loader
    {
      using buffer_t = std::vector<std::uint8_t>;
      using future_t = std::future<buffer_t>;

      // main feature: load data from url with async/future
      template < class T = void >
      auto load(const std::string& url)
        -> future_t
      {
        return std::async
        ( std::launch::async
        , [ url ]
          {
            // temporary buffer
            buffer_t buffer;
            
            cURLpp::Cleanup cleaner;
            cURLpp::Easy request;
            request.setOpt( new cURLpp::Options::Url(url) );
            request.setOpt
            ( new cURLpp::Options::WriteFunction
              ( cURLpp::Types::WriteFunctionFunctor
                ( [ &buffer ] (char* p, std::size_t size, std::size_t blocks)
                  {
                    const auto total_size = size * blocks;
                    std::copy( p, p + total_size, std::back_inserter(buffer) );
                    return total_size;
                  }
                )
              )
            );

            // it is blocking proccess!
            request.perform();

            return buffer;
          }
        );
      }

    }
  }
}
