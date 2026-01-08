`string`类常用成员

```c++
#include <iostream>
#include <string>
using namespace std;
int main(){
    string s="hello world";
    //1.基础属性
    cout<<s.size()<<endl;//长度，不加\0
    cout<<s.empty()<<endl;//判断是否为空
    //2.查找与替换
    size_t pos=s.find("world");//查找子串，返回索引6,找不到返回string::npos;
    if(pos!=string::npos){
        s.replace(pos,5,"python");
        cout<<s<<endl;
    }
    //3.截取子串
    string sub=s.substr(0,5);
    cout<<sub<<endl;
    //4.拼接与追加
    s+="!";
    s.append("  nice");
    cout<<s<<endl;
    //5.插入与删除
    s.insert(5,",");
    s.erase(5,1);
    cout<<s<<endl;
    //6.清空与赋值
    string s2;
    s2.assign(s,0,5);
    s.clear();
    cout<<s2<<endl;
    return 0;
}
```

