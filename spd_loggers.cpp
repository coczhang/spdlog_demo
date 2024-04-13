#include "spd_loggers.h"

spdlog::level::level_enum MyLoggers::global_level = spdlog::level::info;

spdlog::level::level_enum MyLoggers::getGlobalLevel() {
    return global_level;
}

std::vector<spdlog::sink_ptr> MyLoggers::createSinks(const std::string& log_file_name) {
    std::vector<spdlog::sink_ptr> sinks;

    auto sink1 = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    sink1->set_level(MyLoggers::getGlobalLevel());
    sinks.push_back(sink1);

    auto sink2 = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file_name, 1024 * 1024 * 10, 100, false);
    sink2->set_level(spdlog::level::debug);
    sinks.push_back(sink2);
    return sinks;
}

void MyLoggers::createLogger(const std::string& logger_name) {
    std::string log_file_name = logger_name + "_log.txt";
    auto sinks = MyLoggers::createSinks(log_file_name);

    auto logger = std::make_shared<spdlog::logger>(logger_name, begin(sinks), end(sinks));
    logger->set_level(spdlog::level::debug);
    spdlog::register_logger(logger);
}

std::shared_ptr<spdlog::logger> MyLoggers::getLogger(const std::string& logger_name) {
    auto logger = spdlog::get(logger_name);
    if (!logger) {
        //loogerÖ¸ÏòÎª¿Õ
        createLogger(logger_name);
        logger = spdlog::get(logger_name);
    }
    return logger;
}

void MyLoggers::init() {
    auto level = spdlog::level::debug;
    if (std::getenv("STAGE") != nullptr) {
        std::string stage = std::getenv("STAGE");
        if (stage == "dev")
            level = spdlog::level::debug;
    }
    MyLoggers::global_level = level;

    spdlog::flush_every(std::chrono::seconds(1));
    spdlog::flush_on(spdlog::level::debug);

    MyLoggers::createLogger("service");
}
