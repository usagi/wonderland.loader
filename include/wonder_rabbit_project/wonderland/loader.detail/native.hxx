#pragma once

#include <future>
#include <chrono>
#include <string>
#include <cstdint>

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>

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
            
            try
            {
              
              Poco::URI uri( url );

              Poco::Net::HTTPClientSession session( uri.getHost(), uri.getPort() );
              
              const auto path = uri.getPathAndQuery();
              Poco::Net::HTTPRequest request( Poco::Net::HTTPRequest::HTTP_GET, path.empty() ? "/" : path, Poco::Net::HTTPMessage::HTTP_1_1 );
              
              session.sendRequest( request );
              
              Poco::Net::HTTPResponse response;
              
              if ( response.getStatus() != Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK )
                return buffer;
              
              auto& bin = session.receiveResponse( response );
              std::copy( std::istreambuf_iterator< char >( bin ), std::istreambuf_iterator< char >(), std::back_inserter( buffer ) );
            }
            catch( const Poco::Exception& e )
            {
              std::cerr << "== Poco Exception ==> " << e.what();
              throw e;
            }

            return buffer;
          }
        );
      }

    }
  }
}
