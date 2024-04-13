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
#define ERROR(...)     SPDLOG_ERROR(__VA_ARGS__)
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
#endif

	init_spdlog();

	return 1;
}