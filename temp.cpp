#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

int main()
{
  std::string s{"i am"};
  std::string a, b, c;

  std::istringstream iss{s};

  iss >> a >> b >> c;

  iss >> a >> b >> c;
  std::cout << "a: " << a + '\n' << "b: " << b + '\n' << "c: " << c + '\n';
}
