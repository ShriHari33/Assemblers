#include "iostream"
#include "map"
#include "string"
#include "vector"

using namespace std;

string intToRoman(int num)
{
    std::vector<std::pair<char, int>> m{};
    std::string ans{};

    m.push_back({'I', 1});
    m.push_back({'V', 5});
    m.push_back({'X', 10});
    m.push_back({'L', 50});
    m.push_back({'C', 100});
    m.push_back({'D', 500});
    m.push_back({'M', 1000});


    int need = 0;
    auto it = m.rbegin();
    while(num)
    {
        need = num / it->second;
        if(need <= 3 && need > 0)
        {
            std::cout << std::string(need, it->first) << '\n';
            ans += std::string(need, it->first);
            num = num - (need * it->second);
        }
        else if(need == 4)
        {
            char have = it->first;
            //if(have == 'X' || have == 'C')
            {
                ans += std::string(need, it->first);
                num -= (need* it->second);
            }
        }

        it = std::next(it);


        if(it == m.rend())
            break;
    }

    return ans;
}

int main()
{
    std::cout << intToRoman(59);
}