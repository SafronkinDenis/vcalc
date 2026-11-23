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

// ==================== Репортер: только итоговая сводка ====================
class SummaryOnlyReporter : public UnitTest::TestReporter {
public:
    void ReportTestStart(UnitTest::TestDetails const& /*test*/) override {}
    void ReportTestFinish(UnitTest::TestDetails const& /*test*/, float /*secondsElapsed*/) override {}
    void ReportFailure(UnitTest::TestDetails const& /*test*/, char const* /*failure*/) override {}

    void ReportSummary(int totalTestCount, int failedTestCount, int /*failureCount*/, float secondsElapsed) override {
        if (failedTestCount == 0)
            std::cout << "Success: " << totalTestCount << " tests passed." << std::endl;
        else
            std::cout << "FAILURE: " << failedTestCount << " out of " << totalTestCount
                      << " tests failed." << std::endl;
        std::cout << "Test time: " << secondsElapsed << " seconds." << std::endl;
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


// БЛОК 1. SUITE(Interface)

SUITE(Interface) {
    TEST(Test1) {
        OutputSuppressor suppress; resetGetopt();
        UserInterface ui; Params p; Args a{"prog", "-h"};
        CHECK(!ui.parseArgs(a.argc(), a.argv(), p));
    }
    TEST(Test2) {
        OutputSuppressor suppress; resetGetopt();
        UserInterface ui; Params p; Args a{"prog", "--help"};
        CHECK(!ui.parseArgs(a.argc(), a.argv(), p));
    }
    TEST(Test3) {
        OutputSuppressor suppress; resetGetopt();
        UserInterface ui; Params p; Args a{"prog"};
        CHECK(!ui.parseArgs(a.argc(), a.argv(), p));
    }
    TEST(Test4) {
        OutputSuppressor suppress; resetGetopt();
        UserInterface ui; Params p; Args a{"prog", "-p", "8080"};
        CHECK(!ui.parseArgs(a.argc(), a.argv(), p));
    }
    TEST(Test5) {
        resetGetopt(); UserInterface ui; Params p; Args a{"prog", "-r"};
        CHECK(ui.parseArgs(a.argc(), a.argv(), p)); CHECK(p.run);
    }
    TEST(Test6) {
        resetGetopt(); UserInterface ui; Params p; Args a{"prog", "--run"};
        CHECK(ui.parseArgs(a.argc(), a.argv(), p)); CHECK(p.run);
    }
    TEST(Test7) {
        resetGetopt(); UserInterface ui; Params p; Args a{"prog", "-r", "-p", "8080"};
        CHECK(ui.parseArgs(a.argc(), a.argv(), p)); CHECK(p.run); CHECK_EQUAL((uint16_t)8080, p.port);
    }
    TEST(Test8) {
        resetGetopt(); UserInterface ui; Params p;
        Args a{"prog", "--run", "-b", "my.conf", "-l", "my.log", "-p", "12345"};
        CHECK(ui.parseArgs(a.argc(), a.argv(), p)); CHECK(p.run);
        CHECK_EQUAL(std::string("my.conf"), p.databaseFile);
        CHECK_EQUAL(std::string("my.log"), p.logFile);
        CHECK_EQUAL((uint16_t)12345, p.port);
    }
    TEST(Test9) {
        resetGetopt(); UserInterface ui; Params p; Args a{"prog", "-r", "-p", "abc"};
        CHECK_THROW(ui.parseArgs(a.argc(), a.argv(), p), InterfaceError);
    }
    TEST(Test10) {
        resetGetopt(); UserInterface ui; Params p; Args a{"prog", "-r", "-x"};
        CHECK_THROW(ui.parseArgs(a.argc(), a.argv(), p), InterfaceError);
    }
}


// БЛОК 2. SUITE(Database)

SUITE(Database) {
    TEST(Test11) {
        UserDatabase db;
        CHECK_THROW(db.load("no_such_file.conf"), IOError);
    }
    TEST(Test12) {
        { std::ofstream f("t_db.conf"); f << "alice:pass1\nbob:pass2\n"; }
        UserDatabase db; db.load("t_db.conf");
        std::string pw;
        CHECK(db.findUser("alice", pw));
        CHECK_EQUAL(std::string("pass1"), pw);
        CHECK(!db.findUser("eve", pw));
        std::remove("t_db.conf");
    }
}


// БЛОК 3. SUITE(SHA1)

SUITE(SHA1) {
    TEST(Test13) {
        CHECK_EQUAL(std::string("A9993E364706816ABA3E25717850C26C9CD0D89D"), Authenticator::calculateSHA1("abc"));
    }
    TEST(Test14) {
        CHECK_EQUAL(std::string("DA39A3EE5E6B4B0D3255BFEF95601890AFD80709"), Authenticator::calculateSHA1(""));
    }
}


// БЛОК 4. SUITE(Auth)
SUITE(Auth) {
    TEST(Test15) {
        { std::ofstream f("t_auth.conf"); f << "alice:password\n"; }
        UserDatabase db; db.load("t_auth.conf");
        Authenticator a(db);
        std::string salt = "0123456789ABCDEF";
        std::string hash = Authenticator::calculateSHA1(salt + "password");
        CHECK(a.authenticate("alice", salt, hash));
        std::remove("t_auth.conf");
    }
    TEST(Test16) {
        { std::ofstream f("t_auth.conf"); f << "alice:password\n"; }
        UserDatabase db; db.load("t_auth.conf");
        Authenticator a(db);
        std::string salt = "0123456789ABCDEF";
        std::string hash = Authenticator::calculateSHA1(salt + "password");
        CHECK_THROW(a.authenticate("eve", salt, hash), AuthError);
        std::remove("t_auth.conf");
    }
    TEST(Test17) {
        { std::ofstream f("t_auth.conf"); f << "alice:password\n"; }
        UserDatabase db; db.load("t_auth.conf");
        Authenticator a(db);
        CHECK_THROW(a.authenticate("alice", "SHORT", "0123456789ABCDEF0123456789ABCDEF01234567"), AuthError);
        std::remove("t_auth.conf");
    }
}


// БЛОК 5. SUITE(Processor)
SUITE(Processor) {
    TEST(Test18) {
        DataProcessor d; std::vector<uint32_t> v;
        CHECK_EQUAL((uint32_t)0, d.calculateSumOfSquares(v));
    }
    TEST(Test19) {
        DataProcessor d; std::vector<uint32_t> v{1, 2, 3};
        CHECK_EQUAL((uint32_t)14, d.calculateSumOfSquares(v));
    }
    TEST(Test20) {
        DataProcessor d; std::vector<uint32_t> v{65535, 65535};
        CHECK_EQUAL((uint32_t)0xFFFFFFFFu, d.calculateSumOfSquares(v));
    }
}

// БЛОК 6. SUITE(Log)
SUITE(Log) {
    TEST(Test21) {
        std::remove("t.log");
        Logger lg("t.log");
        lg.log("hello");
        lg.logError("bad", false);
        lg.logError("fatal", true);
        std::ifstream f("t.log");
        std::string s((std::istreambuf_iterator<char>(f)), {});
        CHECK(s.find("hello") != std::string::npos);
        CHECK(s.find("ERROR") != std::string::npos);
        CHECK(s.find("CRITICAL") != std::string::npos);
        std::remove("t.log");
    }
}

// ==================== Точка входа ====================
int main() {
    SummaryOnlyReporter reporter;
    UnitTest::TestRunner runner(reporter);
    int result = runner.RunTestsIf(UnitTest::Test::GetTestList(), nullptr, RunAllTests(), 0);
    return result;
}
