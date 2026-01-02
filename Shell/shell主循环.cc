#include <iostream>
#include <string>
using namespace std;
void print() {
    cout << "my_shell";
    cout.flush();
}
int main(){
    while(true){
        string line;
        if(!getline(cin,line)){
            cout << endl;
            break;
        }
        if(line=="exit"){
            cout << "Bye Bye" << endl;
            break;
        }
    }
    return 0;
}