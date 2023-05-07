/*
#include <gtest/gtest.h>

#include "../include/Reader.hpp"

using namespace testing;

// Test fixture for Reader tests
class ReaderTest : public Test {
protected:
    void SetUp() override {
        // Create the logger buffer
        loggerBuffer = std::make_shared<lockfree::SPSCQueue<std::string>>(20);

        // Create the analyzer buffer
        analyzerBuffer = std::make_shared<lockfree::SPSCQueue<std::string>>(20);

        // Initialize the reader
        reader = std::make_unique<Reader>(loggerBuffer, analyzerBuffer);
    }

    void TearDown() override {
        // Stop the reader
        reader->stop();
    }

    std::shared_ptr<lockfree::SPSCQueue<std::string>> loggerBuffer;
    std::shared_ptr<lockfree::SPSCQueue<std::string>> analyzerBuffer;
    std::unique_ptr<Reader> reader;
};

// Test case for read_data() method
TEST_F(ReaderTest, ReadData) {
    // Start the reader
    reader->start();

    // Wait for some time to allow the reader to read data
    std::this_thread::sleep_for(std::chrono::seconds(100));

    // Stop the reader
    reader->stop();

    // Add your assertions here to verify the behavior of the reader
}

*/
/*int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}*/
