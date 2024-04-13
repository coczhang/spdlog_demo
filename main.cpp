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
��־�ȼ������������ε���
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

/* ����stdout/stderr��¼������ */
void stdout_example() {
	auto console = spdlog::stderr_color_mt("console");
	auto err_loger = spdlog::stderr_color_mt("stderr");
	spdlog::get("console")->info("loggers can be retrieved from a global registry using the spdlog::get(logger_name)");
	spdlog::get("stderr")->info("loggers can be retrieved from a global registry using the spdlog::get(logger_name)");
}

/* �����ļ���¼�� */
void basic_lofile_example() {
	try {
		auto logger = spdlog::basic_logger_mt("basic_logger", "logs/basic_logger.log");
		logger->info("basic_logger");

	} catch (const spdlog::spdlog_ex& ex) {
		std::cout << "Log init failed:" << ex.what() << std::endl;
	}
}

/* �����ļ���С */
void rotaing_example() {
	auto max_size = 1048576 * 5;
	// Create a file rotating logger with 5mb size max and 3 rotated files
	auto max_files = 3;
	auto logger = spdlog::rotating_logger_mt("some_logger", "logs/rotating.log", max_size, max_files);
}

/* �����ļ�����Ƶ�ʣ��������� */
void daily_example() {
	// Create a daily logger - a new file is created every day on 2:30am
	auto logger = spdlog::daily_logger_mt("daily_logger", "logs/daily.log", 2, 30);

	/* ʵ����ÿ3�����һ����־�������ͬʱҲҪע�⣬ʹ��ʱȷ����־�������̰߳�ȫ�ġ� */
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
		�첽��־������8K����Ŀ��1����̨�̵߳Ķ��� 
		8192:�������ָ�����̳߳صĴ�С��Ҳ�����̳߳���������ͬʱ���е��߳�����������������У��̳߳صĴ�С������Ϊ 8192��
		1:�������ָ�����̳߳ص��߳�����������������У��̳߳���ֻ��һ���̡߳�
	*/
	spdlog::init_thread_pool(8192, 1);
	/* ��׼����̨��� */
	auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	stdout_sink->set_level(spdlog::level::debug);
	/* ��־�ļ������0��0�ִ�������־ */
	auto file_sink = std::make_shared<spdlog::sinks::daily_file_format_sink_mt>("logs/log.txt", 0, 0);
	file_sink->set_level(spdlog::level::info);
	auto callback_sink = std::make_shared<spdlog::sinks::callback_sink_mt>([](const spdlog::details::log_msg& msg) {
		/* ��־��¼������ */
		std::string name(msg.logger_name.data(), 0, msg.logger_name.size());
		/* ��־��Ϣ */
		std::string str(msg.payload.data(), 0, msg.payload.size());
		/* ��־ʱ�� */
		std::time_t now_c = std::chrono::system_clock::to_time_t(msg.time);

		//�ص��Ĵ����߼��Լ�������Ŀ������壬����ʵʱ��ʾ��UI�����浽���ݿ�ȵ�

		//.... �ص������߼���ʾ��
	    //std::tm localTime;
	    //localtime_s(&localTime, &now_c);
		//char timeStr[50];
		//std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &localTime);
		//// ��ȡ������
		//auto duration = msg.time.time_since_epoch();
		//auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;
		//std::cout << timeStr << "." << std::setfill('0') << std::setw(3) << milliseconds << " " ;
		//std::cout << to_string_view(msg.level).data() << " " << str << std::endl << std::endl << std::flush;
	});
	callback_sink->set_level(spdlog::level::info);

	/* 
		���а�����������ͬ���͵���־���Ŀ�꣺stdout_sink��file_sink �� callback_sink��
		��ЩĿ������ָ����־��Ϣ�������ʽ���������������̨��������ļ���ͨ���ص��������� 
	*/
	std::vector<spdlog::sink_ptr> sinks{ stdout_sink, file_sink,callback_sink };
	
	/*
		��һ�д�����һ����Ϊ log ���첽��־��¼����������˵��
		"logger" ����־��¼�������ơ�
		sinks.begin() �� sinks.end() ��ʾ��ʼ����־��¼��ʱʹ�õ����Ŀ�귶Χ������һ�д����� sinks �����е�Ŀ�ꡣ
		spdlog::thread_pool() ������һ�������첽��־��¼���̳߳ء�
		spdlog::async_overflow_policy::block ��ʾ����־��Ϣ������������ʱ��������ԣ��˴�������ģʽ�������޷����������־��Ϣʱ�����µ���־��Ϣ��
	*/
	auto log = std::make_shared<spdlog::async_logger>("logger", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);

	//������־��¼��������Ҫ�� %^ �� %$  ������Ҫ��ɫ�Ĳ���
	log->set_level(spdlog::level::trace);
	//���ø�ʽ
	//�ο� https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
	//[%Y-%m-%d %H:%M:%S.%e] ʱ��
	//[%l] ��־����
	//[%t] �߳�
	//[%s] �ļ�
	//[%#] �к�
	//[%!] ����
	//[%v] ʵ���ı�
	log->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %^[%l]%$ [%t] [%s %!:%#] %v");
	log->info("stdout_sink, file_sink,callback_sink");
	//���õ����� err ������صĴ���ʱ����ˢ����־��  disk
	log->flush_on(spdlog::level::err);
	//3��ˢ��һ�ζ���
	spdlog::flush_every(std::chrono::seconds(3));
	spdlog::set_default_logger(log);
}

//������־��¼��
std::shared_ptr<spdlog::logger> get_async_file_logger(std::string name) {
	auto log = spdlog::get(name);
	if (!log) {
		//ָ��Ϊ�գ��򴴽���־��¼����
		log = spdlog::daily_logger_mt<spdlog::async_factory>(name, "logs/" + name + "/log.txt");
		log->set_level(spdlog::level::trace);
		log->flush_on(spdlog::level::err);
		log->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %^[%l]%$ [%t] [%s %!:%#] %v");
		//��¼�����Զ�ע��ģ�����Ҫ�ֶ�ע��  spdlog::register_logger(name);
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

//������־�ļ�
#define GETLOG(LOG_NAME) get_async_file_logger(LOG_NAME)

#define LOGGER_TRACE(logger,...)     SPDLOG_LOGGER_TRACE(logger,__VA_ARGS__)
#define LOGGER_DEBUG(logger,...)     SPDLOG_LOGGER_DEBUG(logger,__VA_ARGS__)
#define LOGGER_INFO(logger,...)      SPDLOG_LOGGER_INFO(logger,__VA_ARGS__)
#define LOGGER_WARN(logger,...)      SPDLOG_LOGGER_WARN(logger,__VA_ARGS__)
#define LOGGER_ERROR(logger,...)     SPDLOG_LOGGER_ERROR(logger,__VA_ARGS__)
#define LOGGER_CRITICAL(logger,...)  SPDLOG_LOGGER_CRITICAL(logger,__VA_ARGS__)

//ʱ��ͳ�ƺ�
#define LOGSW() spdlog::stopwatch()

int main() {
#if 0
	/* 
		������־�ȼ�������־�ȼ����ڵ�ǰ����־�ȼ���
		��ô���͵ȼ���־���޷���ӡ
	*/
	spdlog::set_level(spdlog::level::trace);
	/* 
		Spdlog ֧�ֵĸ�ʽ����������
		%v ���� ��־����
		%n ���� ����
		%d ���� ���ں�ʱ��
		%r ���� 12 Сʱ��ʱ��
		%R ���� 24 Сʱ��ʱ��
		%T ���� 24 Сʱ��ʱ�䣨�뾫�ȣ�
		%H ���� Сʱ��00-23��
		%M ���� ���ӣ�00-59��
		%S ���� �루00-59��
		%e ���� ���루��λ���֣�
		%f ���� ΢�루��λ���֣�
		%F ���� ���루��λ���֣�
		%z ���� ʱ��ƫ����
		%s ���� ʱ���
		%L ���� ��־����
		%t ���� �߳� ID
		%P ���� ���� ID
		%^ ���� ת��Ϊ��д��ĸ
		%$ ���� ת��Ϊ��ɫ���

		���磬����ʹ�����´��뽫��־����ת��Ϊ��д��ĸ��
		spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
		������Ĵ����У�"[%^%l%$]" ��ʾ����־����ת��Ϊ��д��ĸ��
	*/
	spdlog::set_pattern("[%H:%M:%S %z] [%^%L%$] [thread %t] %v");
	/* ����־��Ϣ��ӡ������̨ */
	spdlog::trace("hello spdlog trace");
	spdlog::debug("hello spdlog debug");
	/* ��{}Ϊռλ�� */
	spdlog::info("hello spdlog {}", "info");
	spdlog::warn("hello spdlog warn");
	spdlog::error("hello spdlog error");
	spdlog::critical("hello spdlog critical");

	/* ������־ͷ��Ϣ������ΪĬ������ */
	spdlog::set_pattern("%+");  // back to default format
	spdlog::info("hello world");

	/* ����־��Ϣд�뵽�����ļ��� */
	auto logger = spdlog::basic_logger_mt("file_logger", "basic-log.log");
	logger->set_level(spdlog::level::trace);
	/*
	   [%Y-%m-%d %H:%M:%S.%e] ʱ��
	   [%l] ��־����
	   [%s] �ļ�
	   [%#] �к�
	   [%!] ����
	   [%v] ʵ���ı�
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
		ʵʱ�Ľ���־��Ϣд�뵽�ļ��У��������δ�˳�ʱ����־��Ϣ�޷����µ��ļ���
		50����ˢ��һ�ζ���
	*/
	spdlog::flush_every(std::chrono::milliseconds(50));

	/*
		spdlog::sinks::stdout_color_sink_mt �� spdlog ���ṩ��һ�����Ŀ�꣨sink����
		���ܹ�����־��Ϣ���͵���׼�����stdout��������Ϊ��ͬ����־��������󡢾��桢��Ϣ�������ɫ��ǡ�
	*/
	// ����ָ�� stdout_color_sink_mt ����� shared ָ��
	auto sink1 = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	// ���������ʹ�� 'sink1' �����������־��¼�������磺
	auto sink1_logger = std::make_shared<spdlog::logger>("console", sink1);
	spdlog::register_logger(sink1_logger);
	// ��¼һЩ��־��Ϣ
	sink1_logger->info("sink1:����һ����Ϣ��Ϣ");
	sink1_logger->error("sink1:����һ��������Ϣ");

	/*
		spdlog::sinks::daily_file_sink_mt ��һ������ spdlog ������Ŀ�꣬�����Խ���־��Ϣд�뵽�ļ��У�
		�����ܹ��������ڽ�����־�ļ��Ĺ�����ÿ�������һ���µ���־�ļ�����ȷ����־�ļ��Ĵ�С���������������Ӷ������ڹ����ά����
	*/
	// ����ָ�� daily_file_sink_mt ����� shared ָ��
	auto sink2 = std::make_shared<spdlog::sinks::daily_file_sink_mt>("sink2.log", 23, 00);//ÿ���23��00�ִ����µ��ļ�
	// ���������ʹ�� 'sink2' �����������־��¼�������磺
	auto sink2_logger = std::make_shared<spdlog::logger>("daily_logger", sink2);
	spdlog::register_logger(sink2_logger);
	// ��¼һЩ��־��Ϣ
	sink2_logger->info("sink2:����һ����Ϣ��Ϣ");
	sink2_logger->error("sink2:һ��������Ϣ");

	/* spdlog::sinks::rotating_file_sink_mt �� spdlog ���е�һ����־���Ŀ�꣨sink��
	   �����ڽ���־д�뵽�ļ��С���֧����־�ļ����Զ���ת������־�ļ��ﵽָ����Сʱ�����Զ�����һ���µ���־�ļ���
	   ������־д�뵽���ļ��С� 
	*/
	// ����ָ�� rotating_file_sink_mt ����� shared_ptr
	auto sink3 = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("sink3.log", 1024 * 1024 * 5, 3); // 5MB �ļ���С����ౣ�� 3 ���ļ�
	// ���������ʹ�� 'sink3' �����������־��¼�������磺
	auto sink3_logger = std::make_shared<spdlog::logger>("rotating_logger", sink3);
	spdlog::register_logger(sink3_logger);
	// ��¼һЩ��־��Ϣ
	sink3_logger->info("sink3:����һ����Ϣ��Ϣ");
	sink3_logger->error("sink3:����һ��������Ϣ");

	// ��־�ļ���
	std::string log_file_name = "example.log";
	// ����ָ�� basic_file_sink_mt ����� shared_ptr
	auto sink4 = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_name);
	// ���������ʹ�� 'sink4' �����������־��¼�������磺
	auto sink4_logger = std::make_shared<spdlog::logger>("example_logger", sink4);
	spdlog::register_logger(sink4_logger);
	// ��¼һЩ��־��Ϣ
	sink4_logger->info("sink4_logger:����һ����Ϣ��Ϣ");
	sink4_logger->error("sink4_logger:����һ��������Ϣ");

	// ����־��¼�������ٻ��ߵ��� spdlog::shutdown() ʱ��spdlog ���Զ�ˢ����־

	/*
		spdlog::sinks::stdout_color_sink_mt ��һ�����Ŀ�꣨sink����������־��Ϣ�������׼�����stdout����
		ͨ�����ڿ���̨���������֧���������Ϊ��ͬ����־���������ɫ��ǡ�
		������뽫��־ͬʱ������ļ��У�����ʹ�� spdlog::sinks::stdout_color_sink_mt �� spdlog::sinks::basic_file_sink_mt ���������
	*/
	 // ����ָ�� stdout_color_sink_mt �� basic_file_sink_mt ����� shared ָ��
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("console_sink.log", true); // true ��ʾ׷�ӵ��ļ�ĩβ
	// ����һ������ sink��ͬʱ���������̨���ļ�
	auto combined_sink = std::make_shared<spdlog::sinks::dist_sink_mt>();
	combined_sink->add_sink(console_sink);
	combined_sink->add_sink(file_sink);
	// ���������ʹ�� 'combined_sink' �����������־��¼�������磺
	auto combined_logger = std::make_shared<spdlog::logger>("combined_logger", combined_sink);
	spdlog::register_logger(combined_logger);
	// ��¼һЩ��־��Ϣ
	combined_logger->info("combined_logger:����һ����Ϣ��Ϣ");
	combined_logger->error("combined_logger:����һ��������Ϣ");

	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	init_spdlog();
	stdout_example();
	basic_lofile_example();

	/* ������־������ʱ�������ڴ��У���Ҫ��ʱ�����ͨ���ӿڶԻ������־���������
	   ͨ����������������־������������־��¼����
	   ����Ĵ������ñ��������32����־��Ϣ��
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
	/* ʹ������ */
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
	���ĸ��
	1.logger����־����ÿ����־�ڰ���һ��sink��ɵ�vector��ÿ��sink���Էֱ��������ȼ���logger����Ҳ���������ȼ�
	2.sink��ֱ����ˮ�ۣ�ʵ�����������Ķ�����߿�����Ϊ�����Ŀ�꣬spdlog�������˶��ֲ�ͬ���͵�logger�ɹ�ѡ��
	3.formatter����ʽ�����󣬾��󲿷������spdlogĬ�ϵĸ�ʽ���㹻���ˣ���������и��Ի����󣬿��Խ����Զ����ʽ
	4.level����־���𣬲�ͬ����־����ܻ��в�ͬ�����ã����ǻ�������¶�����debug��info��warn��error�ȵļ��𻮷�������ͬ�������������������������Ը����Լ���ʵ�����ѡȡ
	
	�߼���ϵ��ÿ��logger����һ��vector����vector��һ������std::shared_ptr<sink>��ɣ�logger��ÿ����־�������sink����
	��sink������formatter�ĸ�ʽ�����sinkָ���ĵط����п����ǿ���̨���ļ��ȣ������������Ǵ��ڵ���Ľ���spdlog���������������
	
	formatter:
	formatterҲ����ʽ���������ڿ�����־�������ʽ,spdlog�Դ���Ĭ�ϵ�formatter��һ������£����������κ��޸ģ�ֱ��ʹ�ü��ɡ�ע�⣬ÿ��sink����һ��formatter
	
	Ĭ��formatter�ĸ�ʽΪ��[����ʱ��] [logger��] [log����] log����
	[2024-04-13 17:00:55.795] [spd_logger] [debug] found env XXXXXXX : true
	[2024-04-13 17:00:55.795] [func_config] [debug] kafka_brokers : localhost:9092
	[2024-04-13 17:00:55.795] [func_config] [debug] kafka_main_topic : kafka_test
	[2024-04-13 17:00:55.795] [func_config] [debug] kafka_partition_value : -1
	[2024-04-13 17:00:55.795] [spd_logger] [info] initialized
	
	�Զ���formatter:
	���Ĭ�ϵ�formatter���������󣬿����Զ���formatter�����巽ʽ����:
	set_parrtern(pattern_string);
	���磺
	ȫ�ּ���ģ�spdlog::set_pattern(" [%H:%M:%S %z] [thread %t] %v ");
	����logger����ģ�some_logger->set_parttern(">>> %H:%M:%S %z %v <<<");
	����sink����ģ�some_sink-> set_parttern(".. %H: %M ..");
	�����õ���%H %M��Щռλ������ʵ�����Ƕ���Ԥ���趨�õģ���Ҫ�鿴���е�ռλ����������Բο�������վ��
	https://spdlog.docsforge.com/v1.x/3.custom-formatting/#pattern-flags

	sink:
	ÿ��sink��Ӧ��һ�����Ŀ��������ʽ�����ڲ�����һ��formatter�����Ŀ������ǿ���̨���ļ��ȵط���
	���е�sink���������ռ�spdlog::sinks�£���������̽��

	����̨sink:
	spdlog�д�������̨sink�ǳ��򵥣��÷�ʽ������sink��������������նˣ����ǲ�ɫ�ģ�Ҳ����ѡ�ǲ�ɫ�ģ������в�ɫ��Ӧ�ö���ѡ��ɫ�İɡ���������׺��_mt������̣߳�_st�����߳�
	auto sink1 = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

	�ļ�sink:
	//��򵥵��ļ�sink��ֻ��Ҫָ���ļ���
	auto sink1 = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_name);

	//ÿ���12��00����path�´����µ��ļ�
	auto sink2 = std::make_shared<spdlog::sinks::daily_file_sink_mt>(log_file_name, path, 12, 00);

	//��ת�ļ���һ���ļ����˻�д����һ���ļ����ڶ��������ǵ��ļ���С���ޣ��������������ļ��������ֵ
	auto sink3 = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file_name, 1024 * 1024 * 10, 100, false);

	����sink:
	ostream_sink
	syslog_sink
	......
	Ҳ����ͨ���̳�base_sink�����������Զ���sink��������Բο���
	https://spdlog.docsforge.com/v1.x/4.sinks/#implementing-your-own-sink

	sink��flush����:
	������sink��������flush��ʽ����������޷�������file�п���logger������
	����Ϊ������Ҫ��flush��ʽ���ã�ֱ������ȫ�֣�
	spdlog::flush_every(std::chrono::seconds(1));
	spdlog::flush_on(spdlog::level::debug);

	logger:
	��־����ÿ��logger�ڰ�����һ��vector���ڴ��sink��ÿ��sink�����໥����
	���һ����־�����������־ʱ����ͬʱ���������̨���ļ���λ��

	ʹ��Ĭ��logger:
	���������Ŀ��ֻ��Ҫһ��logger��spdlog�ṩ����Ϊ��ݵ�Ĭ��logger��ע�⣬��logger��ȫ�ֹ��ã����������̨�����̡߳���ɫ
	//Use the default logger (stdout, multi-threaded, colored)
	spdlog::info("Hello, {}!", "World");

	�����ض���logger:
	�󲿷������Ĭ��logger�ǲ����õģ���Ϊ���ǿ�����Ҫ����ͬ��Ŀģ����Ե�logger��������Ҫlogger������ļ����г־û������Դ���logger�Ǻ���Ҫ��һ���¡����ڴ���loggerҲ�Ƿǳ��򵥵ģ�
	
	ע�⣺һ��logger�ڼ����ж��sink����ô��Щsink�ֱ�����level�ǿ��Բ�ͬ�ģ���������logger����Ҳ��level����������ʹ��ʱ��logger��level�������ĳ��sink��
	�Ḳ�Ǹ�sink��level�����Խ����ʱ��logger��level�ֶ�����Ϊdebug��Ĭ��Ϊinfo��
	*/

	/*
	��ʽһ��ֱ�Ӵ���
	�봴��sink���ƣ����ǿ��Էǳ���ݵĴ���logger
	���ڴ󲿷�ʱ��һ��loggerֻ����һ��sink������spdlog�ṩ�˴���logger�Ľӿڲ���װ�˴���sink�Ĺ���
	*/
	//һ�����������̨�Ĳ�ɫ���߳�logger������ָ������
	auto console = spdlog::stdout_color_mt("some_unique_name");
	//һ�������ָ���ļ�����ת�ļ�logger������Ĳ���ָ�����ļ�����Ϣ
	auto file_logger = spdlog::rotating_logger_mt("file_logger", "logs/mylogfile.log", 1048576 * 5, 3);

	/*
	��ʽ�������sinks��ʽ����
	��ʱ�򣬵�sink��logger�����ã���ô�����ȴ���sink��vector��Ȼ��ʹ��sinks_vector����logger
	���������У����ȴ�����sink��vector��Ȼ�󴴽�������sink������vector�����ʹ�ø�vector������logger��
	���У�set_level�Ĺ��̲��Ǳ���ģ�register_loggerһ���Ǳ���ģ�����ֻ���ڴ���logger�ĵط�ʹ�ø�logger������register������������¿�
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
	logger��ע�����ȡ 
	��һ���ط�������loggerȴֻ���ڸô�ʹ�ÿ϶��ǲ����õģ�����spdlog�ṩ��һ��ȫ��ע��ͻ�ȡlogger������ֻ��Ҫ��ĳ���ȴ���logger��ע�ᣬ��ô�����������ط�ʹ��ʱֱ�ӻ�ȡ�Ϳ�����
	ע�᣺spdlog::register_logger()
	��ȡ��spdlog::get()
	*/
	//����Ĵ���������ע����һ��logger��������logger_name�����������Ի�ȡ
	auto logger_sink = spdlog::get(log_file_name);

	/* 
	����ע�����ȡ��Ҫע�����:
	�����ȴ���ע����ܻ�ȡ������ÿ��ģ���logger��������ģ���ʼ��ʼ��ʱ������ע�ᡣ�����ȫ�ֳ��Ի�ȡ�����ڵ�logger���᷵�ؿ�ָ�룬
	���ǡ����ʹ�ÿ�ָ�볢�����logger,�������������ı��������ʷǷ��ڴ��ˣ�segment fault��
	ͨ�������ķ�ʽһ������logger���Զ�ע��ģ�����Ҫ�ֶ�ע�ᣬ���Ƿ�ʽ��������logger��Ҫ�ֶ�ע��
	һ��ע�ᣬȫ��ʹ�ã����ֱ�ʶlogger���ڸ���ģ���ȡͬһ�����ֵ�logger���ȡ��ͬһ��logger��ָ��
	*/

	/* 
	logger��ʹ�� 
	��ȡ��һ��logger֮�󣬾Ϳ�������ʹ�����ˣ�ʹ�������ܼ�
	*/
	logger_sink->debug("this is a debug msg");
	logger_sink->warn("warn!!!!");
	logger_sink->info("hello world");

	/* 
	logger��level���� 
	logger��Ĭ��level��info��������ڿ�����������������������ֻ��Ҫdebug�������ϻ���warn�������ϵ�log
	Ҫ����logger�ļ���
	*/
	logger_sink->set_level(spdlog::level::debug);

	/* ��������ȫ��logger���� */
	spdlog::set_level(spdlog::level::warn);

	/* ��������sink�����logger */
	sink1->set_level(spdlog::level::info);
}

