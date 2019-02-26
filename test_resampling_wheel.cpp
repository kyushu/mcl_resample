
#include <iostream>
#include <algorithm>
#include <random> //C++ 11 Random Numbers
#include <vector>

std::random_device rd;
std::mt19937 gen(rd());

double gen_real_random()
{
    // Generate real random between 0 and 1
    std::uniform_real_distribution<double> real_dist(0.0, 1.0); //Real
    return real_dist(gen);
}

double mod(double first_term, double second_term)
{
    // Compute the modulus
    return first_term - (second_term)*floor(first_term / (second_term));
}

int main(int argc, char const *argv[])
{
    //                           0    1    2    3    4    5    6    7
    std::vector<double> weights{0.6, 1.2, 0.6, 2.4, 0.6, 1.2, 1.8, 0.8};

    std::vector<int> selectP;

    double beta = 0;
    double mw = *(std::max_element(weights.begin(), weights.end()) );

    // std::cout << mw << '\n';
    int index = gen_real_random();
    for(size_t i=0; i < weights.size(); ++i)
    {
        beta += gen_real_random() * 2.0*mw;
        while(weights[index] < beta)
        {
            std::cout << "beta: " << beta << '\n';
            printf("w[%d]: %lf\n", index, weights[index]);

            beta -= weights[index];
            printf("beta:%lf\n", beta);
            index = mod(index+1, weights.size());
            printf("index: %d\n",index);
        }
        printf("select: %d\n", index);
        selectP.push_back(index);
    }

    printf("####################################3\n");
    for(size_t i = 0; i < selectP.size(); ++i)
    {
        printf("%d: %d\n", i, selectP[i]);
    }
    return 0;
}
