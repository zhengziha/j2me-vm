输出日志使用Logger.hpp中的LOG_INFO、LOG_ERROR、LOG_WARN、LOG_DEBUG
日志级别    LOG_INFO >= LOG_ERROR >= LOG_WARN >= LOG_DEBUG
修改stubs中的java代码后，需要重新编译stubs中的代码使用./build_rt_jar.sh脚本
项目编译使用./build.sh脚本
代码实现时应该减少平台依赖，尽量使用java层实现，必要时才使用native方法，参考java标准库的实现