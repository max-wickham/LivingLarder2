#include <iostream>
#include <string>
using namespace std;

struct WaterSettings { 
    unsigned int trayWaterTimes[4];
    unsigned int trayWaterUnits;
};


void print(int x[3]){
    for(int i = 0; i < 3; i++){
        cout << x[i] << endl;
    }
}

int main(){
    int x[5] = {0,1,2,3,4};
    print(x + 2);
    cout << sizeof(WaterSettings);
}