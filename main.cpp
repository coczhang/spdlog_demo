#include <iostream>
#include <memory>

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/stopwatch.h"
#include "spdlog/sinks/callback_sink.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>

/*
日志等级从上向下依次递增
enum level_enum : int {
	trace = SPDLOG_LEVEL_TRACE,
	debug = SPDLOG_LEVEL_DEBUG,
	info = SPDLOG_LEVEL_INFO,
	warn = SPDLOG_LEVEL_WARN,
	err = SPDLOG_LEVEL_ERROR,
	critical = SPDLOG_LEVEL_CRITICAL,
	off = SPDLOG_LEVEL_OFF,
	n_levels
};
*/

/* 创建stdout/stderr记录器对象 */
void stdout_example() {
	auto console = spdlog::stderr_color_mt("console");
	auto err_loger = spdlog::stderr_color_mt("stderr");
	spdlog::get("console")->info("loggers can be retrieved from a global registry using the spdlog::get(logger_name)");
	spdlog::get("stderr")->info("loggers can be retrieved from a global registry using the spdlog::get(logger_name)");
}

/* 基本文件记录器 */
void basic_lofile_example() {
	try {
		auto logger = spdlog::basic_logger_mt("basic_logger", "logs/basic_logger.log");
		logger->info("basic_logger");

	} catch (const spdlog::spdlog_ex& ex) {
		std::cout << "Log init failed:" << ex.what() << std::endl;
	}
}

/* 设置文件大小 */
void rotaing_example() {
	auto max_size = 1048576 * 5;
	// Create a file rotating logger with 5mb size max and 3 rotated files
	auto max_files = 3;
	auto logger = spdlog::rotating_logger_mt("some_logger", "logs/rotating.log", max_size, max_files);
}

/* 设置文件生成频率，按日生成 */
void daily_example() {
	// Create a daily logger - a new file is created every day on 2:30am
	auto logger = spdlog::daily_logger_mt("daily_logger", "logs/daily.log", 2, 30);

	/* 实现了每3秒进行一次日志输出，但同时也要注意，使用时确保日志对象是线程安全的。 */
	spdlog::flush_every(std::chrono::seconds(3));
}

void multi_sink_example() {
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	console_sink->set_level(spdlog::level::warn);
	console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");

	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/multisink.log", true);
	file_sink->set_level(spdlog::level::trace);

	spdlog::logger logger("multi_sink", { console_sink, file_sink });
	logger.set_level(spdlog::level::debug);
	logger.warn("this should appear in both console and file");
	logger.info("this message should not appear in the console, only in the file");
}

void init_spdlog() {
	/* 
		异步日志，具有8K个项目和1个后台线程的队列 
		8192:这个参数指定了线程池的大小，也就是线程池中最大可以同时运行的线程数量。在这个例子中，线程池的大小被设置为 8192。
		1:这个参数指定了线程池的线程数量。在这个例子中，线程池中只有一个线程。
	*/
	spdlog::init_thread_pool(8192, 1);
	/* 标准控制台输出 */
	auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	stdout_sink->set_level(spdlog::level::debug);
	/* 日志文件输出，0点0分创建新日志 */
	auto file_sink = std::make_shared<spdlog::sinks::daily_file_format_sink_mt>("logs/log.txt", 0, 0);
	file_sink->set_level(spdlog::level::info);
	auto callback_sink = std::make_shared<spdlog::sinks::callback_sink_mt>([](const spdlog::details::log_msg& msg) {
		/* 日志记录器名称 */
		std::string name(msg.logger_name.data(), 0, msg.logger_name.size());
		/* 日志消息 */
		std::string str(msg.payload.data(), 0, msg.payload.size());
		/* 日志时间 */
		std::time_t now_c = std::chrono::system_clock::to_time_t(msg.time);

		//回调的处理逻辑自己根据项目情况定义，比如实时显示到UI、保存到数据库等等

		//.... 回调处理逻辑的示例
	    //std::tm localTime;
	    //localtime_s(&localTime, &now_c);
		//char timeStr[50];
		//std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &localTime);
		//// 获取毫秒数
		//auto duration = msg.time.time_since_epoch();
		//auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;
		//std::cout << timeStr << "." << std::setfill('0') << std::setw(3) << milliseconds << " " ;
		//std::cout << to_string_view(msg.level).data() << " " << str << std::endl << std::endl << std::flush;
	});
	callback_sink->set_level(spdlog::level::info);

	/* 
		其中包含了三个不同类型的日志输出目标：stdout_sink、file_sink 和 callback_sink。
		这些目标用于指定日志消息的输出方式，例如输出到控制台、输出到文件或通过回调函数处理。 
	*/
	std::vector<spdlog::sink_ptr> sinks{ stdout_sink, file_sink,callback_sink };
	
	/*
		这一行创建了一个名为 log 的异步日志记录器。具体来说：
		"logger" 是日志记录器的名称。
		sinks.begin() 和 sinks.end() 表示初始化日志记录器时使用的输出目标范围，即上一行创建的 sinks 容器中的目标。
		spdlog::thread_pool() 创建了一个用于异步日志记录的线程池。
		spdlog::async_overflow_policy::block 表示当日志消息超出处理能力时的溢出策略，此处是阻塞模式，即在无法处理更多日志消息时阻塞新的日志消息。
	*/
	auto log = std::make_shared<spdlog::async_logger>("logger", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);

	//设置日志记录级别，您需要用 %^ 和 %$  括上想要彩色的部分
	log->set_level(spdlog::level::trace);
	//设置格式
	//参考 https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
	//[%Y-%m-%d %H:%M:%S.%e] 时间
	//[%l] 日志级别
	//[%t] 线程
	//[%s] 文件
	//[%#] 行号
	//[%!] 函数
	//[%v] 实际文本
	log->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %^[%l]%$ [%t] [%s %!:%#] %v");
	log->info("stdout_sink, file_sink,callback_sink");
	//设置当出发 err 或更严重的错误时立刻刷新日志到  disk
	log->flush_on(spdlog::level::err);
	//3秒刷新一次队列
	spdlog::flush_every(std::chrono::seconds(3));
	spdlog::set_default_logger(log);
}

//单个日志记录器
std::shared_ptr<spdlog::logger> get_async_file_logger(std::string name) {
	auto log = spdlog::get(name);
	if (!log) {
		//指针为空，则创建日志记录器，
		log = spdlog::daily_logger_mt<spdlog::async_factory>(name, "logs/" + name + "/log.txt");
		log->set_level(spdlog::level::trace);
		log->flush_on(spdlog::level::err);
		log->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %^[%l]%$ [%t] [%s %!:%#] %v");
		//记录器是自动注册的，不需要手动注册  spdlog::register_logger(name);
	}
	return log;
}

#define INITLOG()     init_spdlog()


#define TRACE(...)     SPDLOG_TRACE(__VA_ARGS__)
#define DEBUG(...)     SPDLOG_DEBUG(__VA_ARGS__)
#define INFO(...)      SPDLOG_INFO(__VA_ARGS__)
#define WARN(...)      SPDLOG_WARN(__VA_ARGS__)
#define ERR(...)     SPDLOG_ERROR(__VA_ARGS__)
#define CRITICAL(...)  SPDLOG_CRITICAL(__VA_ARGS__)

//单个日志文件
#define GETLOG(LOG_NAME) get_async_file_logger(LOG_NAME)

#define LOGGER_TRACE(logger,...)     SPDLOG_LOGGER_TRACE(logger,__VA_ARGS__)
#define LOGGER_DEBUG(logger,...)     SPDLOG_LOGGER_DEBUG(logger,__VA_ARGS__)
#define LOGGER_INFO(logger,...)      SPDLOG_LOGGER_INFO(logger,__VA_ARGS__)
#define LOGGER_WARN(logger,...)      SPDLOG_LOGGER_WARN(logger,__VA_ARGS__)
#define LOGGER_ERROR(logger,...)     SPDLOG_LOGGER_ERROR(logger,__VA_ARGS__)
#define LOGGER_CRITICAL(logger,...)  SPDLOG_LOGGER_CRITICAL(logger,__VA_ARGS__)

//时间统计宏
#define LOGSW() spdlog::stopwatch()

int main() {
#if 0
	/* 
		设置日志等级，当日志等级低于当前的日志等级，
		那么，低等级日志将无法打印
	*/
	spdlog::set_level(spdlog::level::trace);
	/* 
		Spdlog 支持的格式参数包括：
		%v —— 日志内容
		%n —— 换行
		%d —— 日期和时间
		%r —— 12 小时制时间
		%R —— 24 小时制时间
		%T —— 24 小时制时间（秒精度）
		%H —— 小时（00-23）
		%M —— 分钟（00-59）
		%S —— 秒（00-59）
		%e —— 毫秒（三位数字）
		%f —— 微秒（六位数字）
		%F —— 纳秒（九位数字）
		%z —— 时区偏移量
		%s —— 时间戳
		%L —— 日志名称
		%t —— 线程 ID
		%P —— 进程 ID
		%^ —— 转换为大写字母
		%$ —— 转换为彩色输出

		例如，可以使用以下代码将日志级别转换为大写字母：
		spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
		在上面的代码中，"[%^%l%$]" 表示将日志级别转换为大写字母。
	*/
	spdlog::set_pattern("[%H:%M:%S %z] [%^%L%$] [thread %t] %v");
	/* 将日志信息打印到控制台 */
	spdlog::trace("hello spdlog trace");
	spdlog::debug("hello spdlog debug");
	/* 以{}为占位符 */
	spdlog::info("hello spdlog {}", "info");
	spdlog::warn("hello spdlog warn");
	spdlog::error("hello spdlog error");
	spdlog::critical("hello spdlog critical");

	/* 设置日志头信息的类型为默认类型 */
	spdlog::set_pattern("%+");  // back to default format
	spdlog::info("hello world");

	/* 将日志信息写入到本地文件中 */
	auto logger = spdlog::basic_logger_mt("file_logger", "basic-log.log");
	logger->set_level(spdlog::level::trace);
	/*
	   [%Y-%m-%d %H:%M:%S.%e] 时间
	   [%l] 日志级别
	   [%s] 文件
	   [%#] 行号
	   [%!] 函数
	   [%v] 实际文本
	*/
	logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %^[%l]%$ [%t] [%s %!:%#] %v");
	logger->info("hello,{}", "world");
	logger->info("Welcome to spdlog version {}.{}.{}  !", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR,
		SPDLOG_VER_PATCH);

	logger->warn("Easy padding in numbers like {:08d}", 12);
	logger->critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
	logger->info("Support for floats {:03.2f}", 1.23456);
	logger->info("Positional args are {1} {0}..", "too", "supported");
	logger->info("{:>8} aligned, {:<8} aligned", "right", "left");
	logger->error("error info");
	logger->flush_on(spdlog::level::err);
	/*
		实时的将日志信息写入到文件中，避免程序未退出时，日志信息无法更新到文件中
		50毫秒刷新一次队列
	*/
	spdlog::flush_every(std::chrono::milliseconds(50));

	/*
		spdlog::sinks::stdout_color_sink_mt 是 spdlog 库提供的一个输出目标（sink），
		它能够将日志消息发送到标准输出（stdout），并且为不同的日志级别（如错误、警告、信息）添加颜色标记。
	*/
	// 创建指向 stdout_color_sink_mt 对象的 shared 指针
	auto sink1 = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	// 现在你可以使用 'sink1' 来配置你的日志记录器，例如：
	auto sink1_logger = std::make_shared<spdlog::logger>("console", sink1);
	spdlog::register_logger(sink1_logger);
	// 记录一些日志消息
	sink1_logger->info("sink1:这是一个信息消息");
	sink1_logger->error("sink1:这是一个错误消息");

	/*
		spdlog::sinks::daily_file_sink_mt 是一个基于 spdlog 库的输出目标，它可以将日志消息写入到文件中，
		并且能够根据日期进行日志文件的滚动。每天会生成一个新的日志文件，以确保日志文件的大小不会无限增长，从而更易于管理和维护。
	*/
	// 创建指向 daily_file_sink_mt 对象的 shared 指针
	auto sink2 = std::make_shared<spdlog::sinks::daily_file_sink_mt>("sink2.log", 23, 00);//每天的23点00分创建新的文件
	// 现在你可以使用 'sink2' 来配置你的日志记录器，例如：
	auto sink2_logger = std::make_shared<spdlog::logger>("daily_logger", sink2);
	spdlog::register_logger(sink2_logger);
	// 记录一些日志消息
	sink2_logger->info("sink2:这是一个信息消息");
	sink2_logger->error("sink2:一个错误消息");

	/* spdlog::sinks::rotating_file_sink_mt 是 spdlog 库中的一个日志输出目标（sink）
	   ，用于将日志写入到文件中。它支持日志文件的自动轮转，当日志文件达到指定大小时，会自动创建一个新的日志文件，
	   并将日志写入到新文件中。 
	*/
	// 创建指向 rotating_file_sink_mt 对象的 shared_ptr
	auto sink3 = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("sink3.log", 1024 * 1024 * 5, 3); // 5MB 文件大小，最多保留 3 个文件
	// 现在你可以使用 'sink3' 来配置你的日志记录器，例如：
	auto sink3_logger = std::make_shared<spdlog::logger>("rotating_logger", sink3);
	spdlog::register_logger(sink3_logger);
	// 记录一些日志消息
	sink3_logger->info("sink3:这是一个信息消息");
	sink3_logger->error("sink3:这是一个错误消息");

	// 日志文件名
	std::string log_file_name = "example.log";
	// 创建指向 basic_file_sink_mt 对象的 shared_ptr
	auto sink4 = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_name);
	// 现在你可以使用 'sink4' 来配置你的日志记录器，例如：
	auto sink4_logger = std::make_shared<spdlog::logger>("example_logger", sink4);
	spdlog::register_logger(sink4_logger);
	// 记录一些日志消息
	sink4_logger->info("sink4_logger:这是一个信息消息");
	sink4_logger->error("sink4_logger:这是一个错误消息");

	// 当日志记录器被销毁或者调用 spdlog::shutdown() 时，spdlog 会自动刷新日志

	/*
		spdlog::sinks::stdout_color_sink_mt 是一个输出目标（sink），它将日志消息输出到标准输出（stdout），
		通常用于控制台输出，并且支持在输出中为不同的日志级别添加颜色标记。
		如果你想将日志同时输出到文件中，可以使用 spdlog::sinks::stdout_color_sink_mt 和 spdlog::sinks::basic_file_sink_mt 结合起来。
	*/
	 // 创建指向 stdout_color_sink_mt 和 basic_file_sink_mt 对象的 shared 指针
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("console_sink.log", true); // true 表示追加到文件末尾
	// 创建一个多重 sink，同时输出到控制台和文件
	auto combined_sink = std::make_shared<spdlog::sinks::dist_sink_mt>();
	combined_sink->add_sink(console_sink);
	combined_sink->add_sink(file_sink);
	// 现在你可以使用 'combined_sink' 来配置你的日志记录器，例如：
	auto combined_logger = std::make_shared<spdlog::logger>("combined_logger", combined_sink);
	spdlog::register_logger(combined_logger);
	// 记录一些日志消息
	combined_logger->info("combined_logger:这是一个信息消息");
	combined_logger->error("combined_logger:这是一个错误消息");

	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	init_spdlog();
	stdout_example();
	basic_lofile_example();

	/* 调试日志可以暂时保存在内存中，需要的时候可以通过接口对缓存的日志进行输出，
	   通过参数可以设置日志缓存和输出的日志记录数。
	   下面的代码设置保存最近的32条日志消息。
	*/
	spdlog::enable_backtrace(32);
	for (int i = 0; i < 100; i++) {
		spdlog::debug("Backtrace message {}", i);
	}
	// e.g. if some error happened:
	spdlog::dump_backtrace(); // log them now! show the last 32 messages

	multi_sink_example();
#endif


	return 1;
}

void spd_log_example() {
	/* 使用样例 */
	spdlog::info("Welcome to spdlog!");
	spdlog::error("Some error message with arg: {}", 1);

	spdlog::warn("Easy padding in numbers like {:08d}", 12);
	spdlog::critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
	spdlog::info("Support for floats {:03.2f}", 1.23456);
	spdlog::info("Positional args are {1} {0}..", "too", "supported");
	spdlog::info("{:<30}", "left aligned");

	spdlog::set_level(spdlog::level::debug); // Set global log level to debug
	spdlog::debug("This message should be displayed..");

	// change log pattern
	spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");

	// Compile time log levels
	// define SPDLOG_ACTIVE_LEVEL to desired level
	SPDLOG_TRACE("Some trace message with param {}", 42);
	SPDLOG_DEBUG("Some debug message");

	/* 
	核心概念：
	1.logger：日志对象，每个日志内包含一个sink组成的vector，每个sink可以分别设置优先级，logger本身也可设置优先级
	2.sink：直译是水槽，实际上是引流的对象或者可以认为是输出目标，spdlog库内置了多种不同类型的logger可供选择
	3.formatter：格式化对象，绝大部分情况下spdlog默认的格式就足够用了，但是如果有个性化需求，可以进行自定义格式
	4.level：日志级别，不同的日志库可能会有不同的设置，但是基本情况下都会有debug、info、warn、error等的级别划分来处理不同的情况，具体各个级别的情况可以根据自己的实际情况选取
	
	逻辑关系：每个logger包含一个vector，该vector由一个或多个std::shared_ptr<sink>组成，logger的每条日志都会调用sink对象，
	由sink对象按照formatter的格式输出到sink指定的地方（有可能是控制台、文件等），接下来我们从内到外的讲解spdlog的这三个核心组件
	
	formatter:
	formatter也即格式化对象，用于控制日志的输出格式,spdlog自带了默认的formatter，一般情况下，我们无需任何修改，直接使用即可。注意，每个sink会有一个formatter
	
	默认formatter的格式为：[日期时间] [logger名] [log级别] log内容
	[2024-04-13 17:00:55.795] [spd_logger] [debug] found env XXXXXXX : true
	[2024-04-13 17:00:55.795] [func_config] [debug] kafka_brokers : localhost:9092
	[2024-04-13 17:00:55.795] [func_config] [debug] kafka_main_topic : kafka_test
	[2024-04-13 17:00:55.795] [func_config] [debug] kafka_partition_value : -1
	[2024-04-13 17:00:55.795] [spd_logger] [info] initialized
	
	自定义formatter:
	如果默认的formatter不符合需求，可以自定义formatter，具体方式如下:
	set_parrtern(pattern_string);
	例如：
	全局级别的：spdlog::set_pattern(" [%H:%M:%S %z] [thread %t] %v ");
	单个logger级别的：some_logger->set_parttern(">>> %H:%M:%S %z %v <<<");
	单个sink级别的：some_sink-> set_parttern(".. %H: %M ..");
	其中用到了%H %M这些占位符，事实上它们都是预先设定好的，想要查看所有的占位符情况，可以参考以下网站：
	https://spdlog.docsforge.com/v1.x/3.custom-formatting/#pattern-flags

	sink:
	每个sink对应着一个输出目标和输出格式，它内部包含一个formatter，输出目标可以是控制台、文件等地方。
	所有的sink都在命名空间spdlog::sinks下，可以自行探索

	控制台sink:
	spdlog中创建控制台sink非常简单，该方式创建的sink会输出到命令行终端，且是彩色的（也可以选非彩色的，但是有彩色的应该都会选彩色的吧……）。后缀的_mt代表多线程，_st代表单线程
	auto sink1 = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

	文件sink:
	//最简单的文件sink，只需要指定文件名
	auto sink1 = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_name);

	//每天的12点00分在path下创建新的文件
	auto sink2 = std::make_shared<spdlog::sinks::daily_file_sink_mt>(log_file_name, path, 12, 00);

	//轮转文件，一个文件满了会写到下一个文件，第二个参数是单文件大小上限，第三个参数是文件数量最大值
	auto sink3 = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file_name, 1024 * 1024 * 10, 100, false);

	其他sink:
	ostream_sink
	syslog_sink
	......
	也可以通过继承base_sink创建子类来自定义sink，具体可以参考：
	https://spdlog.docsforge.com/v1.x/4.sinks/#implementing-your-own-sink

	sink的flush问题:
	创建好sink后建议设置flush方式，否则可能无法立刻在file中看到logger的内容
	以下为两种重要的flush方式设置（直接设置全局）
	spdlog::flush_every(std::chrono::seconds(1));
	spdlog::flush_on(spdlog::level::debug);

	logger:
	日志对象，每个logger内包含了一个vector用于存放sink，每个sink都是相互独立
	因此一个日志对象在输出日志时可以同时输出到控制台和文件等位置

	使用默认logger:
	如果整个项目中只需要一个logger，spdlog提供了最为便捷的默认logger，注意，该logger在全局公用，输出到控制台、多线程、彩色
	//Use the default logger (stdout, multi-threaded, colored)
	spdlog::info("Hello, {}!", "World");

	创建特定的logger:
	大部分情况下默认logger是不够用的，因为我们可能需要做不同项目模块各自的logger，可能需要logger输出到文件进行持久化，所以创建logger是很重要的一件事。好在创建logger也是非常简单的！
	
	注意：一个logger内假如有多个sink，那么这些sink分别设置level是可以不同的，但是由于logger本身也有level，所以真正使用时，logger的level如果高于某个sink，
	会覆盖该sink的level，所以建议此时把logger的level手动设置为debug（默认为info）
	*/

	/*
	方式一：直接创建
	与创建sink类似，我们可以非常便捷的创建logger
	由于大部分时候一个logger只会有一个sink，所以spdlog提供了创建logger的接口并封装了创建sink的过程
	*/
	//一个输出到控制台的彩色多线程logger，可以指定名字
	auto console = spdlog::stdout_color_mt("some_unique_name");
	//一个输出到指定文件的轮转文件logger，后面的参数指定了文件的信息
	auto file_logger = spdlog::rotating_logger_mt("file_logger", "logs/mylogfile.log", 1048576 * 5, 3);

	/*
	方式二：组合sinks方式创建
	有时候，单sink的logger不够用，那么可以先创建sink的vector，然后使用sinks_vector创建logger
	以下样例中，首先创建了sink的vector，然后创建了两个sink并放入vector，最后使用该vector创建了logger，
	其中，set_level的过程不是必须的，register_logger一般是必须的，否则只能在创建logger的地方使用该logger，关于register的问题可以往下看
	*/
	std::vector<spdlog::sink_ptr> sinks;

	auto sink1 = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	sink1->set_level(spdlog::level::debug);
	sinks.push_back(sink1);

	std::string log_file_name = "sinks.log";
	auto sink2 = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file_name, 1024 * 1024 * 10, 100, false);
	sink2->set_level(spdlog::level::debug);
	sinks.push_back(sink2);

	auto logger = std::make_shared<spdlog::logger>("logger_name", begin(sinks), end(sinks));
	logger->set_level(spdlog::level::debug);
	spdlog::register_logger(logger);

	/* 
	logger的注册与获取 
	在一个地方创建了logger却只能在该处使用肯定是不好用的，所以spdlog提供了一个全局注册和获取logger，我们只需要在某处先创建logger并注册，那么后面在其他地方使用时直接获取就可以了
	注册：spdlog::register_logger()
	获取：spdlog::get()
	*/
	//上面的代码中我们注册了一个logger，名字是logger_name，接下来尝试获取
	auto logger_sink = spdlog::get(log_file_name);

	/* 
	关于注册与获取需要注意的事:
	必须先创建注册才能获取，建议每个模块的logger都在整个模块最开始初始化时创建并注册。如果在全局尝试获取不存在的logger，会返回空指针，
	如果恰好又使用空指针尝试输出logger,会造成整个程序的崩溃（访问非法内存了，segment fault）
	通过上述的方式一创建的logger是自动注册的，不需要手动注册，但是方式二创建的logger需要手动注册
	一旦注册，全局使用，名字标识logger，在各个模块获取同一个名字的logger会获取到同一个logger的指针
	*/

	/* 
	logger的使用 
	获取到一个logger之后，就可以愉快的使用它了，使用起来很简单
	*/
	logger_sink->debug("this is a debug msg");
	logger_sink->warn("warn!!!!");
	logger_sink->info("hello world");

	/* 
	logger的level设置 
	logger的默认level是info，如果处于开发环境或者生产环境，会只需要debug级别以上或者warn级别以上的log
	要设置logger的级别
	*/
	logger_sink->set_level(spdlog::level::debug);

	/* 可以设置全局logger级别 */
	spdlog::set_level(spdlog::level::warn);

	/* 可以设置sink级别的logger */
	sink1->set_level(spdlog::level::info);
}

