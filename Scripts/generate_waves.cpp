/**
 * @file generate_waves.cpp
 * @brief generate the file waves.h
*/

#include <bits/stdc++.h>

using namespace std;

#define sample_cnt 120
#define max_value 0xffff

int s_sin(int x)
{
    float step = (2.0*M_PI)/(float)sample_cnt;
    return sin(step*(float)x)*(max_value/2.0)+(max_value/2.0);
}
int s_square(int x)
{
    return max_value*(x<sample_cnt/2);
}
int s_triangle(int x)
{
    float m = max_value/(float)(sample_cnt/2.0);
    if(x <= sample_cnt/4) return x*m+max_value/2.0;
    else if(x < (sample_cnt*3)/4) return max_value-(x*m-max_value/2);
    return x*m-(max_value*3)/2;
}
int s_saw(int x)
{
    //int m = max_value/(sample_cnt/2);
    //return (x%(sample_cnt/2+1))*m;
    float m = max_value/(float)(sample_cnt-1);
    return x*m;
}
int s_gaussian(int x)
{
    float m = -log(1/(float)(max_value));
    return max_value*exp(-m*pow((x-sample_cnt/2)/(float)(sample_cnt/2.0), 2));
}
int s_stair(int x)
{
    float m = max_value/(float)(sample_cnt/10);
    return ((x+1)/10)*m;
}

void generate(string name, int (*f)(int))
{
    printf("{//%s\n\t", name.c_str());
    int cnt = 0;
    for(int i=0;i<sample_cnt;i++)
    {
        printf("0x%x", max(0, f(i)));
        cnt++;
        if(i == sample_cnt-1)
            break;
        printf(", ");
        if(cnt >= sqrt(sample_cnt))
        {
            cnt = 0;
            printf("\n\t");
        }
    }
    printf("\n}");
}

int main()
{
    freopen("waves.h", "w", stdout);
    vector<pair<int (*)(int), string>> functions = {
                {s_sin, "sin"},
                {s_square, "square"},
                {s_triangle, "triangle"},
                {s_saw, "saw"},
                {s_gaussian, "gaussian"},
                {s_stair, "stair"}
            };
    printf("#ifndef WAVES_H\n#define WAVES_H\n\n");
    printf("\n\nstatic int waves_table[%d][%d] = {\n", functions.size(), sample_cnt);
    generate(functions[0].second, functions[0].first);
    for(int i=1;i<functions.size();i++)
    {
        printf(",\n");
        generate(functions[i].second, functions[i].first);
    }
    printf("\n};\n\n#endif");
}