#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // Your code here.

  if ( is_last_substring ) {
    last_index = first_index + data.size();
    if ( next_id == last_index ) {
      output_.writer().close();
      return;
    }
    last = true;
  }

  if ( ( first_index + data.size() ) <= next_id ) {
    return;
  }

  if ( first_index < next_id ) {
    data = data.substr( next_id - first_index );
    first_index = next_id;
  }

  const uint64_t first_unacceptable = next_id + output_.writer().available_capacity();
  if ( first_index >= first_unacceptable ) {
    return;
  }
  if ( first_index + data.size() > first_unacceptable ) {
    data = data.substr( 0, first_unacceptable - first_index );
  }
  uint64_t end_index = first_index - 1 + data.size();

  bool left_in = false;
  bool right_in = false;
  uint64_t left_block = 0;
  uint64_t right_block = -1;

  for ( const auto& [key, value] : buffer ) {
    if ( key <= end_index ) {
      if ( ( key <= first_index ) && ( first_index <= ( key - 1 + value.size() ) ) ) {
        left_block = key;
        left_in = true;
      }
      if ( ( key <= end_index ) && ( end_index <= ( key - 1 + value.size() ) ) ) {
        right_block = key;
        right_in = true;
      }
      if ( left_in && right_in && right_block == left_block ) {
        return;
      }
    }
  }

  if ( !left_in && !right_in ) {
    for ( auto it = buffer.begin(); it != buffer.end(); ) {
      if ( first_index <= it->first && it->first <= end_index ) {
        it = buffer.erase( it );
      } else {
        ++it;
      }
    }
  }
  if ( left_in && right_in ) {
    data = data.substr( buffer[left_block].size() - ( first_index - left_block ),
                        data.size() - ( left_block + buffer[left_block].size() - first_index )
                          - ( end_index - right_block + 1 ) );
    first_index = left_block + buffer[left_block].size();
    end_index = right_block - 1;
    for ( auto it = buffer.begin(); it != buffer.end(); ) {
      if ( first_index <= it->first && it->first <= end_index ) {
        it = buffer.erase( it );
      } else {
        ++it;
      }
    }
  }
  if ( left_in && !right_in ) {
    data = data.substr( buffer[left_block].size() - ( first_index - left_block ),
                        data.size() - ( left_block + buffer[left_block].size() - first_index ) );
    first_index = left_block + buffer[left_block].size();
    for ( auto it = buffer.begin(); it != buffer.end(); ) {
      if ( first_index <= it->first && it->first <= end_index ) {
        it = buffer.erase( it );
      } else {
        ++it;
      }
    }
  }
  if ( !left_in && right_in ) {
    data = data.substr( 0, data.size() - ( end_index - right_block + 1 ) );
    end_index = right_block - 1;
    for ( auto it = buffer.begin(); it != buffer.end(); ) {
      if ( first_index <= it->first && it->first <= end_index ) {
        it = buffer.erase( it );
      } else {
        ++it;
      }
    }
  }

  if ( !data.empty() ) {
    buffer[first_index] = data;
  }

  for ( auto it = buffer.begin(); it != buffer.end(); ) {
    if ( it->first == next_id ) {
      // uint64_t key = it->first;
      string answer = it->second;

      // uint64_t capacity = output_.writer().available_capacity();

      /*if ( capacity < answer.size() ) {
        answer = answer.substr( 0, capacity );
      }*/

      it = buffer.erase( it );

      output_.writer().push( answer );
      next_id += answer.size();

      if ( last && next_id == last_index ) {
        output_.writer().close();
      }
    } else {
      ++it;
    }
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  uint64_t result = 0;
  for ( const auto& [key, value] : buffer ) {
    result += value.size();
  }
  return result;
}
