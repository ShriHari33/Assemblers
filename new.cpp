#include<iostream>
#include<sstream>

int main()
{
    std::string s{"END"};

    std::string a, b, c;
    std::istringstream iss(s);

    iss >> a;
    iss >> b >> c;

    std::cout << "a: " << a << "\nb: " << b << "\nc: " << c;
}
