#include <status.hpp>

// Status flags byte
std::bitset<8> status;

int error_cycle;

void Status::statusOn(int bit){
  status.set(bit);
}

void Status::statusOff(int bit){
  status.reset(bit);
}

void Status::setErrorCode(unsigned int error) {
  status=((status.to_ulong() & ~0xf0) | ((error << 4) & 0xf0));
}

unsigned int Status::getErrorCode(){
  return (unsigned int)(status.to_ulong() >> 4 );
}

/***
 * Clear status error code after MAX_ERROR_LIFE_CYCLE
 **/
void Status::updateStatusError()
{
  if (error_cycle++ == MAX_ERROR_LIFE_CYCLE) {
    setErrorCode(0);
    error_cycle = 0;
  }
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_STATUSHANDLER)
Status st;
#endif

