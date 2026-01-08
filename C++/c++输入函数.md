c++输入函数

1.`getline`

返回值类型：istream&输入流对象的引用，传入cin就返回cin,输入正常时返回cin是有效状态，while条件为true

专门用于**读取一行字符**（从输入流中读取，直到遇到换行符 `\n` 为止），并且会自动舍弃末尾的 `\n`，能完美读取包含空格、制表符的整行内容（这是 `cin >>` 做不到的）

```c++
getline(输入流对象, 字符串对象, 终止符);
// 终止符可选，默认是 '\n'（换行符）
```

2.`cin>>`

返回值类型：istream&

读取终止符：空格、制表符、换行符

终止符留在输入缓冲区

注意：用cin>>后再用getline，导致\n留在缓冲区被getline读到，可使用如下方法：

```c++
cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
 // numeric_limits 需要包含 <limits> 头文件
```

用 cin.ignore() 忽略缓冲区的换行符（推荐）

3.读取单个字符：cin.get()、cin.getchar()(需包含头文件<cstdio>)

返回值类型：int,字符的ascii码

4.`cin.read()`（读取二进制 / 指定长度数据）

返回值类型：istream&

**作用**：从输入流中读取**指定字节数**的原始数据（二进制 / 文本均可），不跳过任何字符；

**语法**：`cin.read(字符数组/缓冲区, 读取字节数)`

5.文件输入流法：

需包含 `<fstream>`，创建 `ifstream` 对象关联文件，再读取

```c++
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main() {
    // 创建文件输入流，关联 "test.txt"
    ifstream fin("test.txt");
    if (!fin) { // 检查文件是否打开成功
        cout << "文件打开失败！" << endl;
        return 1;
    }
    
    string line;
    // 逐行读取文件内容
    while (getline(fin, line)) {
        cout << "文件行内容：" << line << endl;
    }
    
    fin.close(); // 关闭文件
    return 0;
}

```

6.字符串输入流

```c++
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

int main() {
    string str = "100 3.14 hello";
    istringstream iss(str); // 关联字符串
    
    int a;
    float b;
    string s;
    // 按格式读取字符串中的数据
    iss >> a >> b >> s;
    
    cout << "整数：" << a << endl;
    cout << "浮点数：" << b << endl;
    cout << "字符串：" << s << endl;
    return 0;
}
```

7.跳过指定字节

cin.ignore():istream&

```c++
 cout << "输入：abc123" << endl;
    // 跳过前3个字符，直到遇到数字
    cin.ignore(3, '1'); 
```

8.预览下一个字符

cin.peek():int

```c++
  int ch = cin.peek(); 
```

