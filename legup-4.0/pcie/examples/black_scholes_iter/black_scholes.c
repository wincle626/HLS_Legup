#include <math.h>
#include <stdio.h>

double black_scholes_value(const double S, const double E, const double r, const double sigma, const double T, const double gaussian_random_number)
{
  const double current_value = S * exp((r - (sigma*sigma) / 2.0) * T + sigma * sqrt(T) * gaussian_random_number);
  return exp(-r * T) * ((current_value - E < 0.0) ? 0.0 : current_value - E);
}

int main()
{
  double bs_val = black_scholes_value(1.0, 1.0, 1.0, 3.0, 1.0, 2.5);
  printf("%lf\n", bs_val);
  return (int)(bs_val * 1000);
}
