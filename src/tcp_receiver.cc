#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here.
  if ( message.RST ) {
    /*ISN = Wrap32 { 0 };
    checkpoint = 0;
    start = 0;
    restart = true;*/
    reader().set_error();
    return;
  }
  if ( message.SYN ) {
    ISN = message.seqno;
    start = 1;
    checkpoint = reassembler_.get_next_id();
  }
  if ( start ) {
    uint64_t actual_position = 0;
    if ( message.SYN )
      actual_position = message.seqno.unwrap( ISN, checkpoint + ISN.unwrap( ISN, 0 ) );
    else
      actual_position = message.seqno.unwrap( ISN, checkpoint + ISN.unwrap( ISN, 0 ) ) - 1;
    reassembler_.insert( actual_position, message.payload, message.FIN );
    checkpoint = reassembler_.get_next_id();
  }
  if ( writer().is_closed() ) {
    start = -1;
  }
}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  TCPReceiverMessage receiveMessage;
  if ( start == 1 ) {
    receiveMessage.ackno = ISN.wrap( checkpoint + 1 + ISN.unwrap( ISN, 0 ), ISN );
  } else if ( start == -1 ) {
    receiveMessage.ackno = ISN.wrap( checkpoint + 2 + ISN.unwrap( ISN, 0 ), ISN );
  }
  receiveMessage.window_size = min( uint64_t { 65535 }, reassembler_.writer().available_capacity() );
  receiveMessage.RST = restart || reader().has_error();
  return receiveMessage;
}
