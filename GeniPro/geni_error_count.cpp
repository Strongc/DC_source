// geni_error_count.cpp

#include "geni_error_count.h"


unsigned int dir_timeout_errors;
unsigned int auto_timeout_errors;
unsigned int rx_break_header_errors;
unsigned int rx_break_receiving_errors;

void ResetGeniErrorCount()
{
  dir_timeout_errors = 0;
  auto_timeout_errors = 0;
  rx_break_header_errors = 0;
  rx_break_receiving_errors =0;
}

// geni_error_count.cpp
