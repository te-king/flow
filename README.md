# Flow
Tiny lazy data manipulation for STL containers


Containers can be interpreted as a flow of data. This data can then be manipulated as it is collected.


Depends heavily on the type erasure in std::function so its not particularly fast. Very convenient, however.


# Examples

1. Printing Even Numbers:
```C++
int main() {

    std::vector vec = { 0, 1, 2, 3, 4, 5 };
  
    from(vec)
        .filter([] (auto &&item) {
            return item % 2 == 0; // check if even
        })
        .for_each([] (auto &&item) {
            std::cout << item << std::endl; // print to cout
        });

}
```

```
0
2
4
```

2. Capturing Results:
```C++
int main() {

    std::array arr = { 5, 10, 15, 20, 25, 30 };
    std::vector vec = { 0, 3, 1, 4, 2, 5 };
    
    auto v2 = from(vec)
        .map<int>([&] (auto &&item) {
            return arr[item];
        })
        .to_vector();
        
    std::cout << from(v2).join_to_string();
    
}
```

```
5, 20, 10, 25, 15, 30
```
