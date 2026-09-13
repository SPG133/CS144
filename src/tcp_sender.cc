#include "tcp_sender.hh"
#include "tcp_config.hh"

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  uint64_t result = 0;
  for ( auto& pair : messages ) {
    result += pair.first.sequence_length();
  }
  return result;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return retransmit_times;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  string str { reader().peek() };
  uint64_t length = str.size();
  if ( window == 0 ) {
    return;
  }
  uint32_t window_size = max<uint32_t>( 1, window );
  const uint32_t fill_size = min<uint32_t>( window_size, TCPConfig::MAX_PAYLOAD_SIZE );
  if ( str.size() == string_start ) {
    str = "";
  } else {
    str = str.substr( string_start, min<uint32_t>( str.size(), fill_size ) );
  }
  bool SYN = false;
  bool FIN = false;
  bool RST = false;
  if ( ( SYN_sent == false ) ) {

    SYN = true;
    SYN_sent = true;
  }

  if ( string_start + str.size() == length && ( reader().is_finished() || writer().is_closed() )
       && ( !FIN_sent ) ) {
    FIN = true;
  }

  TCPSenderMessage message {
    .seqno = isn_.wrap( tcp_start, isn_ ),
    .SYN { SYN },
    .payload { str },
    .FIN { FIN },
    .RST { RST },
  };
  if ( message.sequence_length() != 0 ) {
    messages.emplace_back( message, retransmission_timer( 0, initial_RTO_ms_, 1 ) );
    transmit( message );
    window -= message.sequence_length();

    if ( FIN ) {
      FIN_sent = true;
    }
  }

  tcp_start += message.sequence_length();
  string_start += str.size();
  if ( window != 0 && ( ( string_start ) != length ) ) {
    push( transmit );
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  return TCPSenderMessage {
    .seqno = isn_.wrap( tcp_start, isn_ ),
    .SYN { false },
    .payload { "" },
    .FIN { false },
    .RST { false },
  };
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  uint64_t max_seqno = 0;

  for ( const auto& pair : messages ) {
    max_seqno = std::max( max_seqno, pair.first.seqno.unwrap( isn_, tcp_start ) + pair.first.sequence_length() );
  }
  if ( !msg.ackno || msg.ackno->unwrap( isn_, tcp_start ) <= ack_num
       || ( msg.ackno->unwrap( isn_, tcp_start ) > ( max_seqno ) ) ) {
    if ( msg.ackno->unwrap( isn_, tcp_start ) == ack_num ) {
      window = msg.window_size;
    }
    return;
  } else {
    ack_num = msg.ackno->unwrap( isn_, tcp_start );
    window = msg.window_size;
    retransmit_times = 0;
  }
  for ( auto it = messages.begin(); it != messages.end(); ) {
    auto& pair = *it;
    if ( ( pair.first.seqno.unwrap( isn_, tcp_start ) + pair.first.sequence_length() )
         <= ( ack_num ) ) {      // DELETE +1 IN  ( ack_num )
      it = messages.erase( it ); // erase 返回下一个有效迭代器
    } else {
      ++it;
    }
  }
  for ( auto& pair1 : messages ) {
    pair1.second.time = 0;
    pair1.second.RTO = initial_RTO_ms_;
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  messages.sort( [this]( const auto& lhs, const auto& rhs ) {
    return lhs.first.seqno.unwrap( isn_, tcp_start ) < rhs.first.seqno.unwrap( isn_, tcp_start );
  } );

  for ( auto& pair : messages ) {
    pair.second.time += ms_since_last_tick;
    if ( pair.second.time >= pair.second.RTO ) {
      transmit( pair.first );
      pair.second.send_times++;
      retransmit_times++;
      for ( auto& pair1 : messages ) {
        pair1.second.time = 0;
        pair1.second.RTO *= 2;
      }
      return;
    }
  }
}
