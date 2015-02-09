/// @file native.hxx

#pragma once

#include <future>
#include <chrono>
#include <string>
#include <cstdint>
#include <mutex>

#include <boost/optional.hpp>

#ifdef _WIN32
#  ifndef far
#    define far
#  endif
#  ifndef near
#    define near
#  endif
#endif
#include <boost/network.hpp>
#ifdef _WIN32
#  ifdef far
#    undef far
#  endif
#  ifdef near
#    undef near
#  endif
#endif

namespace
{
  // 暫定的に同時接続数制御機構を入れてみる
//#ifdef WONDERLAND_LOADER_AUTO_LIMITS
//  std::array< std::mutex, WONDERLAND_LOADER_AUTO_LIMITS > mutexes;
//#else
  std::array< std::mutex, 6 > mutexes;
//#endif
}

namespace wonder_rabbit_project
{
  namespace wonderland
  {
    namespace loader
    {
      using buffer_t = boost::optional<std::vector<std::uint8_t>>;
      using future_t = std::future<buffer_t>;

      // main feature: load data from url with async/future
      template < class T = void >
      auto load
      ( const std::vector<std::string>& urls
      , const bool auto_limit = true
      , const std::int_fast8_t auto_retry = 3
      , std::size_t initial_buffer_reserve_size = 1024
      )
        -> future_t
      {
        return std::async
        ( std::launch::async
        // TODO: C++14 対応可能次第 move キャプチャー化
        , [ urls, auto_limit, auto_retry, initial_buffer_reserve_size ]
          {
            // temporary buffer
#ifndef _MSC_VER
            buffer_t buffer;
#else
            // MSVC++2015 にしても未だラムダ式で有効であるはずの using/typedef のスコープが無視されたままの対応
            boost::optional<std::vector<std::uint8_t>> buffer;
#endif
            
            int_fast8_t retry = auto_retry;
            
            while( retry-- > 0 )
            {
              std::unique_lock<std::mutex> lock;
              
              if ( auto_limit )
                do
                {
                  for ( auto& mutex : mutexes )
                  {
                    std::unique_lock< std::mutex > lock_( mutex, std::defer_lock );
                    if ( lock_.try_lock() )
                    {
                      lock.swap( lock_ );
                      break;
                    }
                  }
                  
                  if ( lock )
                    break;
                  
                  std::this_thread::sleep_for( std::chrono::microseconds( 300 ) );
                }
                while ( true );

              std::uint16_t last_status = 499u;

              for ( const std::string& url : urls )
              {
                {
                  //using current_client = boost::network::http::client;
                  //*
                  using current_client = boost::network::http::basic_client
                    < boost::network::http::tags::http_async_8bit_tcp_resolve
                    , 1, 1
                    >;
                  //*/

                  current_client::request request( url );
                  
                  request
                    << boost::network::header( "Connection", "close" )
                    << boost::network::header( "UserAgent" , "wonderland.loader" )
                    ;

                  current_client client;
                  const auto response = client.get( request );

                  last_status = status( response );

                  if ( last_status != 200u )
                    continue;

                  const std::string response_body = body( response );
                  buffer = std::vector<std::uint8_t>( response_body.cbegin(), response_body.cend() );

                  return buffer;
                }
              }
              
              // 400系エラーはリトライしても意味が無いのでリトライせず終わる
              if ( last_status >= 400u and last_status < 500u )
                break;

              std::this_thread::sleep_for( std::chrono::milliseconds( 300 ) );
            }

            return buffer;
          }
        );
      }
      

    }
  }
}
