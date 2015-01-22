#pragma once

#include <future>
#include <chrono>
#include <string>
#include <cstdint>
#include <mutex>

#include <boost/optional.hpp>

#ifndef far
#  define far
#endif
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#ifdef far
#  undef far
#endif

namespace
{
  // 暫定的に同時接続数制御機構を入れてみる
#ifdef WONDERLAND_LOADER_AUTO_LIMITS
  std::array< std::mutex, WONDERLAND_LOADER_AUTO_LIMITS > mutexes;
#else
  std::array< std::mutex, 6 > mutexes;
#endif
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
            buffer_t buffer;
            
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

              int last_response;

              for ( const std::string& url : urls )
              {
                try
                {
                  Poco::URI uri( url );

                  Poco::Net::HTTPClientSession session( uri.getHost(), uri.getPort() );
                  
                  const auto path = uri.getPathAndQuery();
                  Poco::Net::HTTPRequest request
                  ( Poco::Net::HTTPRequest::HTTP_GET
                  , path.empty() ? "/" : path, Poco::Net::HTTPMessage::HTTP_1_1
                  );
                  
                  session.sendRequest( request );
                  
                  Poco::Net::HTTPResponse response;
                  
                  auto& bin = session.receiveResponse( response );
                  
                  last_response = static_cast<int>( response.getStatus() );

                  if ( response.getStatus() != Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK )
                    continue;
                  
                  buffer = std::vector<std::uint8_t>();
                  buffer->reserve( initial_buffer_reserve_size );
                  
                  std::copy
                  ( std::istreambuf_iterator< char >( bin )
                  , std::istreambuf_iterator< char >()
                  , std::back_inserter( *buffer )
                  );
                  
                  return buffer;
                }
                catch( const Poco::Exception& e )
                {
                  std::cerr << "Poco Exception and retry after 300[ms]: " << e.what() << " @ "<< url << "\n";
                  if ( buffer )
                    buffer = boost::none;
                }
              }
              
              // 400系エラーはリトライしても意味が無いのでリトライせず終わる
              if ( static_cast<int>(last_response) >= 400 and static_cast<int>(last_response) < 500 )
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
