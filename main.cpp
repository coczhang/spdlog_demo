#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/stopwatch.h"
#include "spdlog/sinks/callback_sink.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <memory>

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
#define ERROR(...)     SPDLOG_ERROR(__VA_ARGS__)
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
#endif

	init_spdlog();

	return 1;
}