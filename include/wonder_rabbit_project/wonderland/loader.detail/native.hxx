#pragma once

#include <future>
#include <chrono>
#include <string>
#include <cstdint>

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
            
            auto retry = 8;
            
            while( retry-- )
            {
              try
              {
                Poco::URI uri( url );

                Poco::Net::HTTPClientSession session( uri.getHost(), uri.getPort() );
                
                const auto path = uri.getPathAndQuery();
                Poco::Net::HTTPRequest request( Poco::Net::HTTPRequest::HTTP_GET, path.empty() ? "/" : path, Poco::Net::HTTPMessage::HTTP_1_1 );
                
                session.sendRequest( request );
                
                Poco::Net::HTTPResponse response;
                
                if ( response.getStatus() != Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK )
                {
                  std::cerr << "HTTP RESPONSE IS NOT 200 and retry after 30[ms]: " << response.getStatus();
                  std::this_thread::sleep_for( std::chrono::milliseconds( 30 ) );
                  continue;
                }
                
                auto& bin = session.receiveResponse( response );
                std::copy( std::istreambuf_iterator< char >( bin ), std::istreambuf_iterator< char >(), std::back_inserter( buffer ) );
                
                break;
              }
              catch( const Poco::Exception& e )
              {
                std::cerr << "Poco Exception and retry after 30[ms]: " << e.what();
                buffer.clear();
                std::this_thread::sleep_for( std::chrono::milliseconds( 30 ) );
                continue;
              }
            }

            return buffer;
          }
        );
      }

    }
  }
}
