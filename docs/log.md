# log 模块代码流程文档

## 文档范围

本文分析 `src/log.h` 与 `src/log.cpp` 中实现的日志模块，重点梳理日志对象、日志级别、格式化器、输出器以及一次日志输出从创建到落地的代码流程。测试入口可参考 `tests/test.cpp`。

## 模块整体职责

`log` 模块位于 `sylar` 命名空间下，当前实现了一个基础日志系统。它的核心思路是：调用方先创建 `LogEvent` 保存本次日志的上下文和正文，再通过 `Logger` 按日志级别进行过滤，最后把日志事件分发给一个或多个 `LogAppender`。每个 `LogAppender` 使用 `LogFormatter` 把事件渲染成字符串，并输出到对应目的地，例如标准输出或文件流。

整体流程可以概括为：构造日志事件，写入日志内容，调用日志器输出，日志器按级别过滤，遍历输出器，输出器再次按级别过滤，格式化器按 pattern 生成字符串，最终写入控制台或文件。

## 主要类与职责

### `LogEvent`

`LogEvent` 表示一次日志事件，声明在 `src/log.h`。它保存日志产生时的上下文信息，包括源文件名 `file_`、行号 `line_`、程序启动后的耗时 `elapse_`、线程 ID `threadId_`、协程 ID `fiberId_`、时间戳 `time_`，以及用于拼接日志正文的 `std::stringstream ss_`。

调用方通过 `getSS()` 获取内部字符串流并写入日志正文，例如测试代码中的 `event->getSS() << "hello sylar";`。当格式化器需要输出正文时，会调用 `getContent()`，该函数返回 `ss_.str()`。

### `LogLevel`

`LogLevel` 定义日志级别枚举：`UNKNOWN = 0`、`DEBUG = 1`、`INFO = 2`、`WARN = 3`、`ERROR = 4`、`FATAL = 5`。级别值越大，表示日志越严重。

`LogLevel::ToString()` 在 `src/log.cpp` 中实现，用于把枚举值转换成字符串。当前通过宏 `XX(name)` 简化 `switch` 分支，已覆盖 `DEBUG`、`INFO`、`WARN`、`ERROR`、`FATAL`，其他值返回 `UNKNOWN`。

### `Logger`

`Logger` 是日志器，负责管理日志级别、默认格式化器和输出器列表。它继承 `std::enable_shared_from_this<Logger>`，因此在 `Logger::log()` 中可以通过 `shared_from_this()` 获取当前日志器的 `shared_ptr` 并传递给输出器。

构造函数 `Logger::Logger(const std::string& name)` 会设置日志器名称，默认日志级别为 `DEBUG`，并创建默认格式化器：`%d [%p] <%f:%l> %m %n`。该 pattern 表示输出时间、日志级别、文件名、行号、日志正文和换行。

`addAppender()` 用于添加输出器。如果输出器本身没有设置格式化器，则会继承日志器的默认格式化器。`delAppender()` 用于从输出器链表中删除指定输出器。

`log()` 是核心分发函数。它先判断本次日志级别是否大于等于日志器自身级别，只有满足条件才继续输出。随后它遍历 `appenders_`，依次调用每个输出器的 `log(self, level, event)`。`debug()`、`info()`、`warn()`、`error()`、`fatal()` 都是对 `log()` 的便捷封装。

### `LogAppender`

`LogAppender` 是日志输出地的抽象基类。它声明了纯虚函数 `log()`，并保存输出器自身的日志级别 `level_` 和格式化器 `format_`。

输出器级别与日志器级别是两层过滤。日志事件必须先通过 `Logger::log()` 的级别判断，再通过具体输出器 `log()` 中的级别判断，才会真正输出。这使得同一个日志器可以挂载多个输出器，并为不同输出目的地设置不同过滤级别。

### `StdoutLogAppender`

`StdoutLogAppender` 输出日志到标准输出。它的 `log()` 函数会判断 `level >= level_`，满足后调用 `format_->format(logger, level, event)` 得到格式化字符串，并写入 `std::cout`。

### `FileLogAppender`

`FileLogAppender` 输出日志到文件。构造函数保存目标文件名 `file_name_`。`log()` 与标准输出类似，先按输出器级别过滤，然后将格式化结果写入 `file_stream_`。

`reopen()` 用于打开或重新打开文件流。如果当前 `file_stream_` 已经打开，则先关闭，再用 `file_name_` 打开文件，最后返回文件流是否成功打开。当前构造函数只保存文件名，并不会自动调用 `reopen()`，因此使用 `FileLogAppender` 时需要先显式调用 `reopen()`，否则 `file_stream_` 可能没有打开，写入不会产生有效文件内容。

### `LogFormatter` 与 `FormatItem`

`LogFormatter` 负责把日志事件格式化成字符串。它持有原始 pattern 字符串 `pattern_`，以及解析 pattern 后得到的 `items_`。每个 item 都是一个 `FormatItem`，代表 pattern 中的一段固定文本或一个格式化占位符。

`LogFormatter` 构造时调用 `init()` 解析 pattern。`format()` 创建 `std::stringstream`，按顺序遍历 `items_`，让每个 `FormatItem` 把自己的内容写入流，最后返回完整字符串。

`FormatItem` 是抽象基类，定义统一接口 `format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)`。不同派生类负责输出不同字段。

当前实现的格式项包括：`%m` 输出日志正文，`%p` 输出日志级别，`%r` 输出日志器名称，`%t` 输出线程 ID，`%n` 输出换行，`%d` 输出时间，`%f` 输出文件名，`%l` 输出行号。代码中还实现了 `ElapseFromatItem` 和 `FiberFromatItem`，分别用于输出耗时和协程 ID，但它们暂未注册到 `s_format_item` 映射表，因此当前 pattern 无法通过占位符使用它们。

## 一次日志输出的代码流程

以 `tests/test.cpp` 为例，调用方首先创建日志器：`sylar::Logger::ptr log(new sylar::Logger);`。构造函数会把日志器名称设为默认值 `root`，级别设为 `DEBUG`，并准备默认格式化器。

然后调用方创建标准输出器并添加到日志器：`log->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));`。由于这个输出器尚未设置自己的格式化器，`Logger::addAppender()` 会把日志器默认格式化器设置到该输出器上。

接着调用方创建日志事件：`new sylar::LogEvent(__FILE__, __LINE__, 0, 1, 2, time(0))`。这里记录了当前文件、当前行号、耗时、线程 ID、协程 ID 和当前时间。随后通过 `event->getSS() << "hello sylar";` 写入日志正文。

当调用 `log->log(sylar::LogLevel::DEBUG, event);` 时，`Logger::log()` 首先比较本次日志级别 `DEBUG` 和日志器级别 `DEBUG`。由于二者相等，日志事件通过第一层过滤。然后日志器通过 `shared_from_this()` 获取自身指针，并遍历输出器列表。当前只有一个 `StdoutLogAppender`，因此会调用它的 `log()`。

`StdoutLogAppender::log()` 再次比较本次日志级别和输出器自身级别。默认输出器级别也是 `DEBUG`，因此事件通过第二层过滤。随后它调用格式化器生成字符串。

默认 pattern 为 `%d [%p] <%f:%l> %m %n`。格式化器会依次执行时间项、固定字符串 ` [`、级别项、固定字符串 `] <`、文件名项、固定字符串 `:`、行号项、固定字符串 `> `、正文项、固定字符串空格、换行项。最终输出效果类似：`2026-06-03 12:00:00 [DEBUG] <tests/test.cpp:8> hello sylar`，末尾带换行。

## pattern 解析流程

`LogFormatter::init()` 负责把 pattern 拆分为可执行的格式项。它先创建一个 `vec`，其中每个元素是三元组 `(str, format, type)`。`type = 0` 表示普通字符串，`type = 1` 表示格式占位符。

解析时，代码逐字符扫描 `pattern_`。如果当前字符不是 `%`，就追加到普通字符串缓冲区 `nstr`。如果遇到 `%`，则进入格式占位符解析。普通字符串在遇到占位符前会被压入 `vec`，然后占位符自身也会被压入 `vec`。

解析完成后，代码通过静态映射表 `s_format_item` 把占位符转换成对应的 `FormatItem` 对象。如果某个占位符没有注册，则会生成固定字符串 `<<error_format %x>>`。如果是普通字符串，则生成 `StringFromatItem`。

因此，格式化阶段不再重新解析 pattern，而是直接顺序执行 `items_` 中的对象。这种设计把 pattern 解析和日志渲染分离，避免每次输出日志时重复解析格式字符串。

## 当前代码中的注意点

`LogEvent::time_` 没有在类内初始化。由于主要路径使用带参构造函数传入时间戳，当前测试路径是安全的；但如果未来补上默认构造函数，需要同时初始化 `time_`。

`FileLogAppender` 构造函数不会自动打开文件，需要调用 `reopen()` 后才能确保文件流可写。如果期望创建文件输出器后立即可用，可以考虑在构造函数中调用 `reopen()`，或者在文档/调用规范中明确要求使用者手动调用。

`%%` 的处理当前只是向普通字符串追加 `%` 后继续循环，但没有跳过第二个 `%`。这可能导致第二个 `%` 再次被当作占位符起点处理。后续可以在处理 `%%` 时让索引前进一位，避免重复解析。

`ElapseFromatItem` 和 `FiberFromatItem` 已经实现，但没有注册进 `s_format_item`。如果希望支持耗时和协程 ID，需要为它们分配占位符并添加到映射表。例如可以用 `%e` 表示 `elapse_`，用 `%F` 表示 `fiberId_`。

`LogFormatter::init()` 在构造格式化器时会把解析结果打印到 `std::cout`，用于调试 pattern 解析。但这会导致程序启动或创建格式化器时产生额外控制台输出。生产环境下建议移除或改为受调试开关控制。

当前日志模块没有加锁保护。`Logger::appenders_`、`LogAppender::format_`、`FileLogAppender::file_stream_` 等共享状态在多线程下可能出现竞争。如果项目后续进入多线程/协程调度阶段，需要为日志器和输出器增加互斥保护，或设计无锁/队列式日志方案。
