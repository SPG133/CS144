#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

bool Writer::is_closed() const
{
  // Your code here.
  return writer_closed_;
}

void Writer::push( string data )
{
  // Your code here.
  if( is_closed() ) {
    return;
  }
  if ( data.size() > available_capacity() ) {
    data.resize( available_capacity() );
  }
  buffer += data;
  bytes_pushed_ += data.size();
}

void Writer::close()
{
  // Your code here.
  writer_closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.

  return capacity_ - buffer.size();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return bytes_pushed_;
}

bool Reader::is_finished() const
{
  // Your code here.
  return buffer.empty() && writer_closed_ ;
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return bytes_popped_;
}

string_view Reader::peek() const
{
  // Your code here.
  return buffer;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  if ( len > buffer.size() ) {
    len = buffer.size();
  }
  buffer = buffer.substr(len);
  bytes_popped_ += len;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return buffer.size();
}
