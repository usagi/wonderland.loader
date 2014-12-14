#include <stdexcept>
#include <cstdint>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>
#include <unordered_map>

#include <wonder_rabbit_project/wonderland/loader.hxx>

auto main() -> int
try
{
  using namespace wonder_rabbit_project::wonderland;

  const char* urls[] =
  { "http://upload.wikimedia.org/wikipedia/commons/4/45/Gyuu-don_001.jpg"
  , "https://upload.wikimedia.org/wikipedia/commons/8/81/Butadon_of_Pancho.jpg"
  };

  using future_buffers_t = std::unordered_map< std::string, loader::future_t >;
  future_buffers_t future_buffers;
  
  for ( const auto& url : urls )
  {
    std::cout << "load begin: " << url << std::endl;
    future_buffers.emplace( make_pair( url, loader::load(url) ) );
  }

  {
    bool exit;

#ifndef EMSCRIPTEN
    do
    {
      exit = true;
#else
    emscripten_set_main_loop_arg([](void* argument)
    {
      const auto& future_buffers = *reinterpret_cast<future_buffers_t*>(argument);
      auto exit = true;
#endif

      for ( const auto& future_buffer : future_buffers )
      {
        const auto future_state = loader::is_ready(future_buffer.second);
        std::cout << ( future_state ? "READY" : "WAIT " ) << " : " << future_buffer.first << std::endl;
        
        exit &= future_state;
      }
#ifndef EMSCRIPTEN
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    } while ( !exit );
#else
      if ( exit )
        emscripten_cancel_main_loop();
    }, &future_buffers, 2, false);
#endif
  }

  {
    for ( auto& future_buffer : future_buffers )
    {
      const auto filename = loader::basename(future_buffer.first);
      
      std::cout << "write filename: " << filename << std::endl;
      
      const auto buffer = future_buffer.second.get();

      std::cout << "get succeeded" << std::endl;

      std::ofstream(filename).write( reinterpret_cast<const char*>( buffer.data() ), buffer.size() );
      
      std::cout << "write " << filename << " " << buffer.size() << " [bytes]" << std::endl;
    }
  }
}
catch(const std::exception& e)
{
  std::cerr << "exception: " << e.what();
  return 2;
}
catch(...)
{
  std::cerr << "unknown exception";
  return 1;
}
