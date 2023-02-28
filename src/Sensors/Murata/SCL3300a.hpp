#ifndef __SCL3300A_HPP
#define __SCL3300A_HPP

#include "SCL3X00.hpp"

#include <complex>
#include <cmath>
#include <iterator>

namespace Xerxes
{

constexpr float G = 9.819f; // m/s^2
constexpr float SQRT2 = 1.41421356237f;
constexpr double PI = 3.1415926536;

unsigned int bitReverse(unsigned int x, int log2n);

void real_fft(std::vector<double>& a, std::vector<std::complex<double>>& b, int log2n);

template<class Iter_T>
void fft(Iter_T a, Iter_T b, int log2n)
{
  typedef typename std::iterator_traits<Iter_T>::value_type complex;
  const std::complex J(0, 1);
  int n = 1 << log2n;
  for (unsigned int i=0; i < n; ++i) {
    b[bitReverse(i, log2n)] = a[i];
  }
  for (int s = 1; s <= log2n; ++s) {
    int m = 1 << s;
    int m2 = m >> 1;
    std::complex w(1, 0);
    std::complex wm = exp(-J * (PI / m2));
    for (int j=0; j < m2; ++j) {
      for (int k=j; k < n; k += m) {
        std::complex t = w * b[k + m2];
        std::complex u = b[k];
        b[k] = u + t;
        b[k + m2] = u - t;
      }
      w *= wm;
    }
  }
}


class SCL3300a : public SCL3X00
{
private:
    /**
     * @brief Get the Deg From Packet object
     * 
     * @param packet    - received data
     * @param mode      - mode of sensor (acceleration or angle)
     * @return double  - angle in degrees 
     */
    double getAccFromPacket(const std::unique_ptr<SclPacket_t>& packet, const cmd_t mode = CMD::Change_to_mode_1);

public:
    using SCL3X00::SCL3X00;
    void init();
    void update();


    /**
     * @brief Get the AccelX, AccelY, AccelZ in
     * 
     * @param os output stream
     * @param scl sensor
     * @return std::ostream& 
     */
    friend std::ostream& operator<<(std::ostream& os, const SCL3300a& scl);
};

} //namespace Xerxes    

#endif // !__SCL3300A_HPP