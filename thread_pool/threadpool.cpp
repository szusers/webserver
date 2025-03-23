#include"./threadpool.h"

/*
模板类在编译阶段发生linking链接报错的解决方法：（我这里最后是使用第一种方法解决的）


1. 确保模板实现被包含在头文件中
模板类的实现应该放在头文件中，因为模板类的实例化通常发生在编译时。因此，确保 thread_pool.h 文件中包含了所有的模板函数实现，而不仅仅是声明。

检查 thread_pool.h 和 threadpool.cpp，确保所有的模板函数定义都放在了头文件 thread_pool.h 中，而不是放在 .cpp 文件中。因为 .cpp 文件中的模板定义可能不会被正确实例化。

2. 确保编译器能够找到 thread_pool 相关的源文件
确认在 CMakeLists.txt 中有正确地将 thread_pool.cpp 加入到编译过程中。即使模板类的实现通常放在头文件中，但如果有一些非模板的代码，还是需要确保 thread_pool.cpp 被编译。

在 CMakeLists.txt 文件中，确保 thread_pool.cpp 被添加到编译目标中。例如：

*/



