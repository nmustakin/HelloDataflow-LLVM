# HelloDataflow-LLVM
Getting started to write Dataflow analysis in LLVM. (Without standard templates)

## Code Guidelines for LLVM APIs
1. [See basic steps to access functions, basic blocks and instructions from LLVM API](https://github.com/ufarooq/HelloPass-LLVM/). The following LLVM APIs might be helpful for implementing Dataflow Analysis. Assuming that you have a basic block and want to find out its successor(s), use the following snippet to iterate over its successors.
```cpp
//#include "llvm/IR/CFG.h"

for (BasicBlock *Succ : successors(&basic_block)) {
	...
}
```
2. Next, use following API to find predecessor(s).
```cpp
//#include "llvm/IR/CFG.h"

for (BasicBlock *Pred : predecessors(&basic_block)) {
	...
}
```
3. Use standard C++ APIs to perform basic set operations such as union of two sets or vectors. 
```cpp
// #include <set>
std::vector<int> v1 = {1, 2, 3, 4, 5}; 
std::vector<int> v2 = {3, 4, 5, 6, 7}; 
std::vector<int> dest1; // create a third vector to write results

std::set_union(v1.begin(), v1.end(), v2.begin(), v2.end(),std::back_inserter(dest1)); // writes Union (v1+v2) to dest1
```
4. Similarly, intersection and difference operations can be performed. 
```cpp
std::set_difference(v1.begin(), v1.end(), v2.begin(), v2.end(),std::back_inserter(dest1)); // writes difference (v1-v2) to dest1

std::set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(),std::back_inserter(dest1)); // writes Common elements of both sets to dest1

``` 
