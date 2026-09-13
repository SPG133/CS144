#include "tcp_sender.hh"
#include "tcp_config.hh"

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return {};
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return {};
}

void TCPSender::push( const TransmitFunction& transmit )
{
  string str { reader().peek() };
  uint64_t length = str.size();
  const uint32_t fill_size = min<uint32_t>( window, TCPConfig::MAX_PAYLOAD_SIZE );
  str = str.substr( string_start, min<uint32_t>( str.size(), fill_size ) );
  bool SYN = false;
  bool FIN = false;
  bool RST = false;
  if ( SYN_sent = false ) {

    SYN = true;
    SYN_sent = true;
  }

  if ( string_start + str.size() == length ) {
    FIN = true;
  }

  TCPSenderMessage message {
    .seqno = isn_.wrap( tcp_start, isn_ ),
    .SYN { SYN },
    .payload { str },
    .FIN { FIN },
    .RST { RST },
  };

  messages.insert( { message, 0 } );
  transmit( message );
  tcp_start += message.sequence_length();
  string_start += str.size();
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  // Your code here.
  return {};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  (void)msg;
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  for ( auto& pair : messages ) {
    pair.second += ms_since_last_tick;
    if ( pair.second > initial_RTO_ms_ ) {
      transmit( pair.first );
      pair.second = 0;
    }
  }
}
