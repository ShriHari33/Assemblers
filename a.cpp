#include <string>

#include "iostream"

int main()
{
  std::string a = "";

  a += std::to_string(13);

  std::cout << a;
}
