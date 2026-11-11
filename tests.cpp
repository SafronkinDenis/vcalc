#include <UnitTest++/UnitTest++.h>
#include <UnitTest++/TestReporter.h>
#include <UnitTest++/TestRunner.h>

#include "UserInterface.h"
#include "UserDatabase.h"
#include "Authenticator.h"
#include "DataProcessor.h"
#include "Logger.h"
#include "Errors.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

// ==================== Глобальная переменная для описания теста ====================
thread_local std::string currentTestDescription;

// ==================== Кастомный репортер ====================
class VerboseReporter : public UnitTest::TestReporter {
public:
    VerboseReporter() : currentSuite_(""), currentScenario_("") {}

    void ReportTestStart(UnitTest::TestDetails const& test) override {
        if (test.suiteName != currentSuite_) {
            currentSuite_ = test.suiteName;
            currentScenario_ = "";
            std::cout << std::endl << "Тестирование " << suiteTitle(currentSuite_) << std::endl;
        }

        std::string scenario = extractScenario(test.testName);
        if (scenario != currentScenario_) {
            currentScenario_ = scenario;
            std::cout << "    " << scenarioTitle(currentSuite_, scenario) << std::endl;
        }
    }

    void ReportTestFinish(UnitTest::TestDetails const& test, float /*secondsElapsed*/) override {
        int failures = UnitTest::CurrentTest::Results()->GetFailureCount();
        std::string status = (failures == 0) ? "True" : "False";
        std::cout << "        " << test.testName << ": " << currentTestDescription << " -> " << status << std::endl;
    }

    void ReportFailure(UnitTest::TestDetails const& /*test*/, char const* /*failure*/) override {
    }

    void ReportSummary(int totalTestCount, int failedTestCount, int failureCount, float secondsElapsed) override {
        std::cout << std::endl;
        if (failedTestCount == 0)
            std::cout << "Success: " << totalTestCount << " tests passed." << std::endl;
        else
            std::cout << "FAILURE: " << failedTestCount << " out of " << totalTestCount
                      << " tests failed (" << failureCount << " failures)." << std::endl;
        std::cout << "Test time: " << secondsElapsed << " seconds." << std::endl;
    }

private:
    std::string currentSuite_;
    std::string currentScenario_;

    std::string extractScenario(const std::string& testName) const {
        if (testName.size() < 3 || testName[0] != 'T') return "";
        auto pos = testName.find('_', 1);
        if (pos == std::string::npos) return "";
        return testName.substr(0, pos + 1);
    }

    std::string suiteTitle(const std::string& suite) const {
        static const std::map<std::string, std::string> titles = {
            {"Interface", "пользовательского интерфейса"},
            {"Database",  "базы клиентов"},
            {"SHA1",      "хеш-функции SHA-1"},
            {"Auth",      "аутентификации"},
            {"Processor", "вычислителя"},
            {"Log",       "журнала ошибок"}
        };
        auto it = titles.find(suite);
        return (it != titles.end()) ? it->second : suite;
    }

    std::string scenarioTitle(const std::string& suite, const std::string& scenario) const {
        static const std::map<std::string, std::map<std::string, std::string>> titles = {
            {"Interface", {
                {"T1_", "Тестирование справки"},
                {"T2_", "Тестирование рабочих параметров"},
                {"T3_", "Тестирование исключений"}
            }},
            {"Database", {
                {"T1_", "Тестирование загрузки файла"}
            }},
            {"SHA1", {
                {"T1_", "Тестирование хеш-функции"}
            }},
            {"Auth", {
                {"T1_", "Тестирование аутентификации"}
            }},
            {"Processor", {
                {"T1_", "Тестирование вычислений"}
            }},
            {"Log", {
                {"T1_", "Тестирование записи в журнал"}
            }}
        };
        auto sit = titles.find(suite);
        if (sit == titles.end()) return scenario;
        auto it = sit->second.find(scenario);
        return (it != sit->second.end()) ? it->second : scenario;
    }
};

// Предикат - запускает все тесты
struct RunAllTests {
    bool operator()(UnitTest::Test const*) const { return true; }
};

// ==================== Подавление вывода ====================
class OutputSuppressor {
public:
    OutputSuppressor() {
        old_cout = std::cout.rdbuf();
        old_cerr = std::cerr.rdbuf();
        std::cout.rdbuf(null_stream.rdbuf());
        std::cerr.rdbuf(null_stream.rdbuf());
    }
    ~OutputSuppressor() {
        std::cout.rdbuf(old_cout);
        std::cerr.rdbuf(old_cerr);
    }
private:
    std::streambuf* old_cout;
    std::streambuf* old_cerr;
    std::ofstream null_stream{"/dev/null"};
};

// ==================== Вспомогательное для argv ====================
struct Args {
    std::vector<std::string> s;
    std::vector<char*> p;
    Args(std::initializer_list<const char*> l) {
        for (auto x : l) s.emplace_back(x);
        for (auto& x : s) p.push_back(x.data());
        p.push_back(nullptr);
    }
    int argc() { return (int)p.size() - 1; }
    char** argv() { return p.data(); }
};

void resetGetopt() { optind = 1; opterr = 0; optopt = 0; }

// БЛОК 1. SUITE(Interface) — Тестирование пользовательского интерфейса

SUITE(Interface) {

    TEST(T1_1_HelpShort) {
        currentTestDescription = " Вывод справки, возврат false";
        OutputSuppressor suppress;
        resetGetopt();
        UserInterface ui; Params p;
        Args a{"prog", "-h"};
        CHECK(!ui.parseArgs(a.argc(), a.argv(), p));
    }
    TEST(T1_2_HelpLong) {
        currentTestDescription = " Вывод справки, возврат false";
        OutputSuppressor suppress;
        resetGetopt();
        UserInterface ui; Params p;
        Args a{"prog", "--help"};
        CHECK(!ui.parseArgs(a.argc(), a.argv(), p));
    }
    TEST(T1_3_NoArgsShowsHelp) {
        currentTestDescription = " Вывод справки, возврат false";
        OutputSuppressor suppress;
        resetGetopt();
        UserInterface ui; Params p;
        Args a{"prog"};
        CHECK(!ui.parseArgs(a.argc(), a.argv(), p));
    }
    TEST(T1_4_NoRunFlagShowsHelp) {
        currentTestDescription = " Вывод справки, возврат false";
        OutputSuppressor suppress;
        resetGetopt();
        UserInterface ui; Params p;
        Args a{"prog", "-p", "8080"};
        CHECK(!ui.parseArgs(a.argc(), a.argv(), p));
    }

    TEST(T2_1_RunShort) {
        currentTestDescription = " params.run = true";
        resetGetopt();
        UserInterface ui; Params p;
        Args a{"prog", "-r"};
        CHECK(ui.parseArgs(a.argc(), a.argv(), p));
        CHECK(p.run);
    }
    TEST(T2_2_RunLong) {
        currentTestDescription = " params.run = true";
        resetGetopt();
        UserInterface ui; Params p;
        Args a{"prog", "--run"};
        CHECK(ui.parseArgs(a.argc(), a.argv(), p));
        CHECK(p.run);
    }
    TEST(T2_3_RunWithPort) {
        currentTestDescription = " params.run = true, port = 8080";
        resetGetopt();
        UserInterface ui; Params p;
        Args a{"prog", "-r", "-p", "8080"};
        CHECK(ui.parseArgs(a.argc(), a.argv(), p));
        CHECK(p.run);
        CHECK_EQUAL((uint16_t)8080, p.port);
    }
    TEST(T2_4_RunAllParams) {
        currentTestDescription = " Все параметры установлены корректно";
        resetGetopt();
        UserInterface ui; Params p;
        Args a{"prog", "--run", "-b", "my.conf", "-l", "my.log", "-p", "12345"};
        CHECK(ui.parseArgs(a.argc(), a.argv(), p));
        CHECK(p.run);
        CHECK_EQUAL(std::string("my.conf"), p.databaseFile);
        CHECK_EQUAL(std::string("my.log"),  p.logFile);
        CHECK_EQUAL((uint16_t)12345, p.port);
    }

    TEST(T3_1_BadPort) {
        currentTestDescription = " Возбуждение InterfaceError";
        resetGetopt();
        UserInterface ui; Params p;
        Args a{"prog", "-r", "-p", "abc"};
        CHECK_THROW(ui.parseArgs(a.argc(), a.argv(), p), InterfaceError);
    }
    TEST(T3_2_UnknownOpt) {
        currentTestDescription = " Возбуждение InterfaceError";
        resetGetopt();
        UserInterface ui; Params p;
        Args a{"prog", "-r", "-x"};
        CHECK_THROW(ui.parseArgs(a.argc(), a.argv(), p), InterfaceError);
    }
}


// БЛОК 2. SUITE(Database) — Тестирование базы клиентов

SUITE(Database) {

    TEST(T1_1_NoFile) {
        currentTestDescription = " Возбуждение IOError";
        UserDatabase db;
        CHECK_THROW(db.load("no_such_file.conf"), IOError);
    }
    TEST(T1_2_LoadAndFind) {
        currentTestDescription = " load() = true, поиск работает";
        { std::ofstream f("t_db.conf"); f << "alice:pass1\nbob:pass2\n"; }
        UserDatabase db;
        db.load("t_db.conf");
        std::string pw;
        CHECK(db.findUser("alice", pw));
        CHECK_EQUAL(std::string("pass1"), pw);
        CHECK(!db.findUser("eve", pw));
        std::remove("t_db.conf");
    }
}


// БЛОК 3. SUITE(SHA1) — Тестирование SHA-1

SUITE(SHA1) {

    TEST(T1_1_KnownValue) {
        currentTestDescription = " hash = A9993E364706816ABA3E25717850C26C9CD0D89D";
        CHECK_EQUAL(std::string("A9993E364706816ABA3E25717850C26C9CD0D89D"),
                    Authenticator::calculateSHA1("abc"));
    }
    TEST(T1_2_Empty) {
        currentTestDescription = " hash = DA39A3EE5E6B4B0D3255BFEF95601890AFD80709";
        CHECK_EQUAL(std::string("DA39A3EE5E6B4B0D3255BFEF95601890AFD80709"),
                    Authenticator::calculateSHA1(""));
    }
}


// БЛОК 4. SUITE(Auth) — Тестирование аутентификации

SUITE(Auth) {

    TEST(T1_1_GoodAuth) {
        currentTestDescription = " authenticate() = true";
        { std::ofstream f("t_auth.conf"); f << "alice:password\n"; }
        UserDatabase db; db.load("t_auth.conf");
        Authenticator a(db);
        std::string salt = "0123456789ABCDEF";
        std::string hash = Authenticator::calculateSHA1(salt + "password");
        CHECK(a.authenticate("alice", salt, hash));
        std::remove("t_auth.conf");
    }
    TEST(T1_2_WrongUser) {
        currentTestDescription = " Возбуждение AuthError";
        { std::ofstream f("t_auth.conf"); f << "alice:password\n"; }
        UserDatabase db; db.load("t_auth.conf");
        Authenticator a(db);
        std::string salt = "0123456789ABCDEF";
        std::string hash = Authenticator::calculateSHA1(salt + "password");
        CHECK_THROW(a.authenticate("eve", salt, hash), AuthError);
        std::remove("t_auth.conf");
    }
    TEST(T1_3_BadSalt) {
        currentTestDescription = " Возбуждение AuthError";
        { std::ofstream f("t_auth.conf"); f << "alice:password\n"; }
        UserDatabase db; db.load("t_auth.conf");
        Authenticator a(db);
        CHECK_THROW(a.authenticate("alice", "SHORT",
            "0123456789ABCDEF0123456789ABCDEF01234567"), AuthError);
        std::remove("t_auth.conf");
    }
}


// БЛОК 5. SUITE(Processor) — Тестирование вычислителя

SUITE(Processor) {

    TEST(T1_1_Empty) {
        currentTestDescription = " result = 0";
        DataProcessor d;
        std::vector<uint32_t> v;
        CHECK_EQUAL((uint32_t)0, d.calculateSumOfSquares(v));
    }
    TEST(T1_2_Simple) {
        currentTestDescription = " result = 14 (1² + 2² + 3²)";
        DataProcessor d;
        std::vector<uint32_t> v{1, 2, 3};
        CHECK_EQUAL((uint32_t)14, d.calculateSumOfSquares(v));
    }
    TEST(T1_3_Overflow) {
        currentTestDescription = " result = 4294967295 (переполнение)";
        DataProcessor d;
        std::vector<uint32_t> v{65535, 65535};
        CHECK_EQUAL((uint32_t)0xFFFFFFFFu, d.calculateSumOfSquares(v));
    }
}


// БЛОК 6. SUITE(Log) — Тестирование журнала

SUITE(Log) {

    TEST(T1_1_Write) {
        currentTestDescription = " В файле есть INFO, ERROR, CRITICAL";
        std::remove("t.log");
        Logger lg("t.log");
        lg.log("hello");
        lg.logError("bad", false);
        lg.logError("fatal", true);
        std::ifstream f("t.log");
        std::string s((std::istreambuf_iterator<char>(f)), {});
        CHECK(s.find("hello")  != std::string::npos);
        CHECK(s.find("ERROR")  != std::string::npos);
        CHECK(s.find("CRITICAL") != std::string::npos);
        std::remove("t.log");
    }
}

// ==================== Точка входа ====================
int main() {
    std::cout << "=== ЗАПУСК МОДУЛЬНЫХ ТЕСТОВ ===" << std::endl;
    VerboseReporter reporter;
    UnitTest::TestRunner runner(reporter);
    int result = runner.RunTestsIf(UnitTest::Test::GetTestList(), nullptr, RunAllTests(), 0);
    std::cout << "=== ТЕСТЫ ЗАВЕРШЕНЫ ===" << std::endl;
    return result;
}
