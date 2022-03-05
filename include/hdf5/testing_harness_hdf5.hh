


#ifndef TESTINGHARNESSHDF5_HH
#define TESTINGHARNESSHDF5_HH
#include <iostream>


#ifndef RC_ENUM
#define RC_ENUM
enum RC
{
    RC_OK = 0,
    RC_ERR = -1
};
#endif //RC_ENUM

struct debugLog {
  private:
    bool on;
    bool zero_rank_logging;
    int my_rank;

  public:
  debugLog(bool turn_on, bool zero_rank_log=false, int rank=-1) {
    on = turn_on;
    zero_rank_logging = zero_rank_log;
    my_rank = rank;
  }
  void set_rank(int rank) {
    my_rank = rank;
  }
  void turn_on_zero_rank_logging() {
    zero_rank_logging = true;
  }
  void turn_off_zero_rank_logging() {
    zero_rank_logging = false;
  }
  void turn_on_logging() {
    on = true;
  }
  void turn_off_logging() {
    on = false;
  }
  template<typename T> debugLog& operator << (const T& x) {
   if(on || (zero_rank_logging && (my_rank == 0) )) {
      std::cout << x;
    }
    return *this;
  }
  debugLog& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
   if(on || (zero_rank_logging && (my_rank == 0) )) {
      std::cout << manipulator;
    }
    return *this;
  }
};

// #include <sstream>
// #include <boost/archive/text_iarchive.hpp> 
// #include <boost/archive/text_oarchive.hpp>


// template <class T>
// void make_single_val_data (T val, std::string &serial_str) {
//     extreme_debug_log << "about to make single val data \n";
//     stringstream ss;
//     boost::archive::text_oarchive oa(ss);
//     oa << val;
//     serial_str = ss.str();
// }

// template <class T1, class T2>
// void make_double_val_data (T1 val_0, T2 val_1, std::string &serial_str) {
//     extreme_debug_log << "about to make double val data \n";
//     stringstream ss;
//     boost::archive::text_oarchive oa(ss);
//     oa << val_0;
//     oa << val_1;
//     serial_str = ss.str();
// }


#endif //TESTINGHARNESSHDF5_HH