#include <iostream>
#include <string>
#include <cmath>

int main()
{
    double a, b;
    int n = 6;
    double x[] = {1, 2, 3, 4, 5, 6};
    double y[] = {11, 11, 11, 11, 11, 14};
	double pxy, sx, sy, sx2, sy2;
	pxy=sx=sy=sx2=sy2=0.0;
	for(int i=0; i<n; i++) {
		sx += x[i];
		sy += y[i];
		sx2 += x[i]*x[i];
		sy2 += y[i]*y[i];
		pxy += x[i]*y[i];
	}
    // std::cout << "pxy = " << pxy << "\n";
    // std::cout << "sx = " << sx << "\n";
    // std::cout << "sy = " << sy << "\n";
    // std::cout << "sx2 = " << sx2 << "\n";
    // std::cout << "sy2 = " << sy2 << "\n";
	b = (n*pxy-sx*sy) / (n*sx2-sx*sx);
	a = (sy-b*sx)/n;
    std::cout << "pendiente = " << a << "\n";
    std::cout << "ordenada = " << b << "\n";

	//valores medios
	double mx = sx / n;
	double my = sy / n;
	pxy=sx2=sy2=0.0;
	for(int i=0; i<n; i++) {
		pxy += (x[i]-mx) * (y[i]-my);
		sx2 += std::pow(x[i]-mx, 2);
		sy2 += std::pow(y[i]-my, 2);
	}
	double c = pxy / std::sqrt(sx2*sy2);
	std::cout << "correlation = " << c << std::endl;
}

